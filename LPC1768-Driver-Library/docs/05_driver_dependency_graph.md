# Document 05 — Driver Dependency Graph

Understanding which drivers depend on other drivers is critical before building or modifying any component. This document visualizes all dependencies in this library.

---

## Dependency Overview

```
═══════════════════════════════════════════════════════════════
           LPC1768 DRIVER DEPENDENCY GRAPH
═══════════════════════════════════════════════════════════════

STANDALONE DRIVERS (no dependencies)
─────────────────────────────────────
  ┌────────┐  ┌────────┐  ┌────────┐
  │  GPIO  │  │  SPI   │  │  PWM   │
  └────────┘  └────────┘  └────────┘

UART-DEPENDENT DRIVERS
──────────────────────
        ┌──────────────────────┐
        │         UART         │  ← Base serial output
        └──────┬───────────────┘
               │
    ┌──────────┼──────────┐
    │          │          │
┌───▼──┐  ┌───▼──┐  ┌────▼───┐
│ ADC  │  │ I2C  │  │  RTC   │
└──────┘  └──┬───┘  └────────┘
             │
         ┌───▼────┐
         │  OLED  │  ← OLED uses I2C for display
         └────────┘

GPIO-DEPENDENT DRIVERS
────────────────────────
        ┌─────────────────────────┐
        │          GPIO           │  ← Base output control
        └──────┬──────────────────┘
               │
    ┌──────────┼──────────────────┐
    │          │                  │
┌───▼──────┐  ┌▼──────┐  ┌───────▼────┐
│  TIMER   │  │  LCD  │  │  7-SEGMENT │
│(uses GPIO│  └───┬───┘  └────────────┘
│ for LED) │      │
└──────────┘   ┌──┴────────────────┐
               │                   │
          ┌────▼────┐     ┌────────▼────┐
          │ KEYPAD  │     │  UART + LCD  │
          │(LCD+GPIO│     │  (UART+LCD) │
          └─────────┘     └────────────┘

GPIO INTERRUPT
──────────────
┌───────────────────┐
│  GPIO_INTERRUPT   │  ← Uses GPIO + NVIC
│  (standalone ISR) │
└───────────────────┘
```

---

## Driver Build Order

When compiling a multi-driver project, add source files in this order:

```
Step 1 (always):  startup_LPC17xx.s   + system_LPC17xx.c
Step 2 (base):    gpio.c              (needed by most)
Step 3 (comms):   uart.c              (needed by ADC, I2C, RTC)
Step 4 (comms):   i2c.c               (needed by OLED)
Step 5 (display): lcd.c               (needed by Keypad, uart_lcd)
Step 6 (sensors): adc.c, spi.c, rtc.c, timer.c, pwm.c
Step 7 (display): oled.c, keypad.c, 7seg.c
Step 8 (combined): uart_lcd (main.c uses both uart + lcd)
```

---

## Dependency Table

| Driver | Depends On | Required Files |
|--------|-----------|---------------|
| GPIO | — | `gpio.c`, `gpio.h` |
| GPIO Interrupt | GPIO (NVIC) | `gpio_interrupt.c` |
| UART | — | `uart.c`, `uart.h` |
| ADC | UART (for printf output) | `adc.c`, `uart.c` |
| I2C | UART (for debug) | `i2c.c`, `uart.c` |
| SPI | — | `spi.c` |
| RTC | UART (for time output) | `rtc.c`, `uart.c` |
| Timer | — (uses NVIC, GPIO optional) | `timer.c` |
| PWM | — | `pwm.c` |
| LCD | GPIO (uses GPIO1 directly) | `lcd.c` |
| OLED | I2C | `oled.c`, `i2c.c` |
| Keypad | LCD (key echoed to LCD) | `keypad.c`, `lcd.c` |
| 7-Segment | GPIO (uses GPIO2) | `7seg.c` |
| UART + LCD | UART + LCD | `uart.c`, `lcd.c` |

---

## Shared Hardware Resources (Conflicts to Avoid)

| Resource | Used By | Port/Pin |
|----------|---------|---------|
| P0.27/P0.28 (I2C) | I2C + OLED | Cannot use simultaneously for different I2C devices without addressing |
| PORT2[0] | PWM1 AND 7-segment (both use P2.0) | **Cannot flash PWM + 7-seg at same time** |
| UART0 (P0.2/P0.3) | UART, ADC, I2C, RTC, UART_LCD | Shared debug channel — only one project at a time |

> **Note:** P2.0 is shared between PWM1 output and 7-segment segment A. When using PWM, disconnect 7-segment from P2.0. When using 7-segment, do not initialize PWM1.
