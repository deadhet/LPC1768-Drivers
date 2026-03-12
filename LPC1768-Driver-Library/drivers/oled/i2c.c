#include "LPC17xx.h"
#include "i2c.h"

void I2C0_Init(uint32_t clock)
{
    uint32_t pclk;

    LPC_SC->PCONP |= (1 << 19);

    LPC_PINCON->PINSEL1 &= ~((3 << 22) | (3 << 24));
    LPC_PINCON->PINSEL1 |=  ((1 << 22) | (1 << 24));

    pclk = SystemCoreClock / 4;

    LPC_I2C0->I2SCLH = pclk / (2 * clock);
    LPC_I2C0->I2SCLL = pclk / (2 * clock);

    LPC_I2C0->I2CONSET = (1 << 6);
}

/* START condition */
void I2C0_Start(void)
{
    LPC_I2C0->I2CONSET = (1 << 5);
    LPC_I2C0->I2CONCLR = (1 << 3);

    while (!(LPC_I2C0->I2CONSET & (1 << 3)));
}

/* STOP condition */
void I2C0_Stop(void)
{
    LPC_I2C0->I2CONSET = (1 << 4);
    LPC_I2C0->I2CONCLR = (1 << 3);
}

/* Write byte */
uint8_t I2C0_Write(uint8_t data)
{
    LPC_I2C0->I2DAT = data;
    LPC_I2C0->I2CONCLR = (1 << 3);


	while (!(LPC_I2C0->I2CONSET & (1 << 3)));

    if ((LPC_I2C0->I2STAT == 0x18) || (LPC_I2C0->I2STAT == 0x28))
        return I2C_SUCCESS;
    else
        return I2C_ERROR;
}

/* Read with ACK */
uint8_t I2C0_ReadAck(void)
{
    LPC_I2C0->I2CONSET = (1 << 2);
    LPC_I2C0->I2CONCLR = (1 << 3);

    while (!(LPC_I2C0->I2CONSET & (1 << 3)));

    return LPC_I2C0->I2DAT;
}

/* Read with NACK */
uint8_t I2C0_ReadNack(void)
{
    LPC_I2C0->I2CONCLR = (1 << 2);
    LPC_I2C0->I2CONCLR = (1 << 3);

    while (!(LPC_I2C0->I2CONSET & (1 << 3)));

    return LPC_I2C0->I2DAT;
}

/* Write single byte */
uint8_t I2C0_WriteByte(uint8_t slaveAddr, uint8_t data)
{
    I2C0_Start();

    if(!I2C0_Write(slaveAddr & ~1))
        return I2C_ERROR;

    if(!I2C0_Write(data))
        return I2C_ERROR;

    I2C0_Stop();

    return I2C_SUCCESS;
}

/* Read single byte */
uint8_t I2C0_ReadByte(uint8_t slaveAddr, uint8_t *data)
{
    I2C0_Start();

    if(!I2C0_Write(slaveAddr | 1))
        return I2C_ERROR;

    *data = I2C0_ReadNack();

    I2C0_Stop();

    return I2C_SUCCESS;
}

/* Write multiple bytes */
uint8_t I2C0_WriteBuffer(uint8_t slaveAddr, uint8_t *data, uint16_t len)
{
    uint16_t i;

    I2C0_Start();

    if(!I2C0_Write(slaveAddr & ~1))
        return I2C_ERROR;

    for(i=0;i<len;i++)
    {
        if(!I2C0_Write(data[i]))
            return I2C_ERROR;
    }

    I2C0_Stop();

    return I2C_SUCCESS;
}

/* Read multiple bytes */
uint8_t I2C0_ReadBuffer(uint8_t slaveAddr, uint8_t *data, uint16_t len)
{
    uint16_t i;

    I2C0_Start();

    if(!I2C0_Write(slaveAddr | 1))
        return I2C_ERROR;

    for(i=0;i<len-1;i++)
    {
        data[i] = I2C0_ReadAck();
    }

    data[len-1] = I2C0_ReadNack();

    I2C0_Stop();

    return I2C_SUCCESS;
}
