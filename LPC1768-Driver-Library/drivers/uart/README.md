# UART Driver — LPC1768

## 1. Driver Overview

**UART (Universal Asynchronous Receiver/Transmitter)** is the most widely used serial communication interface in embedded systems. It enables the LPC1768 to communicate with a PC terminal, other microcontrollers, GPS modules, Bluetooth modules, or any UART-capable device.

UART is called "asynchronous" because it does not use a separate clock line. Instead, both sides agree on the same baud rate before communication begins. The data frame itself contains start and stop bits that allow the receiver to synchronize automatically.

**Real-world applications:**
- Debug output via serial terminal (PuTTY)
- Data logging to a PC
- Communication with GPS, WiFi, or Bluetooth modules
- Inter-processor communication

This driver implements UART0 with configurable baud rate and includes printf-style formatted output.

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

## 3. Registers Used

### LPC_SC→PCONP: Peripheral Power Control

The PCONP (Power Control for Peripherals) register controls which peripherals receive power and a clock signal. By default, most peripherals are powered off to save energy. You must set the appropriate bit before accessing any peripheral's registers. Writing to a powered-off peripheral produces unpredictable results.

| Bit | Field | Value | Description |
|-----|-------|-------|-------------|
| 3 | PCUART0 | 1 | Enable UART0 power and clock |

### LPC_PINCON→PINSEL0: Pin Select Register

Every physical pin on the LPC1768 can be connected to multiple internal functions. The PINSEL registers control which function each pin performs. Each pin uses 2 bits in the PINSEL register:

| Bit value | Meaning |
|-----------|---------|
| 00 | GPIO (default) |
| 01 | First alternate function (UART0 for P0.2/P0.3) |
| 10 | Second alternate function |
| 11 | Third alternate function |

| Bits | Pin | Value | Function Selected |
|------|-----|-------|------------------|
| [5:4] | P0.2 | 01 | TXD0 |
| [7:6] | P0.3 | 01 | RXD0 |

### LPC_UART0→LCR: Line Control Register

The LCR configures the data frame format. It also contains the DLAB (Divisor Latch Access Bit), which is a special control bit that temporarily redirects addresses of other registers to expose the baud rate divisor latches.

| Bit(s) | Field | Value Used | Description |
|--------|-------|-----------|-------------|
| [1:0] | WLS | 11 | 8-bit word length |
| [2] | SBS | 0 | 1 stop bit |
| [3] | PE | 0 | No parity |
| [7] | DLAB | 1 then 0 | 1=access DLL/DLM, 0=normal operation |

### LPC_UART0→DLL / DLM: Divisor Latch Registers

These two registers store the baud rate divisor. They are only accessible when DLAB=1 in LCR. The divisor is a 16-bit value split across two 8-bit registers.

```
PCLK = SystemCoreClock / 4 = 100,000,000 / 4 = 25,000,000 Hz
Divisor = PCLK / (16 × baudrate) = 25,000,000 / (16 × 9600) = 162 (decimal)
DLL = 162 & 0xFF = 0xA2  (lower 8 bits)
DLM = (162 >> 8) & 0xFF = 0x00  (upper 8 bits)
```

The factor of 16 comes from the UART's internal oversampling: it samples each bit 16 times and uses majority voting to determine the bit value, which makes it more immune to noise.

### LPC_UART0→FCR: FIFO Control Register

The FIFO (First-In First-Out) buffer is a small hardware queue that can hold up to 16 bytes before the CPU needs to read them. This prevents data loss if the CPU is momentarily busy.

| Bit | Field | Value | Description |
|-----|-------|-------|-------------|
| 0 | FIFOEN | 1 | Enable FIFO |
| 1 | RXFIFORES | 1 | Reset RX FIFO (discard leftover bytes) |
| 2 | TXFIFORES | 1 | Reset TX FIFO (discard pending TX bytes) |

### LPC_UART0→LSR: Line Status Register

The LSR is a read-only status register that reflects the current state of the UART hardware.

| Bit | Field | Meaning |
|-----|-------|---------|
| 0 | RDR | 1 = Receive data ready (data available in RBR) |
| 5 | THRE | 1 = Transmit Holding Register Empty (OK to send next byte) |

### LPC_UART0→THR: Transmit Holding Register

