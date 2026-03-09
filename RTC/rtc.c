#include "rtc.h"
void RTC_Init(void)
{
    /* 1. Power ON RTC */
    LPC_SC->PCONP |= (1 << 9);

    /* 2. Reset RTC */
    LPC_RTC->CCR = (1 << 1);

    /* 3. Clear reset, enable clock */
    LPC_RTC->CCR = (1 << 0);
}

void RTC_SetTime(uint8_t hr, uint8_t min, uint8_t sec)
{
    LPC_RTC->HOUR = hr;
    LPC_RTC->MIN  = min;
    LPC_RTC->SEC  = sec;
}
void RTC_SetDate(uint8_t day, uint8_t month, uint16_t year)
{
    LPC_RTC->DOM   = day;
    LPC_RTC->MONTH = month;
    LPC_RTC->YEAR  = year;
}
void RTC_GetTime(uint8_t *hr, uint8_t *min, uint8_t *sec)
{
    *hr  = LPC_RTC->HOUR;
    *min = LPC_RTC->MIN;
    *sec = LPC_RTC->SEC;
}
