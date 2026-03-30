# LPC1768 Driver Library

![Platform](https://img.shields.io/badge/Platform-LPC1768%20ARM%20Cortex--M3-blue)
![IDE](https://img.shields.io/badge/IDE-Keil%20%C2%B5Vision%204-orange)
![Language](https://img.shields.io/badge/Language-C-brightgreen)
![License](https://img.shields.io/badge/License-MIT-green)
![Drivers](https://img.shields.io/badge/Drivers-14-purple)
![Board](https://img.shields.io/badge/Board-RDL%20Trainer%20Kit-red)

> **A complete, beginner-friendly driver library and learning manual for the LPC1768 ARM Cortex-M3 microcontroller.**  
> Every peripheral is documented from theory to hardware testing: register-level, line-by-line.

---

## Quick Start: 5 Minutes to First Blink

```
1. Install Keil µVision 4  →  see tools/keil_setup.md
2. Open drivers/gpio/gpio.uvproj
3. Press F7 to Build
4. Locate gpio.hex in the project folder
5. Open Flash Magic → select LPC1768, COM port, Baud 9600
6. Load gpio.hex → click Start
7. Press RESET on the trainer board
8. Watch LED at P1.18 blink! 
```

No hardware? → See [`docs/08_keil_simulation.md`](docs/08_keil_simulation.md) to run in software simulation.

---

## About This Repository

This repository contains custom peripheral drivers written in **C** for the **NXP LPC1768 ARM Cortex-M3** microcontroller, tested on the **RDL Technologies LPC1768 Trainer Kit**.

It serves as both:
- A **production-quality driver library** for embedded development
- A **complete learning manual** from microcontroller architecture to hardware testing

Documentation style is inspired by CMSIS, Linux kernel drivers, and industrial HAL frameworks.

---

## LPC1768 Overview

| Feature | Detail |
|---------|--------|
| CPU | ARM Cortex-M3 @ 100 MHz |
| Flash | 512 KB |
| SRAM | 64 KB |
| GPIO Ports | PORT0–PORT4 |
| Communication | UART, I2C, SPI, CAN, USB |
| Timers | 4× 32-bit timers |
| ADC | 12-bit, 8-channel |
| PWM | 6-channel PWM1 |
| RTC | Real-Time Clock with battery backup |
| Interrupt Controller | NVIC (240 interrupts) |
| Package | LQFP100 |

---

## Trainer Board: RDL Technologies LPC1768 Kit

The **RDL Technologies LPC1768 Trainer Kit** is the target hardware for all drivers. It provides:

- Onboard LEDs (PORT1)
- 4×4 Matrix Keypad
- 16×2 LCD Display (4-bit interface)
- 128×64 OLED Display (I2C)
- ADC Potentiometer (P0.24)
- UART via USB-to-Serial (FTDI)
- I2C EEPROM
- SPI interface
- 7-Segment Display
- RTC with backup
- GPIO headers
- JTAG debug connector

---

## Implemented Drivers

| Driver | Peripheral | Port/Pins | Test Output |
|--------|-----------|-----------|-------------|
| [`gpio`](drivers/gpio/README.md) | GPIO Ports 0–3 | P1.18 | LED blinks |
| [`gpio_interrupt`](drivers/gpio_interrupt/README.md) | EINT3/GPIOINT | P2.10, P1.18 | Button toggles LED |
| [`uart`](drivers/uart/README.md) | UART0 | P0.2, P0.3 | Serial terminal |
| [`adc`](drivers/adc/README.md) | ADC0 Channel 1 | P0.24 | Pot value via UART |
| [`i2c`](drivers/i2c/README.md) | I2C0 | P0.27, P0.28 | I2C frames |
| [`spi`](drivers/spi/README.md) | SPI | P0.15–P0.18 | Byte transfer |
| [`rtc`](drivers/rtc/README.md) | RTC | — | Time via UART |
| [`timer`](drivers/timer/README.md) | Timer0 + NVIC | P1.18 | LED at 500 ms |
| [`pwm`](drivers/pwm/README.md) | PWM1 Ch.1 | P2.0 | 50% duty cycle |
| [`lcd`](drivers/lcd/README.md) | 16×2 HD44780 | P1.0,1,9,10,14,15 | LCD chars + counter |
| [`oled`](drivers/oled/README.md) | SSD1306 OLED | P0.27, P0.28 | OLED text |
| [`keypad`](drivers/keypad/README.md) | 4×4 Matrix | GPIO | Key → LCD |
| [`7segment`](drivers/7segment/README.md) | 7-Seg Display | GPIO2 | Digits 0–9 |
| [`uart_lcd`](drivers/uart_lcd/README.md) | UART + LCD | P0.2/3, P1.x | PC → LCD |

---

## Repository Structure

```
LPC1768-Driver-Library/
├── README.md
├── docs/                         ← Architecture, theory, setup guides
│   ├── 01_lpc1768_architecture.md
│   ├── 02_board_overview.md
│   ├── 03_peripheral_theory.md
│   ├── 04_development_environment.md
│   ├── 05_driver_dependency_graph.md
│   ├── 06_register_reference.md
│   ├── 07_coding_standard.md
│   ├── 08_keil_simulation.md
│   └── images/
├── drivers/                      ← 14 driver folders (source + README)
│   ├── gpio/
│   ├── gpio_interrupt/
│   ├── uart/
│   ├── adc/
│   ├── i2c/
│   ├── spi/
│   ├── rtc/
│   ├── timer/
│   ├── pwm/
│   ├── lcd/
│   ├── oled/
│   ├── keypad/
│   ├── 7segment/
│   └── uart_lcd/
├── common/                       ← Shared startup + system files
│   ├── startup_LPC17xx.s
│   └── system_LPC17xx.c
├── examples/                     ← Standalone example projects
├── hardware/                     ← Pin mappings + board diagrams
│   ├── pin_connections.md
│   ├── board_diagram.md
│   └── pin_port_design.md
└── tools/                        ← Setup guides
    ├── keil_setup.md
    ├── flashing_guide.md
    └── keil_debugging.md
```

---

## Development Environment

| Tool | Version | Purpose |
|------|---------|---------|
| Keil µVision | 4 / 5 | IDE: compile, build, debug, simulate |
| Flash Magic | Latest | Flashing HEX via UART/USB |
| FTDI Drivers | Latest | USB-to-Serial (COM port) |
| PuTTY / Tera Term | Any | Serial terminal (9600 8N1) |

Setup instructions: [`tools/keil_setup.md`](tools/keil_setup.md)

---

## Documentation Sections

| File | Topic |
|------|-------|
| [`docs/01_lpc1768_architecture.md`](docs/01_lpc1768_architecture.md) | ARM Cortex-M3 core, memory map, clock tree, NVIC |
| [`docs/02_board_overview.md`](docs/02_board_overview.md) | Trainer board peripherals and layout |
| [`docs/03_peripheral_theory.md`](docs/03_peripheral_theory.md) | How each peripheral works (before reading code) |
| [`docs/04_development_environment.md`](docs/04_development_environment.md) | Keil setup, project creation, build, flash |
| [`docs/05_driver_dependency_graph.md`](docs/05_driver_dependency_graph.md) | Which drivers depend on which |
| [`docs/06_register_reference.md`](docs/06_register_reference.md) | All registers used — address + bit fields |
| [`docs/07_coding_standard.md`](docs/07_coding_standard.md) | Naming conventions, register access style |
| [`docs/08_keil_simulation.md`](docs/08_keil_simulation.md) | Simulate without hardware |
| [`hardware/pin_port_design.md`](hardware/pin_port_design.md) | Full port/pin reference from trainer board manual |

---

## Future Work

| Driver | Description |
|--------|-------------|
| CAN | Controller Area Network bus driver |
| USB | USB CDC Virtual COM Port driver |
| DMA | Direct Memory Access for UART/ADC |
| EEPROM | AT24C I2C EEPROM driver |
| WDT | Watchdog Timer driver |
| DAC | 10-bit Digital-to-Analog Converter |
| FreeRTOS | RTOS task scheduling integration |

---

## License

This project is licensed under the **MIT License**.  
See [LICENSE](LICENSE) for details.

---

*Built for the embedded systems community: learn by reading, understand by doing.*
