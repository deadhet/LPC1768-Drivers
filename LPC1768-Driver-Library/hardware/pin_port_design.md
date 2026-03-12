# Pin Port Design Reference

> This document is derived from the **LPC1768 Trainer Kit User Manual** (RDL Technologies) and the driver source code in this repository. It provides the definitive pin and port mapping for all onboard peripherals.

---

## 1. LPC1768 Port Architecture

The LPC1768 exposes GPIO through **5 ports** (PORT0–PORT4), each controlled through fast I/O registers in the AHB area.

### Port Base Addresses

| Port | Base Address | Available Pins |
|------|-------------|----------------|
| PORT0 | 0x2009C000 | P0.0–P0.31 (some reserved) |
| PORT1 | 0x2009C020 | P1.0–P1.31 (some reserved) |
| PORT2 | 0x2009C040 | P2.0–P2.13 |
| PORT3 | 0x2009C060 | P3.25–P3.26 only |
| PORT4 | 0x2009C080 | P4.28–P4.29 only |

### Fast I/O Register Set (same structure for all ports)

| Register | Offset | R/W | Description |
|----------|--------|-----|-------------|
| `FIODIRx` | +0x00 | R/W | Direction: **1=Output**, 0=Input |
| `FIOMASKx` | +0x10 | R/W | Mask: write 1 to exclude pin from FIOPIN/FIOSET/FIOCLR |
| `FIOPINx` | +0x14 | R/W | Read actual pin level / Write all output state |
| `FIOSETx` | +0x18 | W | Set individual pins HIGH (1=set, 0=no change) |
| `FIOCLRx` | +0x1C | W | Set individual pins LOW (1=clear, 0=no change) |

### Why FIOSET/FIOCLR instead of FIOPIN?

```c
// SAFE — Atomic. No race condition with interrupts.
LPC_GPIO1->FIOSET = (1 << 18);   // Only sets bit 18, others unchanged
LPC_GPIO1->FIOCLR = (1 << 18);   // Only clears bit 18, others unchanged

// UNSAFE — Read-Modify-Write. Interrupt between read and write = corruption
LPC_GPIO1->FIOPIN |= (1 << 18);  // Three instructions: READ, OR, WRITE
```

---

## 2. PORT0 — Communication & Analog

PORT0 is the primary communication port. Most serial peripheral pins are here.

```
PORT0 Diagram
══════════════════════════════════════════════════════════════════
Bit  | Pin   | Function (our drivers)    | Direction | Peripheral
─────┼───────┼───────────────────────────┼───────────┼──────────
  2  | P0.2  | UART0 TXD                 | Output    | UART
  3  | P0.3  | UART0 RXD                 | Input     | UART
 15  | P0.15 | SPI SCK                   | Output    | SPI
 16  | P0.16 | SPI SSEL (chip select)    | Output    | SPI
 17  | P0.17 | SPI MISO                  | Input     | SPI
 18  | P0.18 | SPI MOSI                  | Output    | SPI
 24  | P0.24 | AD0.1 (Potentiometer)     | Analog In | ADC
 27  | P0.27 | I2C0 SDA                  | Bidir     | I2C / OLED
 28  | P0.28 | I2C0 SCL                  | Output    | I2C / OLED
══════════════════════════════════════════════════════════════════
```

**Pin function configuration (PINSEL registers):**

| Pin | PINSEL | Bits | Value | Function |
|-----|--------|------|-------|---------|
| P0.2 | PINSEL0 | [5:4] | 01 | TXD0 |
| P0.3 | PINSEL0 | [7:6] | 01 | RXD0 |
| P0.15 | PINSEL0 | [31:30] | 11 | SCK |
| P0.16 | PINSEL1 | [1:0] | 11 | SSEL |
| P0.17 | PINSEL1 | [3:2] | 11 | MISO |
| P0.18 | PINSEL1 | [5:4] | 11 | MOSI |
| P0.24 | PINSEL1 | [17:16] | 01 | AD0.1 |
| P0.27 | PINSEL1 | [23:22] | 01 | SDA0 |
| P0.28 | PINSEL1 | [25:24] | 01 | SCL0 |

