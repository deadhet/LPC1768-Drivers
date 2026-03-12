#include "LPC17xx.h"
#include "uart.h"
#include "lcd.h"

void delay_ms(uint32_t ms)
{
    uint32_t i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<20000;j++);
}

int main(void)
{
    char ch;
    uint32_t idle_counter = 0;

    SystemInit();

    UART0_Init(9600);      // Initialize UART
    LCD_Init();            // Initialize LCD

    LCD_String("UART->LCD Test");
    LCD_Command(0xC0);     // Move to second line

    while(1)
    {
        /* Check if UART received data */
        if (LPC_UART0->LSR & (1<<0))
        {
            ch = UART0_ReceiveChar();
            LCD_Char(ch);

            idle_counter = 0;     // Reset timer since input received
        }

        delay_ms(100);            // 100 ms delay
        idle_counter++;

        /* 5 seconds = 100 ms × 50 */
        if(idle_counter >= 50)
        {
            LCD_Command(0x01);    // Clear LCD
            delay_ms(5);

            LCD_Command(0x80);    // Cursor to first line
            idle_counter = 0;
        }
    }
}
