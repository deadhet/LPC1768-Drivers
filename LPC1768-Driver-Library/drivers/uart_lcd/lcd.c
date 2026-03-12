#include "LPC17xx.h"
#include "lcd.h"

#define RS (1<<0)
#define EN (1<<1)
#define D4 (1<<9)
#define D5 (1<<10)
#define D6 (1<<14)
#define D7 (1<<15)

void delay(void)
{
    volatile int i;
    for(i=0;i<30000;i++);
}

void LCD_Pulse(void)
{
    LPC_GPIO1->FIOSET = EN;
    delay();
    LPC_GPIO1->FIOCLR = EN;
}

void LCD_Send4Bit(unsigned char data)
{
    LPC_GPIO1->FIOCLR = D4|D5|D6|D7;

    if(data & 0x01) LPC_GPIO1->FIOSET = D4;
    if(data & 0x02) LPC_GPIO1->FIOSET = D5;
    if(data & 0x04) LPC_GPIO1->FIOSET = D6;
    if(data & 0x08) LPC_GPIO1->FIOSET = D7;

    LCD_Pulse();
}

void LCD_Command(unsigned char cmd)
{
    LPC_GPIO1->FIOCLR = RS;

    LCD_Send4Bit(cmd >> 4);
    LCD_Send4Bit(cmd & 0x0F);

    delay();
}

void LCD_Char(unsigned char data)
{
    LPC_GPIO1->FIOSET = RS;

    LCD_Send4Bit(data >> 4);
    LCD_Send4Bit(data & 0x0F);

    delay();
}

void LCD_String(char *str)
{
    while(*str)
        LCD_Char(*str++);
}

void LCD_Init(void)
{
    LPC_GPIO1->FIODIR |= RS|EN|D4|D5|D6|D7;

    delay();

    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
}
