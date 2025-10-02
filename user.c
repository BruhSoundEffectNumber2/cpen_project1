#include "rtos.h"
#include "LCD.h"

#define TIMESLICE 32000

void Task1()
{
	LCD_Str("Hello World");

	for (;;)
	{
	}
}

void Task2()
{
	for (;;)
	{
	}
}

void Task3()
{
	for (;;)
	{
	}
}

int main(void)
{
	OS_Init();				   // initialize, disable interrupts, 16 MHz
	SYSCTL_RCGCGPIO_R |= 0x28; // activate clock for Ports F and D
	while ((SYSCTL_RCGCGPIO_R & 0x28) == 0)
	{
	} // allow time for clock to stabilize

	LCD_init();

	OS_AddThreads(&Task1, &Task2, &Task3);
	OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
	return 0;			  // this never executes
}
