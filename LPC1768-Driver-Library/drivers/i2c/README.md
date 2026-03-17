# I2C Driver — LPC1768

## 1. Driver Overview

**I2C (Inter-Integrated Circuit)** is a 2-wire serial protocol developed by Philips (now NXP) for connecting multiple slave peripherals to a single master on a shared bus. Only two lines are needed regardless of how many devices are connected: SDA (Serial Data) and SCL (Serial Clock).

The protocol works using a master-slave model. The master (LPC1768) initiates all transactions and generates the clock. Each slave device has a unique 7-bit address that the master uses to select it. The bus is open-drain — both master and slaves can pull the lines LOW, but a pull-up resistor holds them HIGH when no device is driving. This allows the "wired-AND" mechanism that makes I2C work.

**Real-world applications:**
- EEPROM read/write (24LC series)
- OLED display control (SSD1306)
- Temperature sensors (LM75, DS1621)
- Accelerometers and gyroscopes (MPU6050)
- Real-time clocks (DS1307)

This driver implements I2C0 as master mode only, with configurable clock frequency (up to 400 kHz fast mode).

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| I2C0 SDA | PORT0 | P0.27 | Bidirectional | Serial Data |
| I2C0 SCL | PORT0 | P0.28 | Output | Serial Clock |

Both SDA and SCL require external pull-up resistors (typically 4.7 kΩ to 3.3V). These are already present on the LPC1768 trainer board.

**Bus occupancy:** Shared bus — OLED at address `0x78`, EEPROM at address `0xA0`.

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power Control

| Bit | Value | Description |
|-----|-------|-------------|
| 19 | 1 | Enable I2C0 power and clock |

### LPC_PINCON→PINSEL1 — Pin Select (P0.16–P0.31)

P0.27 occupies bits [23:22] and P0.28 occupies bits [25:24] in PINSEL1:

| Bits | Pin | Value | Function |
|------|-----|-------|---------|
| [23:22] | P0.27 | 01 | SDA0 (I2C data) |
| [25:24] | P0.28 | 01 | SCL0 (I2C clock) |

When set to 01, the pin is connected to the I2C0 peripheral hardware. The I2C pins also use open-drain mode automatically when configured this way.

### LPC_I2C0→I2SCLH / I2SCLL — SCL High and Low Period Registers

These registers set the duration of the SCL clock high and low periods. Both are set to the same value for a 50% duty cycle:

```
I2SCLH = I2SCLL = PCLK / (2 × clock_Hz)
For 100 kHz: I2SCLH = 25,000,000 / (2 × 100,000) = 125
For 400 kHz: I2SCLH = 25,000,000 / (2 × 400,000) = 31
```

The total SCL period is (I2SCLH + I2SCLL) / PCLK. Setting SCLH = SCLL = 125 gives period = 250/25,000,000 = 10 µs = 100 kHz.

### LPC_I2C0→I2CONSET — I2C Control Set Register

Writing a 1 to a bit in I2CONSET sets that control bit. Writing a 0 has no effect. This allows setting specific bits atomically.

| Bit | Field | Description |
|-----|-------|-------------|
| 2 | AA | Assert Acknowledge — send ACK after receiving a byte |
| 3 | SI | Serial Interrupt flag — set by hardware when an I2C event occurs |
| 4 | STO | Stop flag — generate a STOP condition |
| 5 | STA | Start flag — generate a START condition |
| 6 | I2EN | I2C interface enable |

### LPC_I2C0→I2CONCLR — I2C Control Clear Register

Writing a 1 to a bit in I2CONCLR clears that control bit. Used to clear the SI (Serial Interrupt) flag after handling an event, and to clear STA after a start is generated.

### LPC_I2C0→I2DAT — I2C Data Register

For transmission: write the byte to send here before clearing SI. For reception: read this register after SI is set to get the received byte.

### LPC_I2C0→I2STAT — I2C Status Register

Provides state machine status codes that indicate what event just occurred on the bus. Key status codes:

| Code | Meaning |
|------|---------|
| 0x08 | START condition transmitted |
| 0x18 | SLA+W transmitted, ACK received |
| 0x20 | SLA+W transmitted, NACK received (slave not present) |
| 0x28 | Data byte transmitted, ACK received |
| 0x30 | Data byte transmitted, NACK received |
| 0x40 | SLA+R transmitted, ACK received |
| 0x50 | Data byte received, ACK returned |
| 0x58 | Data byte received, NACK returned (last byte) |