Writing a byte to THR causes the UART hardware to immediately begin serializing and transmitting that byte on the TX pin. The hardware handles shifting, start bit, stop bit, and timing autonomously once the byte is loaded.

### LPC_UART0→RBR: Receive Buffer Register

When UART hardware receives a complete byte on the RX pin, it stores it in RBR. Reading RBR retrieves the received byte and clears the RDR flag in LSR.

## 4. Driver Architecture

```
Initialization Flow:
UART0_Init(baudrate)
     │
     ├─ 1. Enable UART0 power (PCONP bit 3)
     ├─ 2. Configure P0.2/P0.3 as UART pins (PINSEL0)
     ├─ 3. Calculate PCLK = CCLK/4
     ├─ 4. Calculate baud divisor = PCLK / (16 × baud)
     ├─ 5. Set DLAB=1 → write DLL, DLM
     ├─ 6. Set 8N1 frame + DLAB=0 (LCR = 0x03)
     └─ 7. Enable + reset FIFO (FCR = 0x07)

Transmission Flow:
UART0_SendChar(ch)
     │
     ├─ Poll LSR bit 5 (THRE) until = 1  ←── wait if busy
     └─ Write ch to THR → hardware sends byte

Reception Flow:
UART0_ReceiveChar()
     │
     ├─ Poll LSR bit 0 (RDR) until = 1  ←── wait for data
     └─ Read and return RBR
```

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

### `UART0_ReceiveChar(void)`
Waits for and returns one received character from RBR.

### `UART0_SendString(const char *str)`
Sends a null-terminated string character by character.

### `UART0_Printf(const char *format, ...)`
Formatted print uses `vsprintf()` internally into a 100-byte buffer, then sends via `UART0_SendString()`.

## 6. Code Walkthrough

### Step 1: Enabling UART0 Power

```c
LPC_SC->PCONP |= (1 << 3);
```

`LPC_SC->PCONP` is the System Control Peripheral Power register located at address 0x400FC0C4. Each bit in this register controls whether a specific peripheral receives a clock signal from the PLL. Bit 3 corresponds to UART0. The `|=` operator sets bit 3 while leaving all other bits unchanged this ensures other peripherals that are already powered on (like Timer0, GPIO, etc.) are not accidentally disabled.

Without this step, any write to UART0 registers would have no effect because the hardware has no clock to operate with.

### Step 2: Configuring Pin Functions for UART

```c
LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));
LPC_PINCON->PINSEL0 |=  ((1 << 4) | (1 << 6));
```

PINSEL0 controls the function of pins P0.0 through P0.15. Each pin uses 2 consecutive bits. P0.2 occupies bits [5:4] and P0.3 occupies bits [7:6].

The first line clears both 2-bit fields to 00 (GPIO mode). The value `(3 << 4)` is binary `110000` it creates a mask covering bits 5 and 4. Using `&= ~` clears those bits without affecting others. The same happens for `(3 << 6)` for bits 7 and 6.

The second line sets both fields to 01, which selects alternate function 1: TXD0 for P0.2 and RXD0 for P0.3. Writing `(1 << 4)` sets bit 4, giving PINSEL0[5:4] = 01. Writing `(1 << 6)` sets bit 6, giving PINSEL0[7:6] = 01.

After this, the physical pins P0.2 and P0.3 are no longer general-purpose GPIO they are now internally connected to the UART0 transmitter and receiver hardware.

### Step 3: Baud Rate Calculation

```c
uint32_t pclk = SystemCoreClock / 4;
uint32_t divisor = pclk / (16 * baudrate);
```

`SystemCoreClock` is a CMSIS variable that holds the CPU clock frequency in Hz (100,000,000 Hz on the LPC1768 trainer kit). The peripheral clock (PCLK) for UART0 is CCLK divided by 4 by default, giving 25 MHz. The divisor formula comes from the UART baud rate equation: the internal UART clock runs at 16× the baud rate, so the divisor divides PCLK down to that rate.

For 9600 baud: `divisor = 25,000,000 / (16 × 9600) = 162`

### Step 4: Enabling DLAB and Writing Divisor

```c
LPC_UART0->LCR = (1 << 7);    // DLAB = 1
LPC_UART0->DLL = divisor & 0xFF;
LPC_UART0->DLM = (divisor >> 8) & 0xFF;
```

