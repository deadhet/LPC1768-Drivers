#include "LPC17xx.h"
#include "uart.h"

void delay(void)
{
    volatile int i;
    for(i = 0; i < 500000; i++);
}

int main(void)
{
    char ch;

    SystemInit();           // Initialize system clock
    UART0_Init(9600);       // Initialize UART0 at 9600 baud

    UART0_SendString("UART Driver Test Started\r\n");

    while(1)
    {
        /* Send periodic message */
        UART0_Printf("Hello from LPC1768 UART Driver\r\n");

        delay();

        /* Echo received character */
        if (LPC_UART0->LSR & (1 << 0))   // Data available
        {
            ch = UART0_ReceiveChar();
            UART0_Printf("Received: %c\r\n", ch);
        }
    }
}
