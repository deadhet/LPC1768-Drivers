#include "gpio_interrupt.h"
volatile uint32_t gpio_irq_flag = 0;
void GPIO2_Interrupt_Init(void)
{
    /* 1. Set P2.10 as input */
    LPC_GPIO2->FIODIR &= ~(1 << 10);

    /* 2. Enable Rising Edge Interrupt */
    LPC_GPIOINT->IO2IntEnR |= (1 << 10);

    /* 3. Enable GPIO interrupt in NVIC */
    NVIC_EnableIRQ(EINT3_IRQn);
}
void EINT3_IRQHandler(void)
{
    if (LPC_GPIOINT->IO2IntStatR & (1 << 10))
    {
        /* Clear interrupt */
        LPC_GPIOINT->IO2IntClr = (1 << 10);
				
        /* Example Action: Toggle LED P1.18 */
        LPC_GPIO1->FIOPIN ^= (1 << 18);
				gpio_irq_flag ^= 1;
    }
}
