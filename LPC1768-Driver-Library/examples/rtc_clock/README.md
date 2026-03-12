# Example: RTC Clock

This example runs the hardware Real-Time Clock and prints the current time every second via UART.

## What This Example Does
- Initializes RTC, sets date to 21/5/2026, time 10:30:00
- Reads HOUR, MIN, SEC every loop iteration
- Prints `Time: HH:MM:SS` to serial terminal

## Files Needed
- `drivers/rtc/rtc.c` + `rtc.h`
- `drivers/rtc/uart.c` + `uart.h`
- `drivers/rtc/main.c`
- `common/startup_LPC17xx.s` + `system_LPC17xx.c`

## Quick Steps
1. Flash `rtc.hex` → RESET
2. Open PuTTY → 9600 8N1
3. Watch `Time: 10:30:00`, `10:30:01`, ... ✓

See: [`drivers/rtc/README.md`](../../drivers/rtc/README.md)
