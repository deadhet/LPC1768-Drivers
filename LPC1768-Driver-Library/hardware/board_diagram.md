# Board Diagram

## RDL Technologies LPC1768 Trainer Board — Peripheral Layout

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                         RDL LPC1768 TRAINER KIT                                  │
│                                                                                  │
│   ┌────────────────────────────────────────────┐   ┌────────────────────────┐    │
│   │           16x2 CHARACTER LCD               │   │   128x64 OLED          │    │
│   │         (HD44780 via P1, 4-bit)            │   │   (SSD1306, I2C 0x78)  │    │
│   │  Row 0: ________________                   │   │   ┌──────────────────┐ │    │
│   │  Row 1: ________________                   │   │   │    64px          │ │    │
│   │  RS=P1.0, EN=P1.1                          │   │   │   128px          │ │    │
│   │  D4-D7 = P1.9,10,14,15                     │   │   └──────────────────┘ │    │
│   └────────────────────────────────────────────┘   └────────────────────────┘    │
│                                                                                  │
│   ┌──────────────────────┐   ┌──────────────────────────────────────────────┐    │
│   │     4x4 KEYPAD       │   │              ONBOARD LEDs                    │    │
│   │  [1][2][3][4]        │   │   ○ LED1 (P1.18)  ○ LED2 (P1.19)           │    │
│   │  [5][6][7][8]        │   │   ○ LED3 (P1.20)  ○ LED4 (P1.21)           │    │
│   │  [9][0][A][B]        │   └──────────────────────────────────────────────┘    │
│   │  [C][D][E][F]        │                                                       │
│   └──────────────────────┘                                                       │
│                                                                                  │
│   ┌───────────────────┐   ┌────────────────────────────────────────────────┐     │
│   │   7-SEGMENT       │   │                NXP LPC1768                     │     │
│   │   DISPLAY         │   │           ARM Cortex-M3 @ 100 MHz              │     │
│   │    ___            │   │              LQFP100 Package                   │     │
│   │   |   |  P2.0-7   │   │                                                │     │
│   │   |___|           │   │   Flash: 512KB   SRAM: 64KB                   │     │
│   │   |   |           │   │   GPIO: PORT0-4  UART/I2C/SPI/ADC/PWM/RTC    │     │
│   │   |___|           │   └────────────────────────────────────────────────┘     │
│   └───────────────────┘                                                          │
│                                                                                  │
│   ┌─────────────────┐  ┌─────────────────┐  ┌──────────────────┐                │
│   │  USB / UART     │  │  ADC POT        │  │  SPI HEADER      │                │
│   │  (FTDI Chip)    │  │  (P0.24 AD0.1)  │  │  P0.15-P0.18     │                │
│   │  P0.2=TX        │  │  0-3.3V input   │  │  SCK,SSEL,       │                │
│   │  P0.3=RX        │  │                 │  │  MISO, MOSI      │                │
│   └─────────────────┘  └─────────────────┘  └──────────────────┘                │
│                                                                                  │
│   ┌─────────────────┐  ┌─────────────────┐  ┌──────────────────┐                │
│   │  I2C HEADER     │  │  USER BUTTON    │  │  JTAG (20-pin)   │                │
│   │  P0.27 SDA      │  │  P2.10 (ISR)    │  │  for ULINK2/     │                │
│   │  P0.28 SCL      │  │  + RESET btn    │  │  J-Link debug    │                │
│   └─────────────────┘  └─────────────────┘  └──────────────────┘                │
│                                                                                  │
└──────────────────────────────────────────────────────────────────────────────────┘
```

## Power Input Options

```
[USB Type-B] ──→ 5V input ──→ Onboard LDO ──→ 3.3V (CPU + peripherals)
[DC Barrel]  ──→ 7-12V   ──→ Onboard LDO ──→ 3.3V
```

## Communication Connectors Summary

| Connector | Location | Protocol | Pins |
|-----------|---------|---------|------|
| USB | Bottom left | UART0 via FTDI | P0.2, P0.3 |
| I2C Header | Bottom mid | I2C0 | P0.27 (SDA), P0.28 (SCL) |
| SPI Header | Bottom right | SPI | P0.15–P0.18 |
| JTAG Header | Right side | JTAG/SWD | TCK, TMS, TDI, TDO |
| GPIO Headers | Multiple | GPIO | Various ports |
