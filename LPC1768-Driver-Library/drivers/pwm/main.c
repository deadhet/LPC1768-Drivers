#include <LPC17xx.h>
#include "pwm.h"

int main(void)
{
	int duty;
	volatile int i;
    SystemInit();

    PWM1_Init(1000);   // 1kHz PWM

    while(1)
    {
        for(duty=10; duty<=90; duty+=10)
        {
            PWM1_SetDuty(duty);

            for(i=0;i<500000;i++);
        }
    }
}
