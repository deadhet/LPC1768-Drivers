#ifndef __ADC_H
#define __ADC_H

#include <LPC17xx.h>

void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);

#endif
