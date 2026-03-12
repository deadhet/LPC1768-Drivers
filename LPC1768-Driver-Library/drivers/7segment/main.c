#include <LPC17xx.h>
#include "7seg.h"

void delay_ms(unsigned int ms)
{
    unsigned int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<20000;j++);
}

int main(void)
{
    int i;

    SystemInit();          // Initialize clock
    SevenSeg_Init();       // Initialize 7-seg driver

    while(1)
    {
        /* Count from 0 to 9 */
        for(i=0;i<=9;i++)
        {
            SevenSeg_Display(i);
            delay_ms(200);
        }
    }
}
