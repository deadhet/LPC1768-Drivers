#include "LPC17xx.h"
#include "i2c.h"
#include "oled.h"

int main()
{
    SystemInit();

    I2C0_Init(100000);

    OLED_Init();

    while(1);
}
