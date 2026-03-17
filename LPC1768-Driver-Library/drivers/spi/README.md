# SPI Driver — LPC1768

## 1. Driver Overview

**SPI (Serial Peripheral Interface)** is a high-speed, full-duplex, 4-wire synchronous communication protocol. Unlike I2C, SPI uses dedicated lines for each direction of data transfer and is significantly faster — it can operate at several MHz compared to I2C's typical 100–400 kHz. Each slave gets its own chip-select (SSEL) line, so there is no addressing scheme.

SPI is called "synchronous" because a shared clock line (SCK) is driven by the master. Both master and slave shift their data bits on the same clock edge, which makes the timing exact and allows much higher speeds than asynchronous UART.

Full-duplex means data flows in both directions simultaneously: while the master sends on MOSI, the slave returns data on MISO over the same clock cycle.

**Real-world applications:**
- SPI flash memory (W25Q series, used in IoT firmware updates)
- SD card interface
- High-speed DACs and ADCs
- TFT LCD display controllers
- MAX6675 thermocouple-to-digital converter

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| SPI SCK | PORT0 | P0.15 | Output | Serial clock (master generates) |
| SPI SSEL | PORT0 | P0.16 | Output | Chip select (active LOW) |
| SPI MISO | PORT0 | P0.17 | Input | Master In Slave Out |
| SPI MOSI | PORT0 | P0.18 | Output | Master Out Slave In |

**Clock modes:** The LPC1768 SPI supports four clock modes defined by CPOL (Clock Polarity) and CPHA (Clock Phase). This driver uses Mode 0 (CPOL=0, CPHA=0): clock idles LOW, data sampled on the first (rising) edge.

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power Control

| Bit | Value | Description |
|-----|-------|-------------|
| 8 | 1 | Enable SPI peripheral clock |

The LPC1768 has one true SPI peripheral (LPC_SPI) and an SSP0/SSP1 peripheral that can also work in SPI mode. This driver uses the legacy SPI interface, enabled at PCONP bit 8.

### LPC_PINCON→PINSEL0 and PINSEL1 — Pin Function Selection

P0.15 is in PINSEL0, while P0.16–P0.18 are in PINSEL1:

| Register | Bits | Pin | Value | Function |
|----------|------|-----|-------|---------|
| PINSEL0 | [31:30] | P0.15 | 11 | SCK0 (SPI clock) |
| PINSEL1 | [1:0] | P0.16 | 11 | SSEL0 (chip select) |
| PINSEL1 | [3:2] | P0.17 | 11 | MISO0 |
| PINSEL1 | [5:4] | P0.18 | 11 | MOSI0 |

All four SPI pins use function code `11` (binary). This is different from UART and I2C which use `01`. Each peripheral uses its own function code depending on which alternate function slot it occupies.

### LPC_SPI→SPCR — SPI Control Register

This is the main configuration register for the SPI peripheral.

| Bit(s) | Field | Value | Description |
|--------|-------|-------|-------------|
| 3 | CPOL | 0 | Clock idle state = LOW |
| 4 | CPHA | 0 | Data sampled on first SCK edge (Mode 0) |
| 5 | MSTR | 1 | Master mode (MCU drives SCK and SSEL) |
| [11:8] | BITS | 8 | Transfer size = 8 bits per frame |

**Clock modes table:**

| CPOL | CPHA | Mode | Clock idle | Sample edge |
|------|------|------|-----------|------------|
| 0 | 0 | 0 | LOW | Rising |
| 0 | 1 | 1 | LOW | Falling |
| 1 | 0 | 2 | HIGH | Falling |
| 1 | 1 | 3 | HIGH | Rising |

### LPC_SPI→SPCCR — SPI Clock Counter Register

Controls the SPI clock frequency relative to the peripheral clock:

```
SPI clock = PCLK / SPCCR

PCLK = 25 MHz, SPCCR = 8:
SPI clock = 25,000,000 / 8 = 3.125 MHz
```

The datasheet requires SPCCR to be even and ≥ 8. Values below 8 are illegal and produce undefined behavior.

### LPC_SPI→SPDR — SPI Data Register

Writing to SPDR loads the TX byte and simultaneously starts the transfer. Reading SPDR after transfer completes returns the byte received on MISO.

### LPC_SPI→SPSR — SPI Status Register

| Bit | Field | Meaning |
|-----|-------|---------|
| 7 | SPIF | 1 = Transfer complete; SPDR contains received data |

## 4. Driver Architecture

```
SPI_Init()
  ├─ PCONP[8] = 1
  ├─ Configure P0.15–P0.18 as SPI pins (PINSEL)
  ├─ SPCR: MSTR=1 (master), CPOL=0, CPHA=0 (Mode 0), BITS=8
  └─ SPCCR = 8 (SPI clock = 3.125 MHz)

SPI_Transfer(data)
  ├─ SPDR = data   (loads TX byte, starts transfer)
  ├─ Poll SPSR bit 7 (SPIF) until = 1 (transfer complete)
  └─ return SPDR   (simultaneous RX byte from MISO)
```

