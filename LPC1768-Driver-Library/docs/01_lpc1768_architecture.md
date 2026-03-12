# Document 01 — LPC1768 Architecture

## ARM Cortex-M3 Core

The LPC1768 is built on the **ARM Cortex-M3** processor — a 32-bit RISC core designed specifically for embedded microcontrollers.

### Key CPU Features

| Feature | Detail |
|---------|--------|
| Architecture | ARMv7-M |
| ISA | Thumb-2 (16/32-bit mixed) |
| Clock Speed | Up to 100 MHz |
| Pipeline | 3-stage (Fetch → Decode → Execute) |
| Data Bus | 32-bit |
| Address Bus | 32-bit (4 GB addressable) |
| Endianness | Little-endian |

### Thumb-2 Instruction Set

Thumb-2 combines 16-bit (compact) and 32-bit (powerful) instructions. This means:
- Code density close to 8/16-bit architectures
- Performance close to 32-bit architectures
- No mode switching needed (unlike ARM7/ARM9)

---

## Memory Map

The LPC1768 uses a flat 32-bit address space. The processor can address 4 GB total, but only specific regions are used:

```
Address Range          Size     Region
═══════════════════════════════════════════════
0x00000000–0x0007FFFF  512 KB   Flash (On-Chip)
0x10000000–0x10007FFF  32 KB    SRAM (Local — fast)
0x2007C000–0x2007FFFF  16 KB    AHB SRAM Bank 0
0x20080000–0x20083FFF  16 KB    AHB SRAM Bank 1
0x40000000–0x400FFFFF  1 MB     APB Peripherals
0x20098000–0x2009FFFF           GPIO Registers
0xE0000000–0xE00FFFFF           Cortex-M3 Private (NVIC, SysTick)
```

### Flash Memory (0x00000000)
- 512 KB on-chip Flash
- Code and constant data stored here
- Executed in-place (XIP) — no copy to RAM needed
- Reset vector table starts at 0x00000000

### SRAM (0x10000000)
- 32 KB local SRAM — fastest access (single cycle)
- Stack, heap, and runtime variables live here
- Two additional 16 KB AHB SRAM banks for DMA

---

## GPIO Register Architecture

Each GPIO port has five key registers. All follow the same base address pattern:

```
PORT0 Base: 0x2009C000
PORT1 Base: 0x2009C020
PORT2 Base: 0x2009C040
PORT3 Base: 0x2009C060
PORT4 Base: 0x2009C080
```

| Register | Offset | Function |
|----------|--------|---------|
| FIODIR | +0x00 | Direction (1=Output, 0=Input) |
| FIOMASK | +0x10 | Masked pin access |
| FIOPIN | +0x14 | Read or write pin level |
| FIOSET | +0x18 | Set pins HIGH (write 1) |
| FIOCLR | +0x1C | Set pins LOW (write 1) |

**Important:** Writing to `FIOSET` only sets the specified bits HIGH — it never clears others. Writing to `FIOCLR` only clears specified bits LOW. This is called **atomic set/clear** and eliminates read-modify-write hazards in interrupt-driven code.

---

## Peripheral Buses

The LPC1768 uses two peripheral buses:

```
                    ┌─────────────────────┐
                    │  ARM Cortex-M3 Core  │
                    └──────────┬──────────┘
                               │
                    ┌──────────▼──────────┐
                    │   AHB (Advanced     │   100 MHz
                    │   High-performance  │   ← fast bus
                    │   Bus)              │
                    └──┬──────────────┬───┘
                       │              │
          ┌────────────▼──┐    ┌──────▼──────────┐
          │   AHB-to-APB  │    │   AHB Peripherals│
          │   Bridge      │    │   (GPIO, DMA,    │
          └────────┬──────┘    │    Ethernet)     │
                   │           └─────────────────┘
         ┌─────────┴──────────┐
         │    APB Bus         │  25 MHz (CCLK/4)
         │ (Advanced Peri-    │  ← most peripherals here
         │  pheral Bus)       │
         └─────────────────────┘
           UART0, SPI, I2C, ADC,
           RTC, PWM, Timer0–3
```

---

## Clock System

