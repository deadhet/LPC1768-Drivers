# UART Driver — LPC1768

## 1. Driver Overview

**UART (Universal Asynchronous Receiver/Transmitter)** is the most widely used serial communication interface in embedded systems. It enables the LPC1768 to communicate with a PC terminal, other microcontrollers, GPS modules, Bluetooth modules, or any UART-capable device.

**Real-world applications:**
- Debug output via serial terminal (PuTTY)
- Data logging to a PC
- Communication with GPS, WiFi, or Bluetooth modules
- Inter-processor communication

This driver implements UART0 with configurable baud rate and includes printf-style formatted output.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| UART0 TX | PORT0 | P0.2 | Output | Transmit (LPC → PC) |
| UART0 RX | PORT0 | P0.3 | Input | Receive (PC → LPC) |

**Connection:**
```
LPC1768          FTDI Chip (on trainer board)     PC
P0.2 (TX) ─────→ RX ─────── USB ────────────→ COM port
P0.3 (RX) ←───── TX ←─────────────────────── COM port
```

**Serial settings:** 9600 baud, 8 data bits, No parity, 1 stop bit (8N1)

---

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power Control
| Bit | Field | Value | Description |
|-----|-------|-------|-------------|
| 3 | PCUART0 | 1 | Enable UART0 power and clock |

### LPC_PINCON→PINSEL0 — Pin Select
| Bits | Pin | Value | Function Selected |
|------|-----|-------|------------------|
| [5:4] | P0.2 | 01 | TXD0 |
| [7:6] | P0.3 | 01 | RXD0 |

### LPC_UART0→LCR — Line Control Register
| Bit(s) | Field | Value Used | Description |
|--------|-------|-----------|-------------|
| [1:0] | WLS | 11 | 8-bit word length |
| [2] | SBS | 0 | 1 stop bit |
| [3] | PE | 0 | No parity |
| [7] | DLAB | 1 then 0 | 1=access DLL/DLM, 0=normal |

### LPC_UART0→DLL / DLM — Divisor Latch
Stores baud rate divisor. Split into lower 8 bits (DLL) and upper 8 bits (DLM).

```
PCLK = SystemCoreClock / 4 = 25,000,000 Hz
Divisor = 25,000,000 / (16 × 9600) = 162 (decimal)
DLL = 162 & 0xFF = 0xA2
DLM = (162 >> 8) & 0xFF = 0x00
```

### LPC_UART0→FCR — FIFO Control Register
| Bit | Field | Value | Description |
|-----|-------|-------|-------------|
| 0 | FIFOEN | 1 | Enable FIFO |
| 1 | RXFIFORES | 1 | Reset RX FIFO |
| 2 | TXFIFORES | 1 | Reset TX FIFO |

### LPC_UART0→LSR — Line Status Register
| Bit | Field | Meaning |
|-----|-------|---------|
| 0 | RDR | 1 = Receive data ready (data available in RBR) |
| 5 | THRE | 1 = Transmit Holding Register Empty (OK to send) |

---

## 4. Driver Architecture

```
Initialization Flow:
─────────────────────────────────────────────────────
UART0_Init(baudrate)
     │
     ├─ 1. Enable UART0 power (PCONP bit 3)
     ├─ 2. Configure P0.2/P0.3 as UART pins (PINSEL0)
     ├─ 3. Calculate PCLK = CCLK/4
     ├─ 4. Calculate baud divisor = PCLK / (16 × baud)
     ├─ 5. Set DLAB=1 → write DLL, DLM
     ├─ 6. Set 8N1 frame + DLAB=0 (LCR = 0x03)
     └─ 7. Enable + reset FIFO (FCR)

Transmission Flow:
─────────────────────────────────────────────────────
UART0_SendChar(ch)
     │
     ├─ Poll LSR bit 5 (THRE) until = 1  ←── wait if busy
     └─ Write ch to THR → hardware sends byte

Reception Flow:
─────────────────────────────────────────────────────
UART0_ReceiveChar()
     │
     ├─ Poll LSR bit 0 (RDR) until = 1  ←── wait for data
     └─ Read and return RBR
```

---

## 5. Function Reference