---

## 3. PORT1 — LED & LCD

PORT1 drives the onboard LEDs and the 16×2 LCD display.

```
PORT1 Diagram
══════════════════════════════════════════════════════════════════
Bit  | Pin   | Function (our drivers)    | Direction | Peripheral
─────┼───────┼───────────────────────────┼───────────┼──────────
  0  | P1.0  | LCD RS (Register Select)  | Output    | LCD
  1  | P1.1  | LCD EN (Enable)           | Output    | LCD
  9  | P1.9  | LCD D4 (Data bit 4)       | Output    | LCD
 10  | P1.10 | LCD D5 (Data bit 5)       | Output    | LCD
 14  | P1.14 | LCD D6 (Data bit 6)       | Output    | LCD
 15  | P1.15 | LCD D7 (Data bit 7)       | Output    | LCD
 18  | P1.18 | LED 1                     | Output    | GPIO/Timer/ISR
 19  | P1.19 | LED 2                     | Output    | GPIO
 20  | P1.20 | LED 3                     | Output    | GPIO
 21  | P1.21 | LED 4                     | Output    | GPIO
══════════════════════════════════════════════════════════════════
```

**LCD 4-bit connection diagram:**

```
LPC1768 PORT1          HD44780 16x2 LCD
═════════════          ═════════════════
P1.0 (RS)  ────────→  Pin 4  (RS)
P1.1 (EN)  ────────→  Pin 6  (E)
GND        ────────→  Pin 5  (R/W)  [always write mode]
P1.9  (D4) ────────→  Pin 11 (DB4)
P1.10 (D5) ────────→  Pin 12 (DB5)
P1.14 (D6) ────────→  Pin 13 (DB6)
P1.15 (D7) ────────→  Pin 14 (DB7)
3.3V       ────────→  Pin 2  (VDD)
GND        ────────→  Pin 1  (VSS)
Contrast   ────────→  Pin 3  (V0)  [10K pot]
```

---

## 4. PORT2 — Interrupts, PWM, 7-Segment

```
PORT2 Diagram
══════════════════════════════════════════════════════════════════
Bit  | Pin   | Function (our drivers)    | Direction | Peripheral
─────┼───────┼───────────────────────────┼───────────┼──────────
  0  | P2.0  | PWM1 Ch.1 / 7-SEG Seg A  | Output    | PWM / 7SEG
  1  | P2.1  | 7-SEG Segment B           | Output    | 7SEG
  2  | P2.2  | 7-SEG Segment C           | Output    | 7SEG
  3  | P2.3  | 7-SEG Segment D           | Output    | 7SEG
  4  | P2.4  | 7-SEG Segment E           | Output    | 7SEG
  5  | P2.5  | 7-SEG Segment F           | Output    | 7SEG
  6  | P2.6  | 7-SEG Segment G           | Output    | 7SEG
  7  | P2.7  | 7-SEG Decimal Point       | Output    | 7SEG
 10  | P2.10 | User Button / GPIO IRQ    | Input     | GPIO_INT
══════════════════════════════════════════════════════════════════
```

> ⚠️ **Port 2 Conflict:** P2.0 is used as both PWM1.1 output AND 7-segment segment A. Do not use both drivers simultaneously. When using PWM, configure P2.0 as PWM function (PINSEL4). When using 7-segment, configure P2.0 as GPIO.

---

## 5. Complete Board Peripheral Pin Mapping Table