Setting bit 7 of LCR (the DLAB bit) to 1 causes the hardware to redirect the memory addresses of DLL and DLM to be accessible. At the same hardware addresses where THR and RBR normally sit, the divisor latches now appear instead.

`divisor & 0xFF` extracts the lower 8 bits and writes them to DLL. `(divisor >> 8) & 0xFF` shifts the value right by 8 bits and masks to extract the upper byte for DLM.

For divisor = 162 (0x00A2): DLL = 0xA2, DLM = 0x00.

### Step 5: Setting Frame Format and Clearing DLAB

```c
LPC_UART0->LCR = (3 << 0);    // 8-bit word, 1 stop, no parity, DLAB=0
```

Writing `(3 << 0)` sets bits [1:0] to 11, which selects 8-bit word length. All other bits including bit 7 (DLAB) are cleared by this write. With DLAB=0, the DLL and DLM registers are hidden again and THR/RBR become accessible. The frame is now configured as 8N1: 8 data bits, No parity, 1 stop bit.

### Step 6: Enabling the FIFO

```c
LPC_UART0->FCR = (1 << 0) | (1 << 1) | (1 << 2);
```

This writes 0x07 to FCR. Bit 0 (FIFOEN) enables the FIFO buffer. Bit 1 (RXFIFORES) clears and resets the RX FIFO any stale data from before initialization is discarded. Bit 2 (TXFIFORES) similarly resets the TX FIFO. This must be done during initialization to start with clean buffers.

### Step 7: Sending a Character

```c
void UART0_SendChar(char ch)
{
    while (!(LPC_UART0->LSR & (1 << 5)));
    LPC_UART0->THR = ch;
}
```

Before writing to THR, the code checks bit 5 of LSR (the THRE — Transmit Holding Register Empty flag). If this bit is 0, the previous byte is still being serialized by the UART shift register. The while loop spins until THRE becomes 1, indicating the hardware is ready to accept a new byte.

Once THRE is 1, writing to THR loads the byte into the transmit queue. The UART hardware automatically appends the start bit, 8 data bits (LSB first), and stop bit, then shifts them out on P0.2 at the configured baud rate.

### Step 8: Receiving a Character

```c
char UART0_ReceiveChar(void)
{
    while (!(LPC_UART0->LSR & (1 << 0)));
    return LPC_UART0->RBR;
}
```

Bit 0 of LSR is the RDR (Receive Data Ready) flag. It is set to 1 by hardware when a complete byte has been received and stored in RBR. The while loop blocks until data arrives. Reading RBR returns the received byte and automatically clears the RDR flag.

## 7. Test Program Explanation

The test program (main.c) demonstrates all four driver functions:

1. `SystemInit()` - sets up the PLL to run the CPU at 100 MHz
2. `UART0_Init(9600)` - initializes UART0 at 9600 baud
3. `UART0_SendString("UART Driver Test Started\r\n")` - startup banner
4. Inside `while(1)`:
   - `UART0_Printf("Hello from LPC1768 UART Driver\r\n")` - sends periodically
   - `delay()` - software loop (~500k iterations)
   - Checks `LPC_UART0->LSR & (1 << 0)` directly - if data arrived, reads and echoes it back

The `\r\n` at the end of strings sends both carriage return and line feed, which is required by most terminal emulators to move the cursor to the beginning of the next line.

## 8. Hardware Testing Procedure

### Build & Flash
1. Add: `uart.c`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c` to Keil project
2. **F7** → build → 0 errors
3. Flash `uart.hex` via Flash Magic (9600 baud, ISP mode)
4. Press RESET

### Serial Terminal Setup
5. Open PuTTY → Serial → COM3 (or your COM port) → 9600 → Open

### Expected Output
```
UART Driver Test Started
Hello from LPC1768 UART Driver
Hello from LPC1768 UART Driver
...
(type a character in PuTTY → it echoes back "Received: x")
```

### Init Flow Summary
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

### Debugging in Keil
- **Peripheral view → UART0:** verify LCR=0x03, DLL=0xA2 after `UART0_Init()`
- **Breakpoint on `LPC_UART0->THR = ch`:** observe the byte value being loaded
- **Watch `LPC_UART0->LSR`:** bit 5 = TX empty, bit 0 = RX data ready
