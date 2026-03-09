#include "gpio.h"

void GPIO_Init_Output(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIODIR |= pins;
    if(port == 1) LPC_GPIO1->FIODIR |= pins;
    if(port == 2) LPC_GPIO2->FIODIR |= pins;
    if(port == 3) LPC_GPIO3->FIODIR |= pins;
}

void GPIO_Set(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIOSET = pins;
    if(port == 1) LPC_GPIO1->FIOSET = pins;
    if(port == 2) LPC_GPIO2->FIOSET = pins;
    if(port == 3) LPC_GPIO3->FIOSET = pins;
}

void GPIO_Clear(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIOCLR = pins;
    if(port == 1) LPC_GPIO1->FIOCLR = pins;
    if(port == 2) LPC_GPIO2->FIOCLR = pins;
    if(port == 3) LPC_GPIO3->FIOCLR = pins;
}

uint32_t GPIO_Read(uint8_t port)
{
    if(port == 0) return LPC_GPIO0->FIOPIN;
    if(port == 1) return LPC_GPIO1->FIOPIN;
    if(port == 2) return LPC_GPIO2->FIOPIN;
    if(port == 3) return LPC_GPIO3->FIOPIN;

    return 0;
}
