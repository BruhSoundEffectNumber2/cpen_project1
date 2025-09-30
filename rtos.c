// os.c
// Runs on LM4F120/TM4C123
// A very simple real time operating system with minimal features.
// Daniel Valvano
// January 29, 2015
// Edited by John Tadrous
// June 25, 2020

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015


 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// #include "TM4C123GH6PM.h"
#include "tm4c123gh6pm_def.h"
#include <stdint.h>

/* #define NVIC_ST_CTRL_R          (*((volatile uint32_t *)0xE000E010))
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_R        (*((volatile uint32_t *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile uint32_t *)0xE000E018))
#define NVIC_INT_CTRL_R         (*((volatile uint32_t *)0xE000ED04))
#define NVIC_INT_CTRL_PENDSTSET 0x04000000  // Set pending SysTick interrupt
#define NVIC_SYS_PRI3_R         (*((volatile uint32_t *)0xE000ED20))  // Sys. Handlers 12 to 15 Priority
*/

// Function definitions in osasm.s
void OS_DisableInterrupts();
void OS_EnableInterrupts();
int32_t StartCritical();
void EndCritical(int32_t primask);
void Clock_Init();
void StartOS();

// Maximum number of threads
#define NUM_THREADS 3
// Number of 32-bit words in stack
#define STACK_SIZE 100

struct TCB
{
  // Pointer to stack (valid for threads not running)
  int32_t *sp;
  // Linked-list pointer
  struct TCB *next;
  // If non-zero, this thread is blocked by the semaphore pointed to
  uint32_t *blocked;
};

struct TCB tcbs[NUM_THREADS];
struct TCB *RunPt;
int32_t Stacks[NUM_THREADS][STACK_SIZE];

uint32_t Mail;
uint32_t Lost = 0;
uint32_t Send = 0;

void OS_Suspend()
{
  // Reset counter for fairness
  // NVIC_ST_CURRENT_R = 0;
  // Forcibly trigger an interrupt
  NVIC_INT_CTRL_R |= 0x04000000;
}

void OS_Wait(uint32_t *s)
{
  OS_DisableInterrupts();

  (*s) -= 1;

  if ((*s) < 0)
  {
    RunPt->blocked = s;
    OS_EnableInterrupts();
    OS_Suspend();
  }

  OS_EnableInterrupts();
}

void OS_Signal(uint32_t *s)
{
  struct TCB *pt;

  OS_DisableInterrupts();

  (*s) += 1;

  if ((*s) <= 0)
  {
    // Find and unblock another task
    pt = RunPt->next;

    while (pt->blocked != s)
    {
      pt = pt->next;
    }

    pt->blocked = 0;
  }

  OS_EnableInterrupts();
}

void OS_Schedule()
{
  // Round robin - find the next non-blocked task

  RunPt = RunPt->next;

  while (RunPt->blocked)
  {
    RunPt = RunPt->next;
  }
}

void SendMail(uint32_t data)
{
  Mail = data;

  if (Send)
  {
    Lost++;
  }
  else
  {
    OS_Signal(&Send);
  }
}

uint32_t RecvMail()
{
  OS_Wait(&Send);

  return Mail;
}

// ******** OS_Init ************
// initialize operating system, disable interrupts until OS_Launch
// initialize OS controlled I/O: systick, 16 MHz clock
// input:  none
// output: none
void OS_Init(void)
{
  OS_DisableInterrupts();
  Clock_Init();                                                  // set processor clock to 16 MHz
  NVIC_ST_CTRL_R = 0;                                            // disable SysTick during setup
  NVIC_ST_CURRENT_R = 0;                                         // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0xE0000000; // priority 7
}

void SetInitialStack(int i)
{
  tcbs[i].sp = &Stacks[i][STACK_SIZE - 16]; // thread stack pointer
  Stacks[i][STACK_SIZE - 1] = 0x01000000;   // thumb bit
  Stacks[i][STACK_SIZE - 3] = 0x14141414;   // R14
  Stacks[i][STACK_SIZE - 4] = 0x12121212;   // R12
  Stacks[i][STACK_SIZE - 5] = 0x03030303;   // R3
  Stacks[i][STACK_SIZE - 6] = 0x02020202;   // R2
  Stacks[i][STACK_SIZE - 7] = 0x01010101;   // R1
  Stacks[i][STACK_SIZE - 8] = 0x00000000;   // R0
  Stacks[i][STACK_SIZE - 9] = 0x11111111;   // R11
  Stacks[i][STACK_SIZE - 10] = 0x10101010;  // R10
  Stacks[i][STACK_SIZE - 11] = 0x09090909;  // R9
  Stacks[i][STACK_SIZE - 12] = 0x08080808;  // R8
  Stacks[i][STACK_SIZE - 13] = 0x07070707;  // R7
  Stacks[i][STACK_SIZE - 14] = 0x06060606;  // R6
  Stacks[i][STACK_SIZE - 15] = 0x05050505;  // R5
  Stacks[i][STACK_SIZE - 16] = 0x04040404;  // R4
}

//******** OS_AddThread ***************
// add three foregound threads to the scheduler
// Inputs: three pointers to a void/void foreground tasks
// Outputs: 1 if successful, 0 if this thread can not be added
int OS_AddThreads(void (*task0)(void),
                  void (*task1)(void),
                  void (*task2)(void))
{
  int32_t status;
  status = StartCritical();
  tcbs[0].next = &tcbs[1]; // 0 points to 1
  tcbs[1].next = &tcbs[2]; // 1 points to 2
  tcbs[2].next = &tcbs[0]; // 2 points to 0
  SetInitialStack(0);
  Stacks[0][STACK_SIZE - 2] = (int32_t)(task0); // PC
  SetInitialStack(1);
  Stacks[1][STACK_SIZE - 2] = (int32_t)(task1); // PC
  SetInitialStack(2);
  Stacks[2][STACK_SIZE - 2] = (int32_t)(task2); // PC
  RunPt = &tcbs[0];                             // thread 0 will run first
  EndCritical(status);
  return 1; // successful
}

///******** OS_Launch ***************
// start the scheduler, enable interrupts
// Inputs: number of 60ns clock cycles for each time slice
//         (maximum of 24 bits)
// Outputs: none (does not return)
void OS_Launch(uint32_t theTimeSlice)
{
  NVIC_ST_RELOAD_R = theTimeSlice - 1; // reload value
  NVIC_ST_CTRL_R = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                           // start on the first task
}

void Clock_Init(void)
{
  SYSCTL_RCC_R |= 0x810;
  SYSCTL_RCC_R &= ~(0x400020);
}