#include "timer.h"

volatile uint32_t led_state = 0;

void Timer0_Init(void)
{
    /* Enable Timer0 power */
    LPC_SC->PCONP |= (1 << 1);

    /* PCLK = CCLK/4 */
    LPC_SC->PCLKSEL0 &= ~(3 << 2);

    /* Reset timer */
    LPC_TIM0->TCR = (1 << 1);

    /* Prescaler so TC increments every 1ms
       Assuming CCLK = 100MHz ? PCLK = 25MHz
    */
    LPC_TIM0->PR = 25000 - 1;

    /* Match every 1000ms (1 second) */
    LPC_TIM0->MR0 = 1000;

    /* Interrupt + Reset on MR0 */
    LPC_TIM0->MCR = (1 << 0) | (1 << 1);

    /* Enable Timer interrupt in NVIC */
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* Start timer */
    LPC_TIM0->TCR = (1 << 0);
}

void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)
    {
        LPC_TIM0->IR = 1;   // clear interrupt

        led_state ^= 1;     // toggle debug variable
    }
}