```
                                    ┌──────────────┐
XTAL (12 MHz)  ──────────────────→  │   Main PLL   │
                                    │  (PLLCFG)    │
                                    └──────┬───────┘
                                           │ PLL Output
                                           ▼
                                    ┌──────────────┐
                                    │  CPU Clock   │   CCLK = 100 MHz
                                    │  Divider     │   (CCLKCFG = 3 → PLL/4)
                                    └──────┬───────┘
                                           │
                           ┌───────────────┴──────────────────┐
                           │                                  │
                    ┌──────▼──────┐                  ┌────────▼────────┐
                    │  AHB Clock  │  100 MHz          │   PCLK Divider  │
                    │  (GPIO,DMA) │                   │   (PCLKSEL0/1)  │
                    └─────────────┘                   └────────┬────────┘
                                                               │  CCLK/4 = 25 MHz
                                                               │  (default for most peripherals)
                                                               ▼
                                                    UART, SPI, I2C, ADC,
                                                    Timer, PWM, RTC
```

### PCONP — Peripheral Power Control

Before using any peripheral, its clock must be **turned ON** using the `PCONP` register at `0x400FC0C4`.

| Bit | Peripheral | Value to Enable |
|-----|-----------|----------------|
| 1 | Timer0 | `PCONP |= (1 << 1)` |
| 3 | UART0 | `PCONP |= (1 << 3)` |
| 6 | PWM1 | `PCONP |= (1 << 6)` |
| 8 | SPI | `PCONP |= (1 << 8)` |
| 9 | RTC | `PCONP |= (1 << 9)` |
| 12 | ADC | `PCONP |= (1 << 12)` |
| 19 | I2C0 | `PCONP |= (1 << 19)` |
| 15 | GPIO | Always ON |

---

## NVIC — Nested Vectored Interrupt Controller

The Cortex-M3's NVIC handles all interrupts with **zero software overhead** for entering/exiting ISRs.

Key features:
- **Tail-chaining**: back-to-back ISRs without re-entering stack frame
- **Late arrival**: higher priority interrupt preempts during stacking
- **Priority levels**: 32 programmable priority levels on LPC1768

### Enabling an Interrupt (CMSIS)

```c
NVIC_EnableIRQ(TIMER0_IRQn);   // Enable Timer0 interrupt
NVIC_EnableIRQ(EINT3_IRQn);    // Enable GPIO interrupt (Port 2)
```

### ISR Naming Convention

Interrupt service routine names are **fixed** in the vector table (`startup_LPC17xx.s`):

| Interrupt | ISR Name |
|-----------|---------|
| Timer0 | `TIMER0_IRQHandler` |
| Timer1 | `TIMER1_IRQHandler` |
| UART0 | `UART0_IRQHandler` |
| GPIO (Port2) | `EINT3_IRQHandler` |

---

## Pin Multiplexing — PINSEL Registers

Every LPC1768 pin can serve multiple functions. The **PINSEL** registers select which function is active:

| PINSEL | Controls |
|--------|---------|
| PINSEL0 | P0.0–P0.15 |
| PINSEL1 | P0.16–P0.31 |
| PINSEL2 | P1.0–P1.15 |
| PINSEL3 | P1.16–P1.31 |
| PINSEL4 | P2.0–P2.15 |

Each pin uses **2 bits** in PINSEL:

| Value | Function |
|-------|---------|
| `00` | GPIO |
| `01` | Alternate Function 1 (e.g., UART) |
| `10` | Alternate Function 2 |
| `11` | Alternate Function 3 |

**Example — UART0 Pin Setup:**

```c
// P0.2 = TXD0, P0.3 = RXD0
// PINSEL0 bits [5:4] control P0.2, bits [7:6] control P0.3
LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));  // Clear
LPC_PINCON->PINSEL0 |=  ((1 << 4) | (1 << 6));  // Set to function 01
```

---

## Register-Based Programming Model

All LPC1768 peripherals are controlled through **memory-mapped registers**. These are predefined in `LPC17xx.h` (CMSIS header).

Access pattern:
```c
// Read:    value = LPC_PERIPHERAL->REGISTER;
// Write:   LPC_PERIPHERAL->REGISTER = value;
// Set bit: LPC_PERIPHERAL->REGISTER |= (1 << bit_number);
// Clear:   LPC_PERIPHERAL->REGISTER &= ~(1 << bit_number);
```

This is the foundation of every driver in this repository.
