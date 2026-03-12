# Document 03 — Peripheral Theory

Before reading any driver code, understand how each peripheral works at the hardware level. This document explains the theory behind every peripheral implemented in this library.

---

## 1. GPIO — General Purpose Input/Output

### What is GPIO?

GPIO pins are the most fundamental peripheral of any microcontroller. They can be configured as:
- **Output** — drive a pin HIGH (3.3V) or LOW (0V)
- **Input** — read the logic level of an external signal

### How GPIO Works Internally

```
                 Direction Register (FIODIR)
                         │
                         │  0 = Input
                         │  1 = Output
                    ┌────▼────┐
External Pin ──────►│  Input  │──► FIOPIN (read)
                    │  Latch  │
                    └────┬────┘
                         │
                    ┌────▼────┐
                    │ Output  │◄── FIOSET / FIOCLR (write)
                    │  Driver │
                    └─────────┘
                         │
                    External Pin
```

### GPIO Registers Summary

| Register | Function |
|----------|---------|
| `FIODIR` | Set pin direction (1=Output, 0=Input) |
| `FIOPIN` | Read pin level, or write output level |
| `FIOSET` | Atomically set pins HIGH |
| `FIOCLR` | Atomically set pins LOW |

---

## 2. GPIO Interrupt

### What is a GPIO Interrupt?

Instead of constantly polling (checking) a pin in a loop, a **GPIO interrupt** triggers an ISR automatically when a pin changes state. This is event-driven programming.

### Edge Detection

The LPC1768 GPIO interrupt system supports:
- **Rising edge** — LOW → HIGH transition
- **Falling edge** — HIGH → LOW transition
- Both edges simultaneously

Only **PORT0** and **PORT2** support GPIO interrupts on the LPC1768. Port 2 interrupts share the **EINT3** vector.

### GPIO Interrupt Flow

```
Button pressed → P2.10 goes HIGH
      │
      ▼
Hardware detects rising edge
      │
      ▼
EINT3_IRQHandler() called by NVIC
      │
      ▼
Read IO2IntStatR to confirm source
      │
      ▼
Clear interrupt: IO2IntClr = (1 << 10)
      │
      ▼
Execute action (toggle LED)
      │
      ▼
Return from ISR
```

---

## 3. UART — Universal Asynchronous Receiver/Transmitter

### What is UART?

UART is the simplest serial communication protocol. It sends data one bit at a time over a single wire (TX), and receives on another (RX). No clock line is needed — both sides agree on the baud rate beforehand.

### UART Frame Format

```
Idle  START  D0  D1  D2  D3  D4  D5  D6  D7  STOP  Idle
─────┐      ┌───┐   ┌───┐   ┌───┐   ┌───┐         ┌────
     └──────┘   └───┘   └───┘   └───┘   └─────────┘
      [LOW]                                  [HIGH]
```

- **START bit**: Always LOW — signals beginning of frame
- **Data bits**: 5–9 bits (we use 8-bit)
- **Parity**: Optional error check bit (we use None)
- **STOP bit**: Always HIGH — signals end of frame

### Baud Rate Calculation

```
Baud Rate Divisor = PCLK / (16 × Baud Rate)

For 9600 baud, PCLK = 25 MHz:
Divisor = 25,000,000 / (16 × 9600) = 162.76 ≈ 163
```

The divisor is split into DLL (lower 8 bits) and DLM (upper 8 bits).

---

## 4. ADC — Analog-to-Digital Converter

### What is an ADC?

An ADC converts a continuous analog voltage (0–3.3V) into a discrete digital number. The LPC1768 ADC is:
- **12-bit resolution**: output range 0–4095
- **8 channels**: AD0.0 through AD0.7
- **Conversion time**: ~65 µs at full speed

### Conversion Formula

```
Digital Value = (Analog Input Voltage / VREF) × 4095

Example:
  Analog input = 1.65V, VREF = 3.3V
  Result = (1.65 / 3.3) × 4095 = 2047
```

### Conversion Process

```
1. Select channel in ADCR[7:0]
2. Start conversion: ADCR[26:24] = 001
3. Poll ADGDR[31] — DONE bit
4. When DONE=1, read result from ADGDR[15:4]
5. Stop conversion: ADCR[26:24] = 000
```

---

## 5. I2C — Inter-Integrated Circuit

### What is I2C?

