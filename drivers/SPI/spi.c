#include "spi.h"
void SPI_Init(void)
{
    /* 1. Power ON SPI */
    LPC_SC->PCONP |= (1 << 8);

    /* 2. Select SPI pins */
    LPC_PINCON->PINSEL0 |= (3 << 30);  // P0.15 SCK
    LPC_PINCON->PINSEL1 |= (3 << 0);   // P0.16 SSEL
    LPC_PINCON->PINSEL1 |= (3 << 2);   // P0.17 MISO
    LPC_PINCON->PINSEL1 |= (3 << 4);   // P0.18 MOSI

    /* 3. SPI Control Register */
    LPC_SPI->SPCR = (1 << 5) |  // Master mode
                    (0 << 3) |  // CPOL = 0
                    (0 << 4) |  // CPHA = 0
                    (8 << 8);   // 8-bit data

    /* 4. Set Clock Divider */
    LPC_SPI->SPCCR = 8;  // Must be even, >= 8
}
uint8_t SPI_Transfer(uint8_t data)
{
    LPC_SPI->SPDR = data;   // Load data

    while (!(LPC_SPI->SPSR & (1 << 7))); // Wait SPIF

    return LPC_SPI->SPDR;   // Return received data
}