## 5. Function Reference

### `SPI_Init(void)`
Enables SPI, configures pins, sets master mode, 8-bit transfers, Mode 0, 3.125 MHz clock.

### `SPI_Transfer(uint8_t data)`
Transmits `data` on MOSI and simultaneously receives a byte on MISO. Returns received byte.

| Parameter | Type | Description |
|-----------|------|-------------|
| `data` | uint8_t | Byte to transmit on MOSI |
| Returns | uint8_t | Byte received simultaneously on MISO |

## 6. Code Walkthrough

### Enabling the SPI Peripheral

```c
LPC_SC->PCONP |= (1 << 8);
```

Bit 8 of PCONP is the PCSPI bit. Setting it provides the clock signal to the SPI hardware. As with all peripherals, no register writes to SPI have any effect until this step is done.

### Configuring SPI Pin Functions

```c
LPC_PINCON->PINSEL0 |= (3 << 30);   // P0.15 = SCK0
LPC_PINCON->PINSEL1 |= (3 << 0);    // P0.16 = SSEL0
LPC_PINCON->PINSEL1 |= (3 << 2);    // P0.17 = MISO0
LPC_PINCON->PINSEL1 |= (3 << 4);    // P0.18 = MOSI0
```

`(3 << 30)` places binary `11` at bits [31:30] of PINSEL0, selecting alternate function 11 for P0.15, which is SCK0. The same pattern is applied for the remaining three SPI pins in PINSEL1.

Notice that this driver uses `|=` without first clearing the bits. This works only if the reset value of those PINSEL bits is 00 (GPIO), which is the default. In a robust system, you would clear the bits first with `&= ~mask` before setting them with `|=`.

### Configuring the SPI Control Register

```c
LPC_SPI->SPCR = (1 << 5) |    // MSTR = 1: Master mode
                (0 << 3) |    // CPOL = 0: SCK idle LOW
                (0 << 4) |    // CPHA = 0: sample on first edge
                (8 << 8);     // BITS[11:8] = 8: 8-bit transfers
```

Bit 5 (MSTR) being 1 puts the SPI in master mode — the LPC1768 drives SCK and SSEL. Bits 3 and 4 define the clock mode. With `CPOL=0, CPHA=0` (Mode 0), SCK idles LOW and data is sampled on the first rising edge. Bits [11:8] set the transfer size to 8 bits — one complete byte per transfer.

### Setting the Clock Divider

```c
LPC_SPI->SPCCR = 8;
```

The SPI clock is derived from PCLK (25 MHz) divided by SPCCR. With SPCCR=8, the SPI clock runs at 3.125 MHz. The LPC1768 datasheet requires this value to be an even number and at minimum 8. Using 8 gives the fastest valid SPI clock with PCLK=25 MHz.

### Performing a Transfer

```c
uint8_t SPI_Transfer(uint8_t data)
{
    LPC_SPI->SPDR = data;
    while (!(LPC_SPI->SPSR & (1 << 7)));
    return LPC_SPI->SPDR;
}
```

Writing to SPDR loads the byte into the TX shift register and simultaneously triggers the hardware to start clocking. The hardware generates 8 SCK pulses, shifting out the TX byte on MOSI (MSB first) and shifting in 8 bits from MISO. When complete, SPIF (bit 7 of SPSR) is set by hardware. The while loop waits for this. Reading SPDR after SPIF is set returns the byte received on MISO.

This is true full-duplex: MOSI and MISO transfer happen on the same 8 clock cycles. For a loopback test, connecting MISO to MOSI will return the same byte that was sent.

## 7. Hardware Testing Procedure

### Expected Output
Connect P0.17 (MISO) to P0.18 (MOSI) for a loopback test. Every byte sent should be returned unmodified. Connect an oscilloscope to P0.15 (SCK) and P0.18 (MOSI) to see the 3.125 MHz clock and data pattern.

### Init Flow Summary
```
PCONP[8] = 1
      │
PINSEL: P0.15=SCK, P0.16=SSEL, P0.17=MISO, P0.18=MOSI
      │
SPCR: Master mode, CPOL=0, CPHA=0, 8-bit
      │
SPCCR = 8 (3.125 MHz SPI clock)
      │
SPI Ready — call SPI_Transfer() to exchange bytes
```

### Debugging in Keil
- **Peripheral → SPI:** verify SPCR = 0x820 (MSTR=1, BITS=8) after init
- **Watch SPSR bit 7:** should go HIGH after each transfer then clear
- **Oscilloscope on P0.15:** 3.125 MHz clock during SPI_Transfer()
