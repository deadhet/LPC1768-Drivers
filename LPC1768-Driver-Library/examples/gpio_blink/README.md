# Example: GPIO Blink

This example demonstrates the simplest possible embedded program — blinking an LED using the GPIO driver.

## What This Example Does
- Configures P1.18 as GPIO output
- Toggles LED ON/OFF with a software delay loop

## Files Needed
- `drivers/gpio/gpio.c`
- `drivers/gpio/gpio.h`
- `drivers/gpio/main.c`
- `common/startup_LPC17xx.s`
- `common/system_LPC17xx.c`

## Quick Steps
1. Open Keil → create project → add above files
2. Enable HEX output → Build (F7)
3. Flash `gpio.hex` → Press RESET
4. LED at P1.18 blinks at ~1 Hz ✓

See full documentation: [`drivers/gpio/README.md`](../../drivers/gpio/README.md)
