# Pin Connections — Quick Reference

This is the quick-reference pin table for all drivers. For the complete port architecture and wiring diagrams, see [`pin_port_design.md`](pin_port_design.md).

| Driver | Signal | LPC1768 Pin | Direction |
|--------|--------|-------------|-----------|
| GPIO | LED1 | P1.18 | Output |
| GPIO Interrupt | Button | P2.10 | Input (ISR) |
| GPIO Interrupt | LED | P1.18 | Output |
| UART | TX | P0.2 | Output |
| UART | RX | P0.3 | Input |
| ADC | Potentiometer | P0.24 (AD0.1) | Analog In |
| I2C | SDA | P0.27 | Bidir |
| I2C | SCL | P0.28 | Output |
| SPI | SCK | P0.15 | Output |
| SPI | SSEL | P0.16 | Output |
| SPI | MISO | P0.17 | Input |
| SPI | MOSI | P0.18 | Output |
| PWM | Output | P2.0 (PWM1.1) | Output |
| LCD | RS | P1.0 | Output |
| LCD | EN | P1.1 | Output |
| LCD | D4 | P1.9 | Output |
| LCD | D5 | P1.10 | Output |
| LCD | D6 | P1.14 | Output |
| LCD | D7 | P1.15 | Output |
| OLED | SDA | P0.27 (I2C) | Bidir |
| OLED | SCL | P0.28 (I2C) | Output |
| 7-Segment | Seg A–G, DP | P2.0–P2.7 | Output |
