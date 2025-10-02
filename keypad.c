#include "TM4C123GH6PM.h"
#include "Keypad.h"
#include "delay.h"

RowType ScanTab[5] = {
    { 0x01, "123A" },
    { 0x02, "456B" },
    { 0x04, "789C" },
    { 0x08, "E0FD" },
    { 0x00, "" }
};


static char lastKey = 0;
static uint8_t keyPressed = 0;

void MatrixKeypad_Init(void) {
    SYSCTL->RCGCGPIO |= 0x09;
    while ((SYSCTL->PRGPIO & 0x09) != 0x09);

    GPIOA->AFSEL &= ~0x3C;
    GPIOA->AMSEL &= ~0x3C;
    GPIOA->PCTL &= ~0x00FFFF00;
    GPIOA->DEN |= 0x3C;
    GPIOA->DIR &= ~0x3C;
    GPIOA->PUR |= 0x3C;

    GPIOD->AFSEL &= ~0x0F;
    GPIOD->AMSEL &= ~0x0F;
    GPIOD->PCTL &= ~0x0000FFFF;
    GPIOD->DEN |= 0x0F;
    GPIOD->DIR &= ~0x0F;
    GPIOD->DR8R |= 0x0F;
}

char MatrixKeypad_Scan(void) {
    RowType *pt = ScanTab;
    uint32_t column;
    char key = 0;
    uint32_t j;

    while (pt->direction) {
        GPIOD->DIR = pt->direction;
        GPIOD->DATA &= ~0x0F;
        for (j = 1; j <= 10; j++);

        column = (GPIOA->DATA & 0x3C) >> 2;
        for (j = 0; j <= 3; j++) {
            if ((column & 0x01) == 0) {
                key = pt->keycode[j];
                return key;
            }
            column >>= 1;
        }
        pt++;
    }
    return 0;
}

char MatrixKeypad_GetKey(void) {
    char key = MatrixKeypad_Scan();

    if (key != 0 && keyPressed == 0) {
        lastKey = key;
        keyPressed = 1;
        return key;
    }
    if (key == 0) {
        keyPressed = 0;
    }
    return 0;
}
