#include <LPC17xx.h>
#include <stdio.h>
#include "adc.h"
#include "uart.h"

int main(void)
{
    uint16_t adc_value;
		int i;	
    SystemInit();

    UART0_Init(9600);
    ADC_Init();

    while(1)
    {
        adc_value = ADC_Read(1);   // Channel 1 (P0.24)

        UART0_Printf("ADC Value: %d \r\n", adc_value);

        for(i=0;i<100000;i++);
    }
}
