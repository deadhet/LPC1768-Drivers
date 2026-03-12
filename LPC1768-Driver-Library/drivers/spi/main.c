#include <LPC17xx.h>
#include "spi.h"

int main(void)
{
		volatile int i;
    

    SystemInit();
    SPI_Init();

    while(1)
    {
        uint8_t received = SPI_Transfer(0x55);

        for( i=0;i<100000;i++);
    }
}
