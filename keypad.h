//keypad.h

#ifndef keypad_h
#define keypad_h

#include <stdint.h>

struct Row {
    uint8_t direction;
    char keycode[4];
};
typedef const struct Row RowType;

extern RowType ScanTab[5];

void MatrixKeypad_Init(void);
char MatrixKeypad_GetKey(void);
char MatrixKeypad_Scan(void);

#endif
