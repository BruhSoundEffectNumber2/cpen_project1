#include "rtos.h"

#define TIMESLICE 32000	 // 2ms
#define COLOR_CYCLE 1500 // 15s (7500 time slices @ 2ms/slice)

void Init_LCD_Ports(void);
void Init_LCD(void);
void Set_Position(uint32_t POS);
void Display_Msg(char *Str);

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

uint32_t CURRENTLY_SHOWING_COLOR = 0;

char *Color_To_Str(COLORS color)
{
	switch (color)
	{
	case NONE:
		return "NONE";
	case GREEN:
		return "GREN";
	case BLUE:
		return "BLUE";
	case RED:
		return "RED ";
	case YELLOW:
		return "YELW";
	case CYAN:
		return "CYAN";
	case MAGENTA:
		return "MGTA";
	case WHITE:
		return "WHIT";
	default:
		return "ERR ";
	}
}

void LCD_Display()
{
	for (;;)
	{
		// Clear LCD
		Set_Position(0x00);
		Display_Msg("                ");
		Set_Position(0x40);
		Display_Msg("                ");
		Set_Position(0x00);

		// Top: buffer full or current switches pressed
		if (OS_FIFO_Full())
		{
			Display_Msg("  Buffer Full!  ");
		}
		else
		{
			Display_Msg("Switches: ????");
		}

		Set_Position(0x40);

		// Bottom: input a color if nothing, current and next otherwise
		if (OS_FIFO_Empty() && !CURRENTLY_SHOWING_COLOR)
		{
			Display_Msg("Input a Color!");
		}
		else
		{
			// Find the current color based on the GPIO ports
			uint32_t current = (GPIO_PORTF_DATA_R >> 1) & 0x7;
			uint32_t next;
			int32_t has_next = OS_FIFO_Next(&next);

			Display_Msg("C:");
			Display_Msg(Color_To_Str(current));
			Display_Msg(" N:");

			if (has_next == 0)
			{
				Display_Msg(Color_To_Str(next));
			}
			else
			{
				Display_Msg("????");
			}
		}

		OS_Sleep(125); // 4hz
	}
}

void LED_Change()
{
	for (;;)
	{
		// If the queue is empty, clear the LED and wait for the next color cycle
		if (OS_FIFO_Empty())
		{
			OS_FIFO_Put(MAGENTA); // TODO: Remove this line
			CURRENTLY_SHOWING_COLOR = 0;
			GPIO_PORTF_DATA_R = NONE << 1;
			OS_Sleep(COLOR_CYCLE);
		}
		else
		{
			// Get the next color. If the queue is empty, then it will simply block until there is a new color
			uint32_t next = OS_FIFO_Get();
			CURRENTLY_SHOWING_COLOR = 1;

			GPIO_PORTF_DATA_R = next << 1;

			OS_Sleep(COLOR_CYCLE);
		}
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

	Init_LCD_Ports();
	Init_LCD();

	// Enable GBR LED (Port F 1-3)
	GPIO_PORTF_DIR_R |= 0xE;
	GPIO_PORTF_DEN_R |= 0xE;

	// TODO: Remove these
	OS_FIFO_Put(GREEN);
	OS_FIFO_Put(BLUE);
	OS_FIFO_Put(RED);

	OS_AddThreads(&LCD_Display, &LED_Change, &Color_Add);
	OS_Launch(TIMESLICE); // doesn't return, interrupts enabled in here
	return 0;			  // this never executes
}
