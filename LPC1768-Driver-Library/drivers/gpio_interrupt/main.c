#include <LPC17xx.h>
#include "gpio_interrupt.h"

int main(void)
{
    SystemInit();

    /* Set LED P1.18 as output */
    LPC_GPIO1->FIODIR |= (1 << 18);

    GPIO2_Interrupt_Init();

    while(1)
    {
        /* CPU free, waits for interrupt */
    }
}
