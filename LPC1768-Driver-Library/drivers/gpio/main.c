#include <LPC17xx.h>
#include "gpio.h"

#define LED_PIN (1<<18)

void delay(void)
{
		volatile int i;
    for(i=0;i<800000;i++);
}

int main(void)
{
    SystemInit();

    /* Initialize P1.18 as output */
    GPIO_Init_Output(1, LED_PIN);

    while(1)
    {
        /* Turn LED ON */
        GPIO_Set(1, LED_PIN);

        delay();

        /* Turn LED OFF */
        GPIO_Clear(1, LED_PIN);

        delay();
    }
}
