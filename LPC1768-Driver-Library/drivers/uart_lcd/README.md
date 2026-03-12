# UART + LCD Combined Driver — LPC1768

## 1. Driver Overview

The **UART-LCD** project demonstrates simultaneous use of two independent peripheral drivers — **UART0** (serial communication) and the **16×2 LCD** (display). Characters transmitted from a PC terminal appear on the LCD in real time.

This is a practical demonstration of multi-driver integration in an embedded application.

**Real-world applications:** HMI (Human-Machine Interface), terminal-controlled displays, serial command processors, embedded shells.

---

## 2. Hardware Interface

| Signal | Port | Pin | Description |
|--------|------|-----|-------------|
| UART0 TX | PORT0 | P0.2 | PC → LPC (receive) |
| UART0 RX | PORT0 | P0.3 | LPC → PC (transmit) |
| LCD RS | PORT1 | P1.0 | Register Select |
| LCD EN | PORT1 | P1.1 | Enable |
| LCD D4–D7 | PORT1 | P1.9, P1.10, P1.14, P1.15 | Data bus |

---

## 3. Driver Architecture

```
main()
  ├─ SystemInit()
  ├─ UART0_Init(9600)     ← Initialize UART
  ├─ LCD_Init()           ← Initialize LCD
  ├─ LCD shows "Ready..."  ← startup message
  └─ while(1):
       ├─ if UART data available (LSR bit 0):
       │     char = UART0_ReceiveChar()
       │     LCD_Data(char)          ← Show on LCD
       │     UART0_SendChar(char)    ← Echo back to PC
       └─ (else: wait)
```

---

## 4. Code Walkthrough

```c
// Check if UART received a character
if (LPC_UART0->LSR & (1 << 0))  // LSR bit 0 = RDR (Receiver Data Ready)
{
    char ch = UART0_ReceiveChar();  // Read from RBR
    LCD_Data(ch);                   // Display on LCD (RS=HIGH = data mode)
    UART0_SendChar(ch);             // Echo back to terminal
}
```

**LCD Data vs Command:** `LCD_Data()` sets RS=HIGH then sends the byte as a character. `LCD_Command()` sets RS=LOW for control (cursor, clear, etc.).

---

## 5. Hardware Testing Procedure

### Build & Flash
1. Add: `uart.c`, `uart.h`, `lcd.c`, `lcd.h`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c`
2. Build → flash `uart_lcd.hex` → RESET

### Expected Output
1. LCD shows "Ready..." on row 0
2. Open PuTTY (9600 8N1)
3. Type characters → they appear on LCD and echo back to PuTTY

### Init Flow Diagram
```
SystemInit()
      │
UART0_Init(9600) ──┐
                   │  Both peripherals initialized independently
LCD_Init()   ──────┘
      │
LCD: "Ready..."
      │
Loop: wait for UART byte → display on LCD + echo
```

### Debugging
- LCD shows nothing: verify LCD pins are set OUTPUT in FIODIR
- UART not receiving: check COM port in PuTTY matches Device Manager
- Both working: verify both inits called before the main loop
