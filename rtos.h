#ifndef RTOS_H
#define RTOS_H

// #include "TM4C123GH6PM.h"
#include "tm4c123gh6pm_def.h"
#include <stdint.h>

// Functions defined in osasm_V2.s

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
#define FIFO_SIZE 10

// Thread Control Block
struct TCB
{
    // Pointer to stack (valid for threads not running)
    int32_t *sp;
    // Linked-list pointer
    struct TCB *next;
    // If non-zero, this thread is blocked by the semaphore pointed to
    uint32_t *blocked;
    // Number of timeslices until the thread is done sleeping (0 indicates ready)
    uint32_t sleep;
};

void OS_FIFO_Init(void);
uint32_t OS_FIFO_Full(void);
uint32_t OS_FIFO_Empty(void);
int32_t OS_FIFO_Put(uint32_t data);
uint32_t OS_FIFO_Get(void);

void OS_Suspend();
void OS_Sleep(uint32_t time);
void OS_Wait(uint32_t *s);
void OS_Signal(uint32_t *s);
void OS_Init();
void SetInitialStack(int i);
int OS_AddThreads(void (*task0)(void),
                  void (*task1)(void),
                  void (*task2)(void));
void OS_Launch(uint32_t theTimeSlice);
void Clock_Init(void);

#endif // RTOS_H