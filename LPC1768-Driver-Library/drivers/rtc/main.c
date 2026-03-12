#include <LPC17xx.h>
#include <stdio.h>
#include "rtc.h"
#include "uart.h"

int main(void)
{
		volatile int i;
    uint8_t hr, min, sec;

    SystemInit();

    UART0_Init(9600);

    RTC_Init();

    /* Set initial date and time */
    RTC_SetDate(21, 5, 2026);
    RTC_SetTime(10, 30, 0);

    while(1)
    {
        RTC_GetTime(&hr, &min, &sec);

        UART0_Printf("Time: %02d:%02d:%02d\r\n", hr, min, sec);

        for(i=0;i<800000;i++);
    }
}
