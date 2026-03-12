#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>

void UART0_Init(uint32_t baudrate);
void UART0_SendChar(char ch);
char UART0_ReceiveChar(void);
void UART0_SendString(const char *str);

#endif