## 4. Driver Architecture

The driver is organized in two layers. Low-level functions handle individual bus operations (START, STOP, byte write, byte read). High-level functions compose these into complete transactions.

```
I2C0_Init(clock)
  ├─ PCONP[19] = 1
  ├─ PINSEL1: set P0.27/P0.28 to I2C function
  ├─ I2SCLH = I2SCLL = PCLK / (2 × clock)
  └─ I2CONSET[6] = 1 (enable I2C)

Low-level operations:
  I2C0_Start()        → generate START, wait for SI
  I2C0_Stop()         → generate STOP, clear SI
  I2C0_Write(byte)    → load I2DAT, clear SI (releases bus), wait for SI, check I2STAT
  I2C0_ReadAck()      → set AA, clear SI (releases bus), wait for SI, return I2DAT
  I2C0_ReadNack()     → clear AA, clear SI, wait for SI, return I2DAT

High-level operations:
  I2C0_WriteByte(addr, data)     → Start → Write(addr+W) → Write(data) → Stop
  I2C0_ReadByte(addr, *data)     → Start → Write(addr+R) → ReadNack() → Stop
  I2C0_WriteBuffer(addr, *buf, len) → Start → Write(addr) → Write each byte → Stop
  I2C0_ReadBuffer(addr, *buf, len)  → Start → Write(addr+R) → ReadAck×(n-1) → ReadNack → Stop
```

## 5. Function Reference

| Function | Description |
|----------|-------------|
| `I2C0_Init(uint32_t clock)` | Initialize I2C0 at given clock in Hz (e.g., 100000) |
| `I2C0_Start(void)` | Generate I2C START condition on bus |
| `I2C0_Stop(void)` | Generate I2C STOP condition |
| `I2C0_Write(uint8_t data)` | Send one byte, return I2C_SUCCESS or I2C_ERROR |
| `I2C0_ReadAck(void)` | Receive byte and send ACK (more bytes follow) |
| `I2C0_ReadNack(void)` | Receive byte and send NACK (last byte) |
| `I2C0_WriteByte(addr, data)` | Complete single-byte write transaction |
| `I2C0_ReadByte(addr, *data)` | Complete single-byte read transaction |
| `I2C0_WriteBuffer(addr, *data, len)` | Write multiple bytes |
| `I2C0_ReadBuffer(addr, *data, len)` | Read multiple bytes |

## 6. Code Walkthrough

### Initialization

```c
void I2C0_Init(uint32_t clock)
{
    LPC_SC->PCONP |= (1 << 19);
```

Bit 19 of PCONP enables the I2C0 peripheral clock. Without this, the I2C hardware has no operating clock and register access has no effect.

```c
    LPC_PINCON->PINSEL1 &= ~((3 << 22) | (3 << 24));
    LPC_PINCON->PINSEL1 |=  ((1 << 22) | (1 << 24));
```

PINSEL1 controls P0.16–P0.31. P0.27 is at bits [23:22] and P0.28 is at bits [25:24]. The first line clears both fields to 00 (GPIO). The second line sets bit 22 and bit 24, making both fields 01. Function 01 for these pins is SDA0 and SCL0 respectively.

```c
    pclk = SystemCoreClock / 4;
    LPC_I2C0->I2SCLH = pclk / (2 * clock);
    LPC_I2C0->I2SCLL = pclk / (2 * clock);
```

PCLK is 25 MHz. For 100 kHz I2C: SCLH = SCLL = 25,000,000 / 200,000 = 125. The SCL period is (125+125)/25,000,000 = 10 µs, giving 100 kHz.

```c
    LPC_I2C0->I2CONSET = (1 << 6);
}
```

Setting bit 6 (I2EN) of I2CONSET enables the I2C interface. The I2C peripheral is now active and ready to generate bus conditions.

### Generating a START Condition

```c
void I2C0_Start(void)
{
    LPC_I2C0->I2CONSET = (1 << 5);   // Set STA bit
    LPC_I2C0->I2CONCLR = (1 << 3);   // Clear SI flag → release bus

    while (!(LPC_I2C0->I2CONSET & (1 << 3)));  // Wait for SI to set (START sent)
}
```

