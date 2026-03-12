#ifndef TIMER_H
#define TIMER_H

#include <LPC17xx.h>

void Timer0_Init(void);
extern volatile uint32_t timer_flag;
#endif
