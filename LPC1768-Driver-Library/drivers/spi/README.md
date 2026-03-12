# SPI Driver — LPC1768

## 1. Driver Overview

**SPI (Serial Peripheral Interface)** is a high-speed, full-duplex 4-wire synchronous protocol. Unlike I2C, SPI has no addressing — each slave has its own chip select line. It is faster and simpler, but uses more wires.

**Real-world applications:**
- SPI flash memory (W25Q, M25P)
- SD card interface
- High-speed DACs/ADCs
- Display controllers (some TFT LCDs)
- SPI-based sensors (MAX6675 thermocouple)

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| SPI SCK | PORT0 | P0.15 | Output | Clock |
| SPI SSEL | PORT0 | P0.16 | Output | Chip Select (Active LOW) |
| SPI MISO | PORT0 | P0.17 | Input | Master In Slave Out |
| SPI MOSI | PORT0 | P0.18 | Output | Master Out Slave In |

---

## 3. Registers Used

| Register | Value Used | Description |
|---------|------------|-------------|
| `PCONP` bit 8 | `(1 << 8)` | Enable SPI clock |
| `PINSEL0` [31:30] | `11` | P0.15 = SCK |
| `PINSEL1` [1:0] | `11` | P0.16 = SSEL |
| `PINSEL1` [3:2] | `11` | P0.17 = MISO |
| `PINSEL1` [5:4] | `11` | P0.18 = MOSI |
| `LPC_SPI->SPCR` | `(1<<5)\|(8<<8)` | Master mode, 8-bit data |
| `LPC_SPI->SPCCR` | 8 | Clock counter (must be even ≥ 8) |
| `LPC_SPI->SPDR` | data byte | Data register (TX write, RX read) |
| `LPC_SPI->SPSR` bit 7 | SPIF | Transfer complete flag |

---

## 4. Driver Architecture

```
SPI_Init()
  ├─ PCONP[8] = 1
  ├─ Configure P0.15–P0.18 as SPI pins (PINSEL)
  ├─ SPCR: Master mode (bit5=1), 8-bit data (bits[11:8]=8)
  └─ SPCCR = 8 (clock divider)

SPI_Transfer(data)
  ├─ SPDR = data   (loads TX byte, starts transfer)
  ├─ Poll SPSR bit 7 (SPIF) until = 1  (transfer complete)
  └─ return SPDR   (simultaneous RX byte)
```

---

## 5. Code Walkthrough

```c
void SPI_Init(void)
{
    LPC_SC->PCONP |= (1 << 8);          // Enable SPI peripheral clock

    // Configure all 4 SPI pins (function 11 = SPI)
    LPC_PINCON->PINSEL0 |= (3 << 30);   // P0.15 = SCK
    LPC_PINCON->PINSEL1 |= (3 << 0);    // P0.16 = SSEL
    LPC_PINCON->PINSEL1 |= (3 << 2);    // P0.17 = MISO
    LPC_PINCON->PINSEL1 |= (3 << 4);    // P0.18 = MOSI

    LPC_SPI->SPCR = (1 << 5) |          // Bit 5: MSTR=1 → Master mode
                    (0 << 3) |          // CPOL=0: idle LOW
                    (0 << 4) |          // CPHA=0: sample on first edge (Mode 0)
                    (8 << 8);           // Bits [11:8]: 8-bit data size

    LPC_SPI->SPCCR = 8;                 // SPI clock = PCLK/8. Min value = 8.
}

uint8_t SPI_Transfer(uint8_t data)
{
    LPC_SPI->SPDR = data;                   // Write byte to TX (starts transfer)
    while (!(LPC_SPI->SPSR & (1 << 7)));    // Wait SPIF=1 (transfer done)
    return LPC_SPI->SPDR;                   // Read RX byte (MISO data)
}
// Full-duplex: MOSI sends 'data', MISO receives simultaneously.
```

---

## 6. Hardware Testing Procedure

### Expected Output
Connect MISO to MOSI for **loopback test** — every sent byte returns as received byte. Connect an oscilloscope to P0.15 (SCK) and P0.18 (MOSI) to observe the SPI clock and data signals.

### Init Flow Diagram
```
PCONP[8] = 1
      │
PINSEL: P0.15=SCK, P0.16=SSEL, P0.17=MISO, P0.18=MOSI
      │
SPCR: Master, CPOL=0, CPHA=0, 8-bit
      │
SPCCR = 8 (clock divider)
      │
SPI Ready
```
