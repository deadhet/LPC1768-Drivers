#include "adc.h"
void ADC_Init(void)
{
    /* 1. Power ON ADC (PCONP bit 12) */
    LPC_SC->PCONP |= (1 << 12);

    /* 2. Set ADC peripheral clock = CCLK/4 */
    LPC_SC->PCLKSEL0 &= ~(3 << 24);  

    /* 3. Configure P0.24 as AD0.1 */
    LPC_PINCON->PINSEL1 &= ~(3 << 16);
    LPC_PINCON->PINSEL1 |=  (1 << 16);

    /* 4. ADC Control Register setup */
    LPC_ADC->ADCR = (1 << 1)      |  // Select channel AD0.1
                    (4 << 8)      |  // ADC clock divider
                    (1 << 21);       // Enable ADC
}
uint16_t ADC_Read(uint8_t channel)
{
    uint32_t result;

    /* 1. Select desired channel */
    LPC_ADC->ADCR &= ~(0xFF);
    LPC_ADC->ADCR |= (1 << channel);

    /* 2. Start Conversion */
    LPC_ADC->ADCR |= (1 << 24);

    /* 3. Wait for conversion complete */
    while (!(LPC_ADC->ADGDR & (1UL << 31)));

    /* 4. Read Result */
    result = LPC_ADC->ADGDR;

    /* 5. Stop conversion */
    LPC_ADC->ADCR &= ~(7 << 24);

    return (result >> 4) & 0xFFF;
}
