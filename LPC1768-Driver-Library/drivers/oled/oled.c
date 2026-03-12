#include "oled.h"
#include "i2c.h"
#include "oled_font.h"

static void OLED_WriteCommand(uint8_t cmd)
{
    I2C0_Start();
    I2C0_Write(OLED_I2C_ADDR);
    I2C0_Write(0x00);       // command mode
    I2C0_Write(cmd);
    I2C0_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C0_Start();
    I2C0_Write(OLED_I2C_ADDR);
    I2C0_Write(0x40);       // data mode
    I2C0_Write(data);
    I2C0_Stop();
}

void OLED_Init()
{
    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0x20);
    OLED_WriteCommand(0x10);
    OLED_WriteCommand(0xB0);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x10);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0x7F);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0xF0);
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0x22);
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x20);
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);
}

void OLED_SetCursor(uint8_t x, uint8_t page)
{
    OLED_WriteCommand(0xB0 + page);
    OLED_WriteCommand(((x & 0xF0) >> 4) | 0x10);
    OLED_WriteCommand((x & 0x0F) | 0x01);
}

void OLED_Clear()
{
		uint8_t i, page;
    for(page=0; page<8; page++)
    {
        OLED_SetCursor(0,page);
        for(i=0;i<128;i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_DisplayChar(char c)
{
    uint8_t i;

    for(i=0;i<5;i++)
    {
        OLED_WriteData(Font5x7[c-32][i]);
    }

    OLED_WriteData(0x00);
}

void OLED_DisplayString(char *str)
{
    while(*str)
    {
        OLED_DisplayChar(*str++);
    }
}
