#include "LPC17xx.h"
#include "i2c.h"
#include "uart.h"

void delay_ms(uint32_t ms)
{
    uint32_t i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<20000;j++);
}

int main(void)
{
    uint8_t data = 0x55;

    SystemInit();

    UART0_Init(9600);
    I2C0_Init(100000);

    UART0_Printf("I2C Logic Analyzer Test\r\n");

    while(1)
    {
        UART0_Printf("I2C Start\r\n");

        I2C0_Start();

        I2C0_Write(0xA0);      // Slave address + Write

        I2C0_Write(data);      // Data byte

        I2C0_Stop();

        UART0_Printf("Sent Data: %X\r\n", data);

        data++;

        delay_ms(1000);        // 1 second gap for analyzer
    }
}
