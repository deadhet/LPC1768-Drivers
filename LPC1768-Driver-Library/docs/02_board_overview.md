# Document 02 — Board Overview

## RDL Technologies LPC1768 Trainer Kit

The **RDL Technologies LPC1768 Trainer Kit** is a compact development and training board built around the NXP LPC1768 ARM Cortex-M3 microcontroller. It is designed for embedded systems education and peripheral testing.

All drivers in this repository have been developed and tested specifically for this board.

> **Note:** The pin mapping and peripheral connections in this documentation have been derived from the LPC1768 Trainer Kit User Manual, ensuring that all driver implementations match the actual hardware configuration of the board.

---

## Board Overview Diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│                    RDL LPC1768 TRAINER BOARD                         │
│                                                                      │
│  ┌──────────────┐   ┌─────────────────┐   ┌──────────────────────┐  │
│  │  16x2 LCD    │   │  4x4 KEYPAD     │   │  128x64 OLED         │  │
│  │  (PORT1 4-bit│   │  (GPIO Matrix)  │   │  (I2C @ 0x78)        │  │
│  └──────────────┘   └─────────────────┘   └──────────────────────┘  │
│                                                                      │
│  ┌──────────────┐   ┌─────────────────┐   ┌──────────────────────┐  │
│  │  7-SEGMENT   │   │  LEDs (PORT1)   │   │  ADC POTENTIOMETER   │  │
│  │  DISPLAY     │   │  P1.18-P1.21    │   │  (P0.24 / AD0.1)     │  │
│  │  (PORT2)     │   │                 │   └──────────────────────┘  │
│  └──────────────┘   └─────────────────┘                             │
│                                                                      │
│  ┌──────────────────────────────────────────┐                       │
│  │           NXP LPC1768 (LQFP100)          │                       │
│  │           ARM Cortex-M3 @ 100 MHz        │                       │
│  └──────────────────────────────────────────┘                       │
│                                                                      │
│  ┌──────────────┐   ┌─────────────────┐   ┌──────────────────────┐  │
│  │  UART/USB    │   │  I2C EEPROM     │   │  SPI CONNECTOR       │  │
│  │  (FTDI)      │   │  (P0.27/P0.28)  │   │  (P0.15-P0.18)       │  │
│  └──────────────┘   └─────────────────┘   └──────────────────────┘  │
│                                                                      │
│  ┌──────────────┐   ┌─────────────────┐   ┌──────────────────────┐  │
│  │  RESET BTN   │   │  USER BUTTONS   │   │  POWER & JTAG        │  │
│  │              │   │  (P2.10, etc.)  │   │  CONNECTORS          │  │
│  └──────────────┘   └─────────────────┘   └──────────────────────┘  │
└──────────────────────────────────────────────────────────────────────┘
```

---

## Onboard Peripherals Reference

| Peripheral | Connection | Port/Pin | Interface | Notes |
|-----------|-----------|---------|-----------|-------|
| LEDs (4×) | Onboard | P1.18–P1.21 | GPIO Output | Active HIGH |
| 16×2 LCD | Onboard | P1.0,1,9,10,14,15 | GPIO 4-bit | HD44780 controller |
| 4×4 Keypad | Onboard | GPIO rows/cols | GPIO Scan | Matrix scan |
| OLED 128×64 | Onboard | P0.27, P0.28 | I2C @ 0x78 | SSD1306 controller |
| ADC Potentiometer | Onboard | P0.24 (AD0.1) | ADC Ch.1 | 12-bit 0–3.3V |
| UART/USB | USB connector | P0.2, P0.3 | UART0 | FTDI converter |
| I2C EEPROM | Onboard | P0.27, P0.28 | I2C0 | 24LC/AT24C series |
| SPI connector | Header | P0.15–P0.18 | SPI | External SPI devices |
| 7-Segment | Onboard | P2.0–P2.7 | GPIO Output | Common cathode |
| RTC | Internal | — | RTC peripheral | 32.768 kHz crystal |
| PWM output | Header | P2.0 | PWM1 Ch.1 | Variable duty cycle |
| Reset button | Onboard | nRESET pin | Hardware | Active LOW |
| User button | Onboard | P2.10 | GPIO Input | Active LOW, ISR capable |
| JTAG debug | 20-pin header | TCK, TMS, TDI, TDO | JTAG/SWD | For J-Link/ULINK |

---

## Power Supply

| Input | Voltage | Source |
|-------|---------|--------|
| USB bus power | 5V | USB Type-B connector |
| DC barrel jack | 7–12V | External adapter |
| Operating VCC | 3.3V | Onboard LDO regulator |

> All GPIO pins are **3.3V logic**. Do not connect 5V signals directly to GPIO pins.

---

## Communication Interfaces

### UART / USB-to-Serial
- Onboard **FTDI** chip converts UART0 to USB
- Appears as a virtual COM port on your PC
- Configure: **9600 baud, 8N1, no flow control**
- Used as the primary debug channel for most drivers

### I2C Bus
- Shared bus on P0.27 (SDA) and P0.28 (SCL)
- **OLED display** at address `0x78`
- **EEPROM** at address `0xA0`
- Pulled up to 3.3V with onboard resistors

### SPI Bus
- Available on header: P0.15 (SCK), P0.16 (SSEL), P0.17 (MISO), P0.18 (MOSI)
- Connects to external SPI devices

### CAN & USB
- CAN and USB pins available on headers
- Drivers not included in this release (see Future Work)

---

## GPIO Expansion Headers

The board provides GPIO expansion headers exposing LPC1768 ports for custom wiring. Use these to connect external sensors, actuators, and test equipment.

---

## Debug Options

| Method | Description |
|--------|-------------|
| JTAG | Full debug via 20-pin JTAG header (ULINK/J-Link) |
| SWD | 2-wire debug via SWD pins |
| UART | Printf-style debug via serial terminal |
| Keil Simulator | Software simulation with register observation (no board needed) |

---

## Board Quick Reference

```
                 TOP VIEW — KEY PIN AREAS
    ┌─────────────────────────────────────────┐
    │  [LCD P1.0–P1.15]    [OLED I2C P0.27/28]│
    │  [LEDS P1.18-21]     [UART P0.2/P0.3]  │
    │  [KEYPAD GPIO]       [ADC P0.24]        │
    │  [7SEG P2.0-7]       [PWM P2.0]         │
    │  [SPI P0.15-18]      [BTN P2.10]        │
    └─────────────────────────────────────────┘
```
