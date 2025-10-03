#include "rtos.h"
#include "LCD.h"

#define TIMESLICE 32000 // 2ms

typedef enum COLORS
{
	NONE,	 // ___
	RED,	 // __R
	BLUE,	 // _B_
	MAGENTA, // _BR
	GREEN,	 // G__
	YELLOW,	 // G_R
	CYAN,	 // GB_
	WHITE	 // BGR
} COLORS;

char *Color_To_Str(COLORS color)
{
	switch (color)
	{
	case NONE:
		return "   ";
	case GREEN:
		return "GRN";
	case BLUE:
		return "BLU";
	case RED:
		return "RED";
	case YELLOW:
		return "YLW";
	case CYAN:
		return "CYA";
	case MAGENTA:
		return "MGA";
	case WHITE:
		return "WHT";
	default:
		return "ERR";
	}
}

void LCD_Display()
{
	LCD_Str("Hello World");

	for (;;)
	{
	}
}

void LED_Change()
{
	for (;;)
	{
		// See if the queue is empty so we can clear the LED first
		if (OS_FIFO_Empty())
		{
			GPIO_PORTF_DATA_R = NONE << 1;
		}

		// Get the next color. If the queue is empty, then it will simply block until there is a new color
		uint32_t next = OS_FIFO_Get();

		GPIO_PORTF_DATA_R = next << 1;

		// Sleep for 15 seconds (7500 time slices @ 2ms/slice)
		OS_Sleep(7500);
	}
}

void Color_Add()
{
	for (;;)
	{
	}
}

int main(void)
{
	OS_Init(); // initialize, disable interrupts, 16 MHz
	OS_FIFO_Init();

	// Enable all GPIO Ports and wait for them to stabilize
	SYSCTL_RCGCGPIO_R = 0x3F;

	while ((SYSCTL_RCGCGPIO_R & 0x3F) == 0)
	{
	}

	// Enable GBR LED (Port F 1-3)
	GPIO_PORTF_DIR_R |= 0xE;
	GPIO_PORTF_DEN_R |= 0xE;

	LCD_init();

	OS_AddThreads(&LCD_Display, &LED_Change, &Color_Add);
	OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
	return 0;			  // this never executes
}
