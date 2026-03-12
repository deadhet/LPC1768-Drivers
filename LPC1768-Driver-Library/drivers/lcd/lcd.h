#ifndef LCD_H
#define LCD_H

#include <LPC17xx.h>
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_String(char *str);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);

#endif