I2C is a 2-wire serial bus for connecting microcontrollers to peripheral devices. It uses:
- **SDA** — Serial Data
- **SCL** — Serial Clock

Multiple devices share the same two wires, each with a unique 7-bit or 10-bit address.

### I2C Protocol Diagram

```
       S  ADDR  W  ACK  DATA  ACK  P
SDA: ──┐7b7b7b7b7b7b7b0│▔│ 8b8b8b8b8b8b8b8 │▔│──
SCL: ───██████████████████████████████████████────
       START                               STOP
```

### I2C Transaction Types

| Type | Sequence |
|------|---------|
| Write | START → ADDR+W → ACK → DATA → ACK → STOP |
| Read | START → ADDR+R → ACK → DATA → NACK → STOP |
| Write then Read | START → ADDR+W → ACK → REG → ACK → RESTART → ADDR+R → ACK → DATA → NACK → STOP |

### ACK / NACK

- **ACK (0)**: Receiver pulls SDA LOW — "I received OK, send more"
- **NACK (1)**: Receiver releases SDA HIGH — "I'm done / error"

### I2C Status Codes (LPC1768)

| Code | Meaning |
|------|---------|
| 0x08 | START transmitted |
| 0x10 | Repeated START transmitted |
| 0x18 | SLA+W sent, ACK received |
| 0x28 | Data byte sent, ACK received |
| 0x40 | SLA+R sent, ACK received |
| 0x50 | Data received, ACK sent |
| 0x58 | Data received, NACK sent |

---

## 6. SPI — Serial Peripheral Interface

### What is SPI?

SPI is a high-speed, full-duplex serial bus using 4 wires:

| Signal | Direction | Purpose |
|--------|-----------|---------|
| SCK | Master→Slave | Clock |
| MOSI | Master→Slave | Master Out, Slave In |
| MISO | Slave→Master | Master In, Slave Out |
| SSEL/CS | Master→Slave | Chip Select (Active LOW) |

### SPI Timing

```
     CS:  ─┐                                    ┌──
           └────────────────────────────────────┘
    SCK:   ─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
            └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └
   MOSI:   ─[b7][b6][b5][b4][b3][b2][b1][b0]──
   MISO:   ─[b7][b6][b5][b4][b3][b2][b1][b0]──
```

SPI is full-duplex: MOSI and MISO transfer simultaneously.

### SPI Modes (CPOL/CPHA)

| Mode | CPOL | CPHA | Clock Idle | Data Sampled |
|------|------|------|-----------|-------------|
| 0 | 0 | 0 | LOW | Rising edge |
| 1 | 0 | 1 | LOW | Falling edge |
| 2 | 1 | 0 | HIGH | Falling edge |
| 3 | 1 | 1 | HIGH | Rising edge |

Our SPI driver uses **Mode 0** (CPOL=0, CPHA=0).

---

## 7. RTC — Real-Time Clock

### What is an RTC?

An RTC maintains accurate time (hours, minutes, seconds) and date (day, month, year) even when the main CPU is off (with battery backup). It uses a dedicated 32.768 kHz oscillator.

The LPC1768 RTC has:
- Separate registers for SEC, MIN, HOUR, DOM (day of month), MONTH, YEAR, DOW (day of week)
- Hardware compare/alarm registers
- Battery backup support

### RTC Initialization Sequence

```
1. Power ON (PCONP bit 9)
2. Stop RTC (CCR = 0)
3. Reset counters (CCR bit 1 = 1, then 0)
4. Set initial time (HOUR, MIN, SEC)
5. Set initial date (DOM, MONTH, YEAR)
6. Start clock (CCR bit 0 = 1)
```

---

## 8. Timer — General Purpose Timer/Counter

### What is a Timer?

A timer is a hardware counter that increments at each clock cycle. When it reaches a configured match value, it can:
- Trigger an interrupt
- Reset itself
- Toggle a pin

The LPC1768 has 4 identical 32-bit timers (Timer0–Timer3).

### Timer Registers

| Register | Purpose |
|----------|---------|
| `TC` | Timer Counter — current count |
| `PR` | Prescaler — divides clock before TC increments |
| `MR0–MR3` | Match Registers — compare values |
| `MCR` | Match Control — what happens on match (IRQ/reset/stop) |
| `TCR` | Timer Control — enable/disable/reset |
| `IR` | Interrupt Register — indicates / clears match interrupt |

### Timer0 at 500 ms with PCLK = 25 MHz

