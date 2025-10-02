/****************************************************************************
Author: Payton Peters
Lab 2: C Drivers for LCD and Keypad
Date Created: January 29, 2025
Last Modified:
Description: This program will initialize the LCD controller and display text 
on the LCD screen. It also includes functions to send commands and data to 
the LCD, as well as initialize ports and handle delays.
****************************************************************************/

/* Initialize LCD controller and flash "HELLO" on the LCD.
 *
 * The connections between the LCD controller of EduPad 
 * board and the Tiva LaunchPad are
 * 
 * PA2-PA5 for LCD D4-D7, respectively. 
 * PE0 for LCD R/S
 * PC6 for LCD EN
 *
 * For simplicity, all delay below 1 ms uses 1 ms.
 *
 * Built and tested with Keil MDK-ARM v5.24a and TM4C_DFP v1.1.0
 */

#include "TM4C123GH6PM.h"
#include "delay.h"
#include "LCD.h"

void LCD_Str(char *str) {
    while(*str) {
        LCD_data(*str++);
    }
}

// initialize LCD controller
void LCD_init(void) {
    PORTS_init();
    delayMs(20); // LCD controller reset sequence

    LCD_nibble_write(0x30, 0);
    delayMs(5);

    LCD_nibble_write(0x30, 0);
    delayMs(1);

    LCD_nibble_write(0x30, 0);
    delayMs(1);

    LCD_nibble_write(0x20, 0); // use 4-bit data mode
    delayMs(1);

    LCD_command(0x28); // set 4-bit data, 2-line, 5x7 font
    LCD_command(0x06); // move cursor right
    LCD_command(0x01); // clear screen, move cursor to home
    LCD_command(0x0F); // turn on display, cursor blinking
}

/* PA2-PA5 for LCD D4-D7, respectively.
* PE0 for LCD R/S
* PC6 for LCD EN
*/
void PORTS_init(void) {
    SYSCTL->RCGCGPIO |= 0x01; // enable clock to GPIOA
    SYSCTL->RCGCGPIO |= 0x10; // enable clock to GPIOE
    SYSCTL->RCGCGPIO |= 0x08; // enable clock to GPIOD
    SYSCTL->RCGCGPIO |= 0x04; // enable clock to GPIOC
    
    // PORTA 5-2 LCD D7-D4
    GPIOA->AMSEL &= ~0x3C; // turn off analog of PORTA 5-2
    GPIOA->DATA &= ~0x3C; // PORTA 5-2 output low
    GPIOA->DIR |= 0x3C; // PORTA 5-2 as GPIO output pins
    GPIOA->DEN |= 0x3C; // PORTA 5-2 as digital pins
    
    // PORTE 0 for LCD R/S
    GPIOE->AMSEL &= ~0x01; // disable analog
    GPIOE->DIR |= 0x01; // set PORTE 0 as output for CS
    GPIOE->DEN |= 0x01; // set PORTE 0 as digital pins
    GPIOE->DATA |= 0x01; // set PORTE 0 idle high
    
    // PORTC 6 for LCD EN
    GPIOC->AMSEL &= ~0x40; // disable analog
    GPIOC->DIR |= 0x40; // set PORTC 6 as output for CS
    GPIOC->DEN |= 0x40; // set PORTC 6 as digital pins
    GPIOC->DATA &= ~0x40; // set PORTC 6 idle low
    GPIOD->AMSEL &= ~0x80; // disable analog
    GPIOD->DIR |= 0x80; // set PORTD 7 as output for CS
    GPIOD->DEN |= 0x80; // set PORTD 7 as digital pins
    GPIOD->DATA |= 0x80; // set PORTD 7 idle high
}

void LCD_nibble_write(char data, unsigned char control) {
    /* populate data bits */
    GPIOA->DIR |= 0x3C; // PORTA 5-2 as GPIO output pins
    GPIOA->DATA &= ~0x3C; // clear data bits
    GPIOA->DATA |= (data & 0xF0) >> 2; // set data bits
    
    /* set R/S bit */
    if (control & RS)
    GPIOE->DATA |= 1;
    else
    GPIOE->DATA &= ~1;
    
    /* pulse E */
    GPIOC->DATA |= 1 << 6;
    delayMs(0);
    GPIOC->DATA &= ~(1 << 6);
    GPIOA->DIR &= ~0x3C; // PORTA 5-2 as GPIO input pins
}

void LCD_command(unsigned char command) {
    LCD_nibble_write(command & 0xF0, 0); // upper nibble first
    LCD_nibble_write(command << 4, 0); // then lower nibble
    
    if (command < 4)
        delayMs(2); // command 1 and 2 needs up to 1.64ms
    else
        delayMs(1); // all others 40 us
}

void LCD_data(char data) {
    LCD_nibble_write(data & 0xF0, RS); // upper nibble first
    LCD_nibble_write(data << 4, RS); // then lower nibble
    delayMs(1);
}
