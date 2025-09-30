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

// All TCBs
struct TCB tcbs[NUM_THREADS];
// Pointer to actively running thread
struct TCB *RunPt;
// Stacks for all threads
int32_t Stacks[NUM_THREADS][STACK_SIZE];

uint32_t Mail;
uint32_t Lost = 0;
uint32_t Send = 0;

#endif // RTOS_H