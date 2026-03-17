# UART + LCD Combined Driver — LPC1768

## 1. Driver Overview

The **UART-LCD** project demonstrates the simultaneous operation of two completely independent peripheral drivers in a single embedded application — **UART0** for serial communication and a **16×2 HD44780 LCD** for display output. Characters typed in a PC terminal (PuTTY) are received over UART, displayed on the LCD, and echoed back to the terminal.

This is a practical example of multi-peripheral integration: both drivers share the same CPU but operate on entirely different hardware peripherals with no conflict. UART0 uses PORT0 pins and involves the UART peripheral registers. The LCD uses PORT1 GPIO pins and communicates via a 4-bit parallel interface.

**Real-world applications:**
- HMI (Human-Machine Interface) — serial command to LCD display
- Terminal-controlled embedded systems
- Serial protocol proxy terminals
- Embedded shell/debug consoles

## 2. Hardware Interface

### UART0

| Signal | Port | Pin | Description |
|--------|------|-----|-------------|
| UART0 TX | PORT0 | P0.2 | LPC1768 → PC (serial transmit) |
| UART0 RX | PORT0 | P0.3 | PC → LPC1768 (serial receive) |

### LCD (16×2 HD44780 in 4-bit mode)

| Signal | Port | Pin | Description |
|--------|------|-----|-------------|
| RS | PORT1 | P1.0 | Register Select (0=command, 1=data) |
| EN | PORT1 | P1.1 | Enable strobe |
| D4 | PORT1 | P1.9 | Data bit 4 |
| D5 | PORT1 | P1.10 | Data bit 5 |
| D6 | PORT1 | P1.14 | Data bit 6 |
| D7 | PORT1 | P1.15 | Data bit 7 |

## 3. Driver Architecture

Both drivers initialize independently during startup. Once initialized, neither interferes with the other because they operate on different hardware registers and different GPIO ports.

```
main()
  ├─ SystemInit()         ← 100 MHz PLL
  ├─ UART0_Init(9600)     ← UART using P0.2/P0.3 and UART0 registers
  ├─ LCD_Init()           ← LCD using P1.0,1,9,10,14,15 GPIO
  ├─ LCD: show "Ready..." ← startup message on display
  └─ while(1):
       if (LPC_UART0->LSR & (1 << 0)):  ← check RDR flag
         ch = UART0_ReceiveChar()        ← read byte from RBR
         LCD_Char(ch)                    ← display on LCD (RS=HIGH)
         UART0_SendChar(ch)              ← echo to terminal
```

## 4. Function Reference

This project reuses existing driver functions:

| Function | Source | Description |
|----------|--------|-------------|
| `UART0_Init(9600)` | uart.c | Initialize serial at 9600 baud, 8N1 |
| `UART0_ReceiveChar()` | uart.c | Blocking read from RBR when data ready |
| `UART0_SendChar(ch)` | uart.c | Transmit one character via THR |
| `LCD_Init()` | lcd.c | Initialize HD44780 in 4-bit mode |
| `LCD_Char(ch)` | lcd.c | Send character byte (RS=HIGH) |
| `LCD_String(str)` | lcd.c | Send string to LCD |

## 5. Code Walkthrough

### Initializing Both Peripherals

```c
UART0_Init(9600);
LCD_Init();
```

UART0_Init performs: PCONP bit 3 enable, PINSEL0 configure P0.2/P0.3, baud rate divisor calculation, DLAB write, LCR = 8N1, FIFO enable. After this call, the UART hardware is active and ready to receive or transmit data.

LCD_Init performs: PINSEL3 clear for GPIO mode, FIODIR set for all LCD pins, power-on delay, then the 4-bit initialization sequence (0x02, 0x28, 0x0C, 0x06, clear display). After this call, the LCD hardware is ready to receive character writes.

Neither initialization function touches the other's hardware or registers. They are completely independent.

### Checking for Received UART Data

```c
if (LPC_UART0->LSR & (1 << 0))
```

LSR is the Line Status Register of UART0. Bit 0 is the RDR (Receive Data Ready) flag. It is set to 1 by the UART hardware when a complete byte has been received and placed in the RBR. Checking this bit directly (rather than calling `UART0_ReceiveChar()` which blocks) allows the main loop to continue running without hanging while waiting for input.

If bit 0 is 0, no data has arrived, and the if block is skipped entirely.

### Reading the Received Character

```c
char ch = UART0_ReceiveChar();
```

This function polls `LPC_UART0->LSR & (1 << 0)` — but since we already checked it in the if condition, it will return immediately. Then it reads and returns `LPC_UART0->RBR`. Reading RBR automatically clears the RDR flag.

### Displaying Character on LCD

```c
LCD_Char(ch);
```

The `LCD_Char()` function (in this lcd.c) sets RS HIGH (P1.0 driven HIGH via FIOSET), then calls `LCD_Send4Bit(ch >> 4)` followed by `LCD_Send4Bit(ch & 0x0F)`. With RS HIGH, the HD44780 interprets the incoming byte as a character code and writes it to DDRAM at the current cursor position, then auto-increments the cursor.

If `ch` is 'A' (ASCII 65 = 0x41): upper nibble = 0x4, lower nibble = 0x1. These are sent as two separate EN pulses. The LCD renders the character 'A' at the current cursor position.

### Echoing Back to Terminal

```c
UART0_SendChar(ch);
```

`UART0_SendChar()` waits for `LPC_UART0->LSR` bit 5 (THRE = Transmit Holding Register Empty) to be 1, then writes the character to `LPC_UART0->THR`. The UART hardware serializes and transmits the byte back to the PC terminal. This creates an echo effect — the terminal displays what you typed, confirming the round trip: PC → LPC (UART RX) → LCD display → LPC → PC (UART TX echo).

### Row Overflow Handling

```c
idle_counter++;
if (idle_counter >= 16)    // Row 0 is full (16 chars)
{
    LCD_Command(0xC0);     // Move cursor to row 1, column 0
    idle_counter = 0;
}
```

The LCD row 0 has 16 columns. After 16 characters, the cursor moves to row 1. If a total of 32 characters have been displayed, the screen is cleared and the cursor returns to the top. This simple buffer management is implemented in main.c.

## 6. Test Program Explanation

Main.c initializes both peripherals, shows "Ready..." on the LCD, then enters the main loop. Each character typed in PuTTY triggers:
1. UART RDR flag goes HIGH (byte received)
2. `UART0_ReceiveChar()` reads it
3. `LCD_Char()` displays it on the LCD
4. `UART0_SendChar()` echoes it back to PuTTY

The LCD displays typed characters sequentially. PuTTY shows the echo of what was sent.

## 7. Hardware Testing Procedure

### Build & Flash
1. Add: `uart.c`, `uart.h`, `lcd.c`, `lcd.h`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c`
2. Build → flash `uart_lcd.hex` → RESET

### Expected Output
1. LCD row 0 shows "Ready..."
2. Open PuTTY: COM port, 9600 baud, 8N1
3. Type characters in PuTTY → characters appear on LCD → same characters echoed in PuTTY

### Init Flow Summary
```
SystemInit() → 100 MHz
      │
UART0_Init(9600) ─── UART ready
LCD_Init()       ─── LCD ready (independent)
      │
LCD: "Ready..."
      │
Loop: wait for UART byte → LCD display + UART echo
```

### Debugging in Keil
- **If LCD shows nothing:** verify LCD FIODIR bits for P1.0,1,9,10,14,15 all set
- **If UART not receiving:** verify COM port in PuTTY matches Device Manager, check P0.2/P0.3 PINSEL0 bits
- **If echo not working:** verify THRE polling in UART0_SendChar (LSR bit 5)
- **Both inits must succeed independently:** test each driver alone first before combining

## 8. Expected Output

```
LCD:   R e a d y . . .
(type 'H' in PuTTY)
LCD:   R e a d y . . .   H
PuTTY: H (echoed back)
(type 'i')
LCD:   R e a d y . . .   H i
PuTTY: Hi
```
