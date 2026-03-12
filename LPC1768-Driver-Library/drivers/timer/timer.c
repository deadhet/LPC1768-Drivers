#include "timer.h"

/* actual definition */
volatile uint32_t timer_flag = 0;

void Timer0_Init(void)
{
    /* Enable power for Timer0 */
    LPC_SC->PCONP |= (1 << 1);

    /* Peripheral clock = CCLK/4 */
    LPC_SC->PCLKSEL0 &= ~(3 << 2);

    /* Reset timer */
    LPC_TIM0->TCR = (1 << 1);

    /* Prescaler for 1ms tick */
    LPC_TIM0->PR = 25000 - 1;

    /* Match every 500ms */
    LPC_TIM0->MR0 = 2000;

    /* Interrupt + Reset on MR0 */
    LPC_TIM0->MCR = (1 << 0) | (1 << 1);

    /* Enable Timer interrupt */
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* Start timer */
    LPC_TIM0->TCR = 1;
}

void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)
    {
        LPC_TIM0->IR = 1;      // clear interrupt

        timer_flag ^= 1;       // toggle flag
    }
}
