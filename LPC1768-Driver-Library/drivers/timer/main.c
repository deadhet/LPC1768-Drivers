#include <LPC17xx.h>
#include "timer.h"

int main(void)
{
    SystemInit();

    /* LED output P1.18 */
    LPC_GPIO1->FIODIR |= (1 << 18);

    Timer0_Init();

    while (1)
    {
        if(timer_flag)
            LPC_GPIO1->FIOSET = (1 << 18);
        else
            LPC_GPIO1->FIOCLR = (1 << 18);
    }
}