### `UART0_Init(uint32_t baudrate)`
Initializes UART0 with baud rate, 8N1 frame, FIFO enabled.

| Parameter | Type | Description |
|-----------|------|-------------|
| `baudrate` | uint32_t | Baud rate (e.g., 9600, 115200) |
| Returns | void | |

**Registers modified:** PCONP, PINSEL0, LCR, DLL, DLM, FCR

### `UART0_SendChar(char ch)`
Transmits a single character, blocking until THR is empty.

| Parameter | Type | Description |
|-----------|------|-------------|
| `ch` | char | Character to send |
| Returns | void | |

### `UART0_ReceiveChar(void)`
Waits for and returns one received character.

| Returns | char | Received byte from RBR |
|---------|------|----------------------|

### `UART0_SendString(const char *str)`
Sends a null-terminated string character by character.

### `UART0_Printf(const char *format, ...)`
Formatted print — uses `vsprintf()` internally into a 100-byte buffer, then sends via `UART0_SendString()`.

---

## 6. Code Walkthrough

```c
void UART0_Init(uint32_t baudrate)
{
    uint32_t pclk, divisor;

    LPC_SC->PCONP |= (1 << 3);          // BIT 3: Enable UART0 clock

    // Clear PINSEL0 bits [5:4] and [7:6], then set to 01 (UART function)
    LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));
    LPC_PINCON->PINSEL0 |=  ((1 << 4) | (1 << 6));

    pclk = SystemCoreClock / 4;         // PCLK default = CCLK/4 = 25 MHz
    divisor = pclk / (16 * baudrate);   // Baud rate formula

    LPC_UART0->LCR = (1 << 7);          // DLAB=1: enable access to DLL/DLM
    LPC_UART0->DLL = divisor & 0xFF;    // Lower byte of divisor
    LPC_UART0->DLM = (divisor >> 8) & 0xFF; // Upper byte
    LPC_UART0->LCR = (3 << 0);          // 8-bit, 1 stop, no parity. DLAB=0.
    LPC_UART0->FCR = (1<<0)|(1<<1)|(1<<2); // Enable FIFO + reset TX+RX
}

void UART0_SendChar(char ch)
{
    while (!(LPC_UART0->LSR & (1 << 5))); // Wait: THRE=1 (transmitter empty)
    LPC_UART0->THR = ch;                   // Load byte — UART hardware sends it
}

char UART0_ReceiveChar(void)
{
    while (!(LPC_UART0->LSR & (1 << 0))); // Wait: RDR=1 (data available)
    return LPC_UART0->RBR;                 // Read byte from Receive Buffer Register
}
```

---

## 7. Test Program (main.c)

**What it does:**
1. Initializes UART0 at 9600 baud
2. Sends "UART Driver Test Started" on startup
3. Enters infinite loop:
   - Sends "Hello from LPC1768 UART Driver" periodically
   - Checks if a byte has been received
   - If yes, echoes it back with "Received: X"

---

## 8. Hardware Testing Procedure

### Build & Flash
1. Add: `uart.c`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c` to Keil project
2. **F7** → build → 0 errors
3. Flash `uart.hex` via Flash Magic (9600 baud, ISP mode)
4. Press RESET

### Serial Terminal Setup
5. Open PuTTY → Serial → COM3 → 9600 → Open

### Expected Output
```
UART Driver Test Started
Hello from LPC1768 UART Driver
Hello from LPC1768 UART Driver
...
(type a character in PuTTY → it echoes back "Received: x")
```

### Init Flow Diagram
```
Enable PCONP bit 3
      │
Configure PINSEL0 (P0.2=TXD, P0.3=RXD)
      │
PCLK = 25 MHz → divisor = 162
      │
DLAB=1 → write DLL=0xA2, DLM=0x00
      │
LCR = 0x03 (8N1, DLAB=0)
      │
FCR = 0x07 (FIFO enable + reset)
      │
UART Ready
```

### Debugging
- **Peripheral view → UART0:** verify LCR=0x03, DLL=0xA2 after init
- **Breakpoint on `LPC_UART0->THR = ch`:** observe byte value
- **Watch `LPC_UART0->LSR`:** bit 5 = TX empty, bit 0 = RX ready
