#include "pwm.h"
void PWM1_Init(uint32_t frequency)
{
	//declaration, initialization
		uint32_t pclk;uint32_t period;
	
    /* 1. Power ON PWM1 */
    LPC_SC->PCONP |= (1 << 6);

    /* 2. Set PCLK = CCLK/4 */
    LPC_SC->PCLKSEL0 &= ~(3 << 12);

    /* 3. Select P2.0 as PWM1.1 */
    LPC_PINCON->PINSEL4 &= ~(3 << 0);
    LPC_PINCON->PINSEL4 |=  (1 << 0);

    /* 4. Reset PWM */
    LPC_PWM1->TCR = (1 << 1);

    /* 5. Prescaler = 0 */
    LPC_PWM1->PR = 0;
	
    /* 6. Calculate period */
    pclk = SystemCoreClock / 4;
    period = pclk / frequency;

    LPC_PWM1->MR0 = period;   // Period register
    LPC_PWM1->MR1 = period / 2;  // 50% default duty

    /* 7. Reset on MR0 */
    LPC_PWM1->MCR = (1 << 1);

    /* 8. Enable PWM1.1 output */
    LPC_PWM1->PCR |= (1 << 9);

    /* 9. Latch MR0 and MR1 */
    LPC_PWM1->LER |= (1 << 0) | (1 << 1);

    /* 10. Enable counter and PWM */
    LPC_PWM1->TCR = (1 << 0) | (1 << 3);
}
void PWM1_SetDuty(uint8_t duty_percent)
{
    uint32_t period = LPC_PWM1->MR0;

    LPC_PWM1->MR1 = (period * duty_percent) / 100;

    LPC_PWM1->LER |= (1 << 1);
}