Setting bit 5 (STA) tells the hardware to generate a START condition at the next opportunity. Clearing bit 3 (SI) releases the bus — the I2C hardware resumes activity. When the hardware successfully places the START on the bus, it sets SI again and halts. The while loop waits for this to happen. After the loop, I2STAT will contain 0x08, confirming START was transmitted.

### Writing a Byte

```c
uint8_t I2C0_Write(uint8_t data)
{
    LPC_I2C0->I2DAT = data;           // Load byte into data register
    LPC_I2C0->I2CONCLR = (1 << 3);   // Clear SI → hardware begins clocking byte out

    while (!(LPC_I2C0->I2CONSET & (1 << 3)));  // Wait for SI (byte transmitted)

    if ((LPC_I2C0->I2STAT == 0x18) || (LPC_I2C0->I2STAT == 0x28))
        return I2C_SUCCESS;
    else
        return I2C_ERROR;
}
```

The byte to be transmitted is first loaded into I2DAT. Then SI is cleared, which signals the hardware to begin clocking those 8 bits out on SDA, synchronized with SCL. The hardware sets SI once the byte is transmitted and the slave's ACK/NACK bit has been sampled. I2STAT is then checked: 0x18 means the slave address+write was acknowledged, 0x28 means a data byte was acknowledged. Any other code means an error occurred (such as 0x20 for NACK, meaning the slave did not respond to its address).

### Reading with ACK (more bytes follow)

```c
uint8_t I2C0_ReadAck(void)
{
    LPC_I2C0->I2CONSET = (1 << 2);   // Set AA: send ACK after receiving
    LPC_I2C0->I2CONCLR = (1 << 3);   // Clear SI → hardware begins receiving
    while (!(LPC_I2C0->I2CONSET & (1 << 3)));
    return LPC_I2C0->I2DAT;          // Return received byte
}
```

Setting AA (bit 2) tells the hardware to automatically send an ACK pulse after receiving each byte. This signals to the slave "I received your byte, please send the next one." Clearing SI releases the bus so the slave can drive data. When the byte is received, SI is set again. Reading I2DAT retrieves the byte.

### Reading with NACK (last byte)

```c
uint8_t I2C0_ReadNack(void)
{
    LPC_I2C0->I2CONCLR = (1 << 2);   // Clear AA: send NACK after receiving
    LPC_I2C0->I2CONCLR = (1 << 3);   // Clear SI
    while (!(LPC_I2C0->I2CONSET & (1 << 3)));
    return LPC_I2C0->I2DAT;
}
```

Clearing AA causes the hardware to send NACK after the byte is received. This signals to the slave "this is the last byte I want, stop sending." After ReadNack, the master immediately generates a STOP condition.

### High-Level Write Transaction

```c
uint8_t I2C0_WriteByte(uint8_t slaveAddr, uint8_t data)
{
    I2C0_Start();
    if (!I2C0_Write(slaveAddr & ~1))   // Send address with R/W=0 (write)
        return I2C_ERROR;
    if (!I2C0_Write(data))              // Send data byte
        return I2C_ERROR;
    I2C0_Stop();
    return I2C_SUCCESS;
}
```

`slaveAddr & ~1` clears bit 0 of the address, setting the R/W bit to 0 (write mode). For example, for EEPROM at 0xA0: 0xA0 = 10100000, bit 0 is already 0, so `& ~1` has no effect. For reading, `slaveAddr | 1` sets bit 0 to signal read mode.

## 7. Test Program Explanation

The main.c transmits incrementing byte values to address 0xA0 once per second. This address is the standard I2C address for AT24C series EEPROMs. If an EEPROM is connected, each transmission stores data in it. If no slave is present, I2C0_Write will return I2C_ERROR (slave sends NACK). The UART prints "I2C Start" and "Sent Data: XX" for each transaction.

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

### Init Flow Summary
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

### Debugging in Keil
- **Peripheral → I2C0:** watch I2STAT codes during a transaction
  - 0x08 = START sent, 0x18 = addr+ACK, 0x28 = data+ACK
  - 0x20 = addr+NACK: slave not responding, check address and wiring
- **Watch `I2C0->I2STAT` live:** helps identify which step of the protocol is failing
