#include "LPC17xx.h"
#include "uart.h"

#include <stdarg.h>
#include <stdio.h>

void UART0_Init(uint32_t baudrate)
{
    uint32_t pclk, divisor;

    /* 1. Power ON UART0 (Bit 3 in PCONP) */
    LPC_SC->PCONP |= (1 << 3);

    /* 2. Set P0.2 as TXD0 and P0.3 as RXD0 */
    LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6)); // Clear bits
    LPC_PINCON->PINSEL0 |=  ((1 << 4) | (1 << 6)); // Set function 01

    /* 3. Set PCLK for UART0 = CCLK/4 (default already) */
    pclk = SystemCoreClock / 4;

    /* 4. Calculate Baud Rate Divisor */
    divisor = pclk / (16 * baudrate);

    /* 5. Enable DLAB to access DLL & DLM */
    LPC_UART0->LCR = (1 << 7); // DLAB = 1

    /* 6. Set Divisor Registers */
    LPC_UART0->DLL = divisor & 0xFF;
    LPC_UART0->DLM = (divisor >> 8) & 0xFF;

    /* 7. 8-bit, 1 Stop bit, No parity */
    LPC_UART0->LCR = (3 << 0); // 8-bit, DLAB=0 the 0th and the 1st bit are for word length selection

    /* 8. Enable FIFO and Reset TX/RX FIFO */
    LPC_UART0->FCR = (1 << 0) | (1 << 1) | (1 << 2);
}
void UART0_SendChar(char ch)
{
    /* Wait until THR is empty */
    while (!(LPC_UART0->LSR & (1 << 5)));//Transmitting HOlding Register

    /* Load character into THR */
    LPC_UART0->THR = ch;
}

char UART0_ReceiveChar(void)
{
    /* Wait until data is received */
    while (!(LPC_UART0->LSR & (1 << 0)));//0th bit is the RDR that is data is received and ready

    return LPC_UART0->RBR;//receiver buffer register
}


void UART0_SendString(const char *str)
{
    while (*str)
    {
        UART0_SendChar(*str++);
    }
}

void UART0_Printf(const char *format, ...)
{
    char buffer[100];
    __va_list args;

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    UART0_SendString(buffer);
}
