#include "lcd.h"

// Pin Definitions
#define RS (1<<0)
#define EN (1<<1)
#define D4 (1<<9)
#define D5 (1<<10)
#define D6 (1<<14)
#define D7 (1<<15)

// Small delay
void LCD_Delay(void)
{
		volatile int i;
    for(i=0;i<50000;i++);
}

// Send 4 bits to LCD
void LCD_SendNibble(unsigned char nibble)
{
    LPC_GPIO1->FIOCLR = D4|D5|D6|D7;

    if(nibble & 0x01) LPC_GPIO1->FIOSET = D4;
    if(nibble & 0x02) LPC_GPIO1->FIOSET = D5;
    if(nibble & 0x04) LPC_GPIO1->FIOSET = D6;
    if(nibble & 0x08) LPC_GPIO1->FIOSET = D7;

    LPC_GPIO1->FIOSET = EN;
    LCD_Delay();
    LPC_GPIO1->FIOCLR = EN;
}

// Send Command
void LCD_Command(unsigned char cmd)
{
    LPC_GPIO1->FIOCLR = RS;

    LCD_SendNibble(cmd >> 4);
    LCD_SendNibble(cmd & 0x0F);

    LCD_Delay();
}

// Send Data
void LCD_Data(unsigned char data)
{
    LPC_GPIO1->FIOSET = RS;

    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);

    LCD_Delay();
}

// Send String
void LCD_String(char *str)
{
    while(*str)
    {
        LCD_Data(*str++);
    }
}

// Clear LCD
void LCD_Clear(void)
{
    LCD_Command(0x01);
}

// Set Cursor Position
void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;

    if(row == 0)
        address = 0x80 + col;
    else
        address = 0xC0 + col;

    LCD_Command(address);
}

// Initialize LCD
void LCD_Init(void)
{
		volatile int i;
    for(i=0;i<100000;i++);
    // Configure pins as GPIO
    LPC_PINCON->PINSEL3 &= ~(0x3<<0);   // P1.0
    LPC_PINCON->PINSEL3 &= ~(0x3<<2);   // P1.1
    LPC_PINCON->PINSEL3 &= ~(0x3<<18);  // P1.9
    LPC_PINCON->PINSEL3 &= ~(0x3<<20);  // P1.10
    LPC_PINCON->PINSEL3 &= ~(0x3<<28);  // P1.14
    LPC_PINCON->PINSEL3 &= ~(0x3<<30);  // P1.15

    // Set pins as output
    LPC_GPIO1->FIODIR |= RS|EN|D4|D5|D6|D7;

    LCD_Delay();

    // 4-bit mode init sequence
    LCD_Command(0x02);
    LCD_Command(0x28);  // 4-bit, 2-line
    LCD_Command(0x0C);  // Display ON
    LCD_Command(0x06);  // Entry mode
    LCD_Clear();
}
