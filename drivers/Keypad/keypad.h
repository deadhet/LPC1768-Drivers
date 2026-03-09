#ifndef KEYPAD_H
#define KEYPAD_H

#include <LPC17xx.h>
#include <stdint.h>

typedef struct
{
    LPC_GPIO_TypeDef *rowPort;
    LPC_GPIO_TypeDef *colPort;

    uint8_t rowPins[4];
    uint8_t colPins[4];

} KEYPAD_Handle_t;


void Keypad_Init(KEYPAD_Handle_t *keypad);
char Keypad_GetKey(KEYPAD_Handle_t *keypad);

#endif
