#ifndef __RTC_H
#define __RTC_H

#include <LPC17xx.h>

void RTC_Init(void);
void RTC_SetTime(uint8_t hr, uint8_t min, uint8_t sec);
void RTC_SetDate(uint8_t day, uint8_t month, uint16_t year);
void RTC_GetTime(uint8_t *hr, uint8_t *min, uint8_t *sec);

#endif
