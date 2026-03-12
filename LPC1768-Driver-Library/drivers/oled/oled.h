#ifndef __OLED_H
#define __OLED_H

#include "LPC17xx.h"

#define OLED_WIDTH   128
#define OLED_HEIGHT  64

#define OLED_I2C_ADDR  0x78

void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t x, uint8_t page);
void OLED_DisplayChar(char c);
void OLED_DisplayString(char *str);

#endif
