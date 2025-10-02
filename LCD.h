#ifndef LCD_h
#define LCD_h

#include "TM4C123GH6PM.h"
#include <stdint.h>

#define RS 1    // BIT0 mask for register select signal
#define EN 2    // BIT1 mask for enable signal
#define DEBOUNCE_DELAY 20

// Function Declarations
void LCD_nibble_write(char data, unsigned char control);
void LCD_command(unsigned char command);
void LCD_data(char data);
void LCD_init(void);
void PORTS_init(void);
void LCD_Str(char *str);

#endif 