```
Prescaler (PR) = 25000 - 1   → timer ticks every 1 ms
Match (MR0) = 500            → interrupt every 500 ticks = 500 ms
MCR = interrupt + reset on MR0 match
```

---

## 9. PWM — Pulse Width Modulation

### What is PWM?

PWM generates a square wave with a controllable **duty cycle** — the percentage of time the signal is HIGH within one period.

```
Period (T)
├──────────────────────────────────────────────────┤
█████████████████████                               █████
│←── ON (duty) ──→│←─────── OFF ────────────────→│

Duty Cycle = (ON time / Period) × 100%
```

### PWM Applications

- **Motor speed control** — higher duty = faster motor
- **LED brightness** — higher duty = brighter LED
- **Servo position** — specific duty = specific angle

### PWM Period Calculation

```
PCLK = CCLK/4 = 25 MHz

Period Register (MR0) = PCLK / Frequency
For 1 kHz: MR0 = 25,000,000 / 1000 = 25,000

Duty Register (MR1) = MR0 × (duty_percent / 100)
For 50%: MR1 = 25,000 × 0.5 = 12,500
```

---

## 10. LCD — 16×2 Character Display

### What is an LCD?

The standard 16×2 LCD uses the **HD44780** controller. It can display 2 rows × 16 characters each.

Communication modes:
- **8-bit**: 8 data lines (D0–D7)
- **4-bit**: 4 data lines (D4–D7) — saves GPIO pins ✓ (our driver uses this)

### 4-bit Communication

In 4-bit mode, each character/command byte is sent as **two nibbles** (half-bytes):
1. Upper nibble (bits [7:4]) — EN pulse
2. Lower nibble (bits [3:0]) — EN pulse

### LCD Command Set

| Command | Code | Effect |
|---------|------|--------|
| Clear display | 0x01 | Clear all characters |
| Home | 0x02 | Move cursor to (0,0) |
| Function set 4-bit 2-line | 0x28 | Configure LCD |
| Display ON | 0x0C | Turn on display |
| Entry mode | 0x06 | Cursor moves right |
| Set DDRAM address row 0 | 0x80+col | Position cursor |
| Set DDRAM address row 1 | 0xC0+col | Position cursor |

---

## 11. OLED — SSD1306 128×64 Display

### What is an OLED?

An OLED (Organic Light-Emitting Diode) display emits its own light — no backlight needed. The SSD1306 controller drives a 128×64 pixel monochrome display and communicates via I2C.

The display is organized as **8 pages** of 128 bytes each (8 pixels tall per page = 64 pixels total).

### Character Rendering

Text is rendered using a **5×7 pixel font**. Each character is stored as 5 bytes, one byte per column, where each bit represents a pixel.

### OLED I2C Address: `0x78`

---

## 12. Keypad — 4×4 Matrix

### What is a Matrix Keypad?

A 4×4 keypad has 16 keys wired as a 4×4 grid. Only 8 wires are needed (4 rows + 4 columns) instead of 16 individual connections.

### Matrix Scan Algorithm

```
For each row (0–3):
    1. Drive ALL rows HIGH
    2. Drive CURRENT row LOW
    3. Read all column pins
    4. If any column reads LOW → key pressed at [row][col]
    5. Debounce: wait, re-read, wait for release
    6. Return key character from keyMap[row][col]
```

---

## 13. 7-Segment Display

### What is a 7-Segment Display?

A 7-segment display has 7 LED segments (a–g) that form digits 0–9 when illuminated in combinations.

```
  _
 |_|     ← Segment layout: a=top, b=upper-right, c=lower-right,
 |_|        d=bottom, e=lower-left, f=upper-left, g=middle
```

### Segment Encoding Table

| Digit | Binary (gfedcba) | Hex |
|-------|-------------------|-----|
| 0 | 0011 1111 | 0x3F |
| 1 | 0000 0110 | 0x06 |
| 2 | 0101 1011 | 0x5B |
| 3 | 0100 1111 | 0x4F |
| 4 | 0110 0110 | 0x66 |
| 5 | 0110 1101 | 0x6D |
| 6 | 0111 1101 | 0x7D |
| 7 | 0000 0111 | 0x07 |
| 8 | 0111 1111 | 0x7F |
| 9 | 0110 1111 | 0x6F |

Our driver maps `PORT2[7:0]` directly to segments a–g, sending the byte from this table.
