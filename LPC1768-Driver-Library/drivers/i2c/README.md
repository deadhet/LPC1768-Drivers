# I2C Driver — LPC1768

## 1. Driver Overview

**I2C (Inter-Integrated Circuit)** is a 2-wire serial protocol developed by Philips (now NXP) for connecting multiple peripherals on a shared bus. Only two wires are needed for up to 127 slave devices.

**Real-world applications:**
- EEPROM read/write (24LC series)
- OLED display control (SSD1306)
- Temperature sensors (LM75, DS1621)
- Accelerometers, gyroscopes (MPU6050)

This driver implements I2C0 as **master mode** with configurable clock frequency.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| I2C0 SDA | PORT0 | P0.27 | Bidir | Serial Data |
| I2C0 SCL | PORT0 | P0.28 | Output | Serial Clock |

Shared bus — OLED at `0x78`, EEPROM at `0xA0`.

---

## 3. Registers Used

| Register | Purpose | Value Used |
|---------|---------|------------|
| `LPC_SC->PCONP` bit 19 | Enable I2C0 power | `(1 << 19)` |
| `LPC_PINCON->PINSEL1` [23:22] | P0.27 = SDA0 | `01` |
| `LPC_PINCON->PINSEL1` [25:24] | P0.28 = SCL0 | `01` |
| `LPC_I2C0->I2SCLH` | SCL high period | `pclk / (2 × clock)` |
| `LPC_I2C0->I2SCLL` | SCL low period | `pclk / (2 × clock)` |
| `LPC_I2C0->I2CONSET` bit 6 | Enable I2C | `(1 << 6)` |
| `LPC_I2C0->I2CONSET` bit 5 | Generate START | `(1 << 5)` |
| `LPC_I2C0->I2CONSET` bit 4 | Generate STOP | `(1 << 4)` |
| `LPC_I2C0->I2CONCLR` bit 3 | Clear SI flag | `(1 << 3)` |
| `LPC_I2C0->I2DAT` | TX/RX data | byte to send |
| `LPC_I2C0->I2STAT` | Status code | 0x18=SLA+W ACK, 0x28=data ACK |

**SCL Frequency calculation:**
```
I2SCLH = I2SCLL = PCLK / (2 × clock_Hz)
For 100 kHz: I2SCLH = 25,000,000 / (2 × 100,000) = 125
```

---

## 4. Driver Architecture

```
I2C0_Init(clock)
  ├─ PCONP[19] = 1
  ├─ PINSEL1: set P0.27/P0.28 to I2C function
  ├─ I2SCLH = I2SCLL = PCLK / (2 × clock)
  └─ I2CONSET[6] = 1 (enable)

High-Level Write:     I2C0_WriteByte(addr, data)
  └─ I2C0_Start()     → generate START on bus
  └─ I2C0_Write(addr & ~1)  → send SLA+W, wait ACK
  └─ I2C0_Write(data)       → send data byte, wait ACK
  └─ I2C0_Stop()            → generate STOP

High-Level Read:      I2C0_ReadByte(addr, &data)
  └─ I2C0_Start()     → generate START
  └─ I2C0_Write(addr | 1)   → send SLA+R, wait ACK
  └─ I2C0_ReadNack()         → receive byte, send NACK
  └─ I2C0_Stop()
```

---

## 5. Function Reference

| Function | Description |
|----------|-------------|
| `I2C0_Init(uint32_t clock)` | Initialize I2C0 at given clock (e.g., 100000 for 100 kHz) |
| `I2C0_Start(void)` | Generate I2C START condition |
| `I2C0_Stop(void)` | Generate I2C STOP condition |
| `I2C0_Write(uint8_t data)` → returns I2C_SUCCESS/ERROR | Send byte, wait for SI, check ACK |
| `I2C0_ReadAck(void)` → uint8_t | Receive byte and send ACK |
| `I2C0_ReadNack(void)` → uint8_t | Receive byte and send NACK (last byte) |
| `I2C0_WriteByte(addr, data)` | Full write transaction |
| `I2C0_ReadByte(addr, *data)` | Full read transaction |
| `I2C0_WriteBuffer(addr, *data, len)` | Write multiple bytes |
| `I2C0_ReadBuffer(addr, *data, len)` | Read multiple bytes |

---

## 6. Code Walkthrough

```c
uint8_t I2C0_Write(uint8_t data)
{
    LPC_I2C0->I2DAT = data;          // Load byte into data register
    LPC_I2C0->I2CONCLR = (1 << 3);  // Clear SI flag → releases bus, sends byte

    while (!(LPC_I2C0->I2CONSET & (1 << 3))); // Wait for SI to set again (byte sent)

    // Check I2STAT to confirm ACK received:
    if ((LPC_I2C0->I2STAT == 0x18) ||  // 0x18 = SLA+W sent + ACK received
        (LPC_I2C0->I2STAT == 0x28))     // 0x28 = Data sent + ACK received
        return I2C_SUCCESS;
    else
        return I2C_ERROR;               // NACK or bus error
}
```

---

## 7. Test Program (main.c)

Transmits incrementing bytes to address `0xA0` (EEPROM/device) once per second. Use a logic analyzer on SDA/SCL to observe the I2C frames. UART prints "Sent Data: XX".

---

## 8. Hardware Testing Procedure

### Build & Flash
1. Add: `i2c.c`, `uart.c`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c`
2. Build → flash `i2c.hex` → RESET

### Expected Output (Serial Terminal)
```
I2C Logic Analyzer Test
I2C Start
Sent Data: 55
I2C Start
Sent Data: 56
...
```

### Init Flow Diagram
```
PCONP[19] = 1
      │
PINSEL1: P0.27=SDA0, P0.28=SCL0
      │
I2SCLH = I2SCLL = 125 (100 kHz)
      │
I2CONSET[6] = 1 (I2C enabled)
      │
I2C0 Ready
```

### Debugging
- **Peripheral → I2C0:** Watch I2STAT codes during transaction
- 0x08 = START sent, 0x18 = addr+ACK, 0x28 = data+ACK
- **If stuck at 0x20:** slave not responding (check address/wiring)
