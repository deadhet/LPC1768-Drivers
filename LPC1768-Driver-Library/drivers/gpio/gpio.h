#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <LPC17xx.h>

void GPIO_Init_Output(uint8_t port, uint32_t pins);
void GPIO_Set(uint8_t port, uint32_t pins);
void GPIO_Clear(uint8_t port, uint32_t pins);
uint32_t GPIO_Read(uint8_t port);

#endif
