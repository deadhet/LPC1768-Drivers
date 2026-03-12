# Example: ADC Potentiometer Read

This example reads the onboard potentiometer (P0.24) and prints the 12-bit value to the serial terminal.

## What This Example Does
- Initializes ADC Channel 1 (P0.24)
- Reads potentiometer position continuously
- Prints `ADC Value: XXXX` via UART

## Files Needed
- `drivers/adc/adc.c` + `adc.h`
- `drivers/adc/uart.c` + `uart.h`
- `drivers/adc/main.c`
- `common/startup_LPC17xx.s` + `system_LPC17xx.c`

## Quick Steps
1. Flash `adc.hex` → RESET
2. Open PuTTY → 9600 8N1
3. Rotate onboard potentiometer
4. Watch value change 0–4095 ✓

See: [`drivers/adc/README.md`](../../drivers/adc/README.md)
