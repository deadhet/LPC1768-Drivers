# Example: UART Echo

This example demonstrates UART serial communication — receiving characters from a PC and echoing them back.

## What This Example Does
- Initializes UART0 at 9600 baud
- Sends "Hello from LPC1768 UART Driver" periodically
- Echoes any received character back with `Received: X`

## Files Needed
- `drivers/uart/uart.c` + `uart.h` + `main.c`
- `common/startup_LPC17xx.s` + `system_LPC17xx.c`

## Quick Steps
1. Flash `uart.hex` → RESET
2. Open PuTTY → Serial → COM port → 9600 8N1
3. Watch "Hello from LPC1768" appear
4. Type any character → it echoes back ✓

See: [`drivers/uart/README.md`](../../drivers/uart/README.md)
