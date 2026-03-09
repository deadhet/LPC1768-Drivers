#ifndef __PWM_H
#define __PWM_H

#include <LPC17xx.h>

void PWM1_Init(uint32_t frequency);
void PWM1_SetDuty(uint8_t duty_percent);

#endif
