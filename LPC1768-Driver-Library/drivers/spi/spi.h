#ifndef __SPI_H
#define __SPI_H

#include <LPC17xx.h>

void SPI_Init(void);
uint8_t SPI_Transfer(uint8_t data);

#endif
