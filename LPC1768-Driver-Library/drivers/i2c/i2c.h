#ifndef __I2C_H
#define __I2C_H

#include "LPC17xx.h"
#include <stdint.h>

#define I2C_SUCCESS 1
#define I2C_ERROR   0

/* Initialization */
void I2C0_Init(uint32_t clock);

/* Basic Control */
void I2C0_Start(void);
void I2C0_Stop(void);

/* Byte Operations */
uint8_t I2C0_Write(uint8_t data);
uint8_t I2C0_ReadAck(void);
uint8_t I2C0_ReadNack(void);

/* High Level Operations */
uint8_t I2C0_WriteByte(uint8_t slaveAddr, uint8_t data);
uint8_t I2C0_ReadByte(uint8_t slaveAddr, uint8_t *data);

uint8_t I2C0_WriteBuffer(uint8_t slaveAddr, uint8_t *data, uint16_t len);
uint8_t I2C0_ReadBuffer(uint8_t slaveAddr, uint8_t *data, uint16_t len);

#endif
