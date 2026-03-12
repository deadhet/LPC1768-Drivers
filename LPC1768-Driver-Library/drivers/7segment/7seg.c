#include <LPC17xx.h>
#include "7seg.h"

const uint8_t seg_pattern[10] =
{
    0x3F, //0
    0x06, //1
    0x5B, //2
    0x4F, //3
    0x66, //4
    0x6D, //5
    0x7D, //6
    0x07, //7
    0x7F, //8
    0x6F  //9
};

void SevenSeg_Init(void)
{
    LPC_GPIO2->FIODIR |= 0xFF;  // Segment lines
}

void SevenSeg_Display(uint16_t number)
{
    uint8_t digit = number % 10;
    LPC_GPIO2->FIOPIN = seg_pattern[digit];
}