| Peripheral | Port | Pin(s) | PINSEL | Direction | Description |
|-----------|------|--------|--------|-----------|-------------|
| LED 1 | P1 | 18 | GPIO (00) | Output | Onboard LED 1 — Active HIGH |
| LED 2 | P1 | 19 | GPIO (00) | Output | Onboard LED 2 |
| LED 3 | P1 | 20 | GPIO (00) | Output | Onboard LED 3 |
| LED 4 | P1 | 21 | GPIO (00) | Output | Onboard LED 4 |
| UART0 TX | P0 | 2 | PINSEL0 [5:4]=01 | Output | Serial transmit |
| UART0 RX | P0 | 3 | PINSEL0 [7:6]=01 | Input | Serial receive |
| ADC CH1 | P0 | 24 | PINSEL1 [17:16]=01 | Analog In | Potentiometer input |
| I2C0 SDA | P0 | 27 | PINSEL1 [23:22]=01 | Bidir | I2C data (OLED, EEPROM) |
| I2C0 SCL | P0 | 28 | PINSEL1 [25:24]=01 | Output | I2C clock |
| SPI SCK | P0 | 15 | PINSEL0 [31:30]=11 | Output | SPI clock |
| SPI SSEL | P0 | 16 | PINSEL1 [1:0]=11 | Output | SPI chip select |
| SPI MISO | P0 | 17 | PINSEL1 [3:2]=11 | Input | SPI data in |
| SPI MOSI | P0 | 18 | PINSEL1 [5:4]=11 | Output | SPI data out |
| LCD RS | P1 | 0 | PINSEL3 [1:0]=00 | Output | LCD register select |
| LCD EN | P1 | 1 | PINSEL3 [3:2]=00 | Output | LCD enable strobe |
| LCD D4 | P1 | 9 | PINSEL3 [19:18]=00 | Output | LCD data bit 4 |
| LCD D5 | P1 | 10 | PINSEL3 [21:20]=00 | Output | LCD data bit 5 |
| LCD D6 | P1 | 14 | PINSEL3 [29:28]=00 | Output | LCD data bit 6 |
| LCD D7 | P1 | 15 | PINSEL3 [31:30]=00 | Output | LCD data bit 7 |
| PWM1.1 | P2 | 0 | PINSEL4 [1:0]=01 | Output | PWM output channel 1 |
| 7-SEG A | P2 | 0 | GPIO (00) | Output | Segment A (shared with PWM!) |
| 7-SEG B | P2 | 1 | GPIO (00) | Output | Segment B |
| 7-SEG C | P2 | 2 | GPIO (00) | Output | Segment C |
| 7-SEG D | P2 | 3 | GPIO (00) | Output | Segment D |
| 7-SEG E | P2 | 4 | GPIO (00) | Output | Segment E |
| 7-SEG F | P2 | 5 | GPIO (00) | Output | Segment F |
| 7-SEG G | P2 | 6 | GPIO (00) | Output | Segment G |
| 7-SEG DP | P2 | 7 | GPIO (00) | Output | Decimal point |
| User Button | P2 | 10 | GPIO (00) | Input | Active LOW, ISR-capable |
| OLED SDA | P0 | 27 | PINSEL1 [23:22]=01 | Bidir | OLED SSD1306 (shared I2C) |
| OLED SCL | P0 | 28 | PINSEL1 [25:24]=01 | Output | OLED clock |

---

## 6. Keypad Pin Assignment

The 4×4 matrix keypad uses independent GPIO pins for rows (output) and columns (input). The exact pin assignment depends on the trainer board wiring — connect as shown in the keypad driver's `main.c`.

The `KEYPAD_Handle_t` structure allows flexible pin assignment:

```c
KEYPAD_Handle_t kp;
kp.rowPort = LPC_GPIO0;       // Rows on PORT0
kp.colPort = LPC_GPIO0;       // Columns on PORT0
kp.rowPins[0] = 1;  // P0.1 = Row 0
kp.rowPins[1] = 2;  // P0.2 = Row 1
// ... etc. — assign to match your board wiring
```
