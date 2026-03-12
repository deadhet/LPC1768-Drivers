# Document 06 — Central Register Reference

This document provides a complete, centralized reference for every hardware register used across all 14 drivers in this library. Use this as a quick lookup when reading driver code or debugging.

---

## How to Read This Table

- **Address**: Base memory address of the register
- **Access**: R (read-only), W (write-only), R/W (read-write)
- **Reset Value**: Value after power-on reset

---

## System Control Registers

| Register | C Reference | Address | Access | Description |
|---------|------------|---------|--------|-------------|
| PCONP | `LPC_SC->PCONP` | 0x400FC0C4 | R/W | Peripheral power control (enable clocks) |
| PCLKSEL0 | `LPC_SC->PCLKSEL0` | 0x400FC1A8 | R/W | Peripheral clock select (peripherals 0–15) |
| PCLKSEL1 | `LPC_SC->PCLKSEL1` | 0x400FC1AC | R/W | Peripheral clock select (peripherals 16–31) |

### PCONP Bit Map

| Bit | Peripheral | Enable Value |
|-----|-----------|-------------|
| 1 | Timer0 | `(1 << 1)` |
| 2 | Timer1 | `(1 << 2)` |
| 3 | UART0 | `(1 << 3)` |
| 4 | UART1 | `(1 << 4)` |
| 6 | PWM1 | `(1 << 6)` |
| 8 | SPI | `(1 << 8)` |
| 9 | RTC | `(1 << 9)` |
| 12 | ADC | `(1 << 12)` |
| 15 | GPIO | Always ON |
| 19 | I2C0 | `(1 << 19)` |
| 26 | I2C1 | `(1 << 26)` |

---

## Pin Connect (PINCON) Registers

| Register | C Reference | Address | Controls |
|---------|------------|---------|---------|
| PINSEL0 | `LPC_PINCON->PINSEL0` | 0x4002C000 | P0.0–P0.15 |
| PINSEL1 | `LPC_PINCON->PINSEL1` | 0x4002C004 | P0.16–P0.31 |
| PINSEL3 | `LPC_PINCON->PINSEL3` | 0x4002C00C | P1.16–P1.31 |
| PINSEL4 | `LPC_PINCON->PINSEL4` | 0x4002C010 | P2.0–P2.15 |

---

## GPIO Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| P0 DIR | `LPC_GPIO0->FIODIR` | 0x2009C000 | Direction: 1=output, 0=input |
| P0 PIN | `LPC_GPIO0->FIOPIN` | 0x2009C014 | Read pin level |
| P0 SET | `LPC_GPIO0->FIOSET` | 0x2009C018 | Set pins HIGH |
| P0 CLR | `LPC_GPIO0->FIOCLR` | 0x2009C01C | Set pins LOW |
| P1 DIR | `LPC_GPIO1->FIODIR` | 0x2009C020 | Direction |
| P1 PIN | `LPC_GPIO1->FIOPIN` | 0x2009C034 | Pin state |
| P1 SET | `LPC_GPIO1->FIOSET` | 0x2009C038 | Set HIGH |
| P1 CLR | `LPC_GPIO1->FIOCLR` | 0x2009C03C | Set LOW |
| P2 DIR | `LPC_GPIO2->FIODIR` | 0x2009C040 | Direction |
| P2 PIN | `LPC_GPIO2->FIOPIN` | 0x2009C054 | Pin state |
| P2 SET | `LPC_GPIO2->FIOSET` | 0x2009C058 | Set HIGH |
| P2 CLR | `LPC_GPIO2->FIOCLR` | 0x2009C05C | Set LOW |

---

## GPIO Interrupt Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| IO0IntEnR | `LPC_GPIOINT->IO0IntEnR` | 0x40028090 | P0 rising edge interrupt enable |
| IO0IntEnF | `LPC_GPIOINT->IO0IntEnF` | 0x40028094 | P0 falling edge interrupt enable |
| IO0IntStatR | `LPC_GPIOINT->IO0IntStatR` | 0x40028084 | P0 rising edge status |
| IO0IntClr | `LPC_GPIOINT->IO0IntClr` | 0x4002808C | P0 interrupt clear |
| IO2IntEnR | `LPC_GPIOINT->IO2IntEnR` | 0x400280B0 | P2 rising edge interrupt enable |
| IO2IntEnF | `LPC_GPIOINT->IO2IntEnF` | 0x400280B4 | P2 falling edge interrupt enable |
| IO2IntStatR | `LPC_GPIOINT->IO2IntStatR` | 0x400280A4 | P2 rising edge status |
| IO2IntClr | `LPC_GPIOINT->IO2IntClr` | 0x400280AC | P2 interrupt clear |

---

## UART0 Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| RBR | `LPC_UART0->RBR` | 0x4000C000 | Receiver Buffer Register (DLAB=0, read) |
| THR | `LPC_UART0->THR` | 0x4000C000 | Transmit Holding Register (DLAB=0, write) |
| DLL | `LPC_UART0->DLL` | 0x4000C000 | Divisor Latch LSB (DLAB=1) |
| DLM | `LPC_UART0->DLM` | 0x4000C004 | Divisor Latch MSB (DLAB=1) |
| IER | `LPC_UART0->IER` | 0x4000C004 | Interrupt Enable Register |
| FCR | `LPC_UART0->FCR` | 0x4000C008 | FIFO Control Register (write) |
| LCR | `LPC_UART0->LCR` | 0x4000C00C | Line Control Register |
| LSR | `LPC_UART0->LSR` | 0x4000C014 | Line Status Register |

### LCR Bit Fields

| Bits | Field | Values |
|------|-------|--------|
| [1:0] | Word Length | 00=5-bit, 01=6, 10=7, 11=8-bit |
| [2] | Stop bits | 0=1 stop, 1=2 stop |
| [3] | Parity enable | 0=none, 1=enabled |
| [7] | DLAB | 1=access DLL/DLM, 0=normal |

### LSR Bit Fields

| Bit | Field | Meaning |
|-----|-------|---------|
| 0 | RDR | Receiver Data Ready — data available to read |
| 5 | THRE | Transmit Holding Register Empty — OK to write |
| 6 | TEMT | Transmitter Empty — shift register empty |

---

## ADC Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| ADCR | `LPC_ADC->ADCR` | 0x40034000 | ADC Control Register |
| ADGDR | `LPC_ADC->ADGDR` | 0x40034004 | ADC Global Data Register (result) |
| ADINTEN | `LPC_ADC->ADINTEN` | 0x4003400C | ADC Interrupt Enable |

### ADCR Bit Fields

| Bits | Field | Description |
|------|-------|-------------|
| [7:0] | SEL | Channel select (bit 0=Ch0, bit 1=Ch1, ...) |
| [15:8] | CLKDIV | Clock divider (ADC clock = PCLK / (CLKDIV+1)) |
| [16] | BURST | Burst conversion mode |
| [21] | PDN | Power-down: 0=powered down, 1=operational |
| [26:24] | START | 000=no start, 001=start now |

### ADGDR Bit Fields

| Bits | Field | Description |
|------|-------|-------------|
| [15:4] | RESULT | 12-bit conversion result |
| [26:24] | CHN | Channel that generated this result |
| [31] | DONE | 1 = conversion complete |

---

## I2C0 Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| I2CONSET | `LPC_I2C0->I2CONSET` | 0x4001C000 | Control Set |
| I2STAT | `LPC_I2C0->I2STAT` | 0x4001C004 | Status |
| I2DAT | `LPC_I2C0->I2DAT` | 0x4001C008 | Data Register |
| I2CONCLR | `LPC_I2C0->I2CONCLR` | 0x4001C018 | Control Clear |
| I2SCLH | `LPC_I2C0->I2SCLH` | 0x4001C010 | SCL High period |
| I2SCLL | `LPC_I2C0->I2SCLL` | 0x4001C014 | SCL Low period |

### I2CONSET Bit Fields

| Bit | Field | Function |
|-----|-------|---------|
| 2 | AA | Assert Acknowledge — send ACK after receive |
| 3 | SI | Serial Interrupt flag — set when state changes |
| 4 | STO | STOP flag — generate STOP condition |
| 5 | STA | START flag — generate START condition |
| 6 | I2EN | I2C Enable |

---

## SPI Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| SPCR | `LPC_SPI->SPCR` | 0x40020000 | SPI Control Register |
| SPSR | `LPC_SPI->SPSR` | 0x40020004 | SPI Status Register |
| SPDR | `LPC_SPI->SPDR` | 0x40020008 | SPI Data Register |
| SPCCR | `LPC_SPI->SPCCR` | 0x4002000C | SPI Clock Counter |

### SPCR Bit Fields

| Bit | Field | Description |
|-----|-------|-------------|
| 3 | CPHA | Clock Phase: 0=sample on first edge, 1=second edge |
| 4 | CPOL | Clock Polarity: 0=idle LOW, 1=idle HIGH |
| 5 | MSTR | Master/Slave: 1=Master, 0=Slave |
| 6 | LSBF | LSB first: 0=MSB first (standard) |
| 7 | SPIE | SPI Interrupt Enable |
| [11:8] | BITS | Data bit size: 1000=8-bit, 1001=9-bit, etc. |

### SPSR Bit Fields

| Bit | Field | Meaning |
|-----|-------|---------|
| 7 | SPIF | SPI Transfer Complete — transfer done |

---

## RTC Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| ILR | `LPC_RTC->ILR` | 0x40024000 | Interrupt Location Register |
| CCR | `LPC_RTC->CCR` | 0x40024008 | Clock Control Register |
| SEC | `LPC_RTC->SEC` | 0x40024020 | Seconds (0–59) |
| MIN | `LPC_RTC->MIN` | 0x40024024 | Minutes (0–59) |
| HOUR | `LPC_RTC->HOUR` | 0x40024028 | Hours (0–23) |
| DOM | `LPC_RTC->DOM` | 0x4002402C | Day of Month (1–28/31) |
| DOW | `LPC_RTC->DOW` | 0x40024030 | Day of Week (0–6) |
| DOY | `LPC_RTC->DOY` | 0x40024034 | Day of Year (1–366) |
| MONTH | `LPC_RTC->MONTH` | 0x40024038 | Month (1–12) |
| YEAR | `LPC_RTC->YEAR` | 0x4002403C | Year (0–4095) |

### CCR Bit Fields

| Bit | Field | Description |
|-----|-------|-------------|
| 0 | CLKEN | 1=Enable RTC counter, 0=Halt |
| 1 | CTCRST | 1=Reset counters, self-clearing |

---

## Timer0 Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| IR | `LPC_TIM0->IR` | 0x40004000 | Interrupt Register |
| TCR | `LPC_TIM0->TCR` | 0x40004004 | Timer Control Register |
| TC | `LPC_TIM0->TC` | 0x40004008 | Timer Counter |
| PR | `LPC_TIM0->PR` | 0x4000400C | Prescale Register |
| PC | `LPC_TIM0->PC` | 0x40004010 | Prescale Counter |
| MCR | `LPC_TIM0->MCR` | 0x40004014 | Match Control Register |
| MR0 | `LPC_TIM0->MR0` | 0x40004018 | Match Register 0 |
| MR1 | `LPC_TIM0->MR1` | 0x4000401C | Match Register 1 |

### TCR Bit Fields

| Bit | Field | Description |
|-----|-------|-------------|
| 0 | CEN | Counter Enable: 1=run, 0=stop |
| 1 | CRST | Counter Reset: 1=reset TC and PC |

### MCR Bit Fields (per match channel)

| Bits | For MR0 | Description |
|------|--------|-------------|
| 0 | MR0I | Interrupt on MR0 match |
| 1 | MR0R | Reset TC on MR0 match |
| 2 | MR0S | Stop TC on MR0 match |

---

## PWM1 Registers

| Register | C Reference | Address | Description |
|---------|------------|---------|-------------|
| IR | `LPC_PWM1->IR` | 0x40018000 | Interrupt Register |
| TCR | `LPC_PWM1->TCR` | 0x40018004 | Timer Control Register |
| PR | `LPC_PWM1->PR` | 0x4001800C | Prescale |
| MCR | `LPC_PWM1->MCR` | 0x40018014 | Match Control |
| MR0 | `LPC_PWM1->MR0` | 0x40018018 | PWM Period |
| MR1 | `LPC_PWM1->MR1` | 0x4001801C | PWM Ch.1 Duty |
| PCR | `LPC_PWM1->PCR` | 0x4001804C | PWM Control |
| LER | `LPC_PWM1->LER` | 0x40018050 | Load Enable Register |

### TCR for PWM Mode

| Bits | Setting |
|------|---------|
| `(1 << 0)` | Counter Enable |
| `(1 << 3)` | PWM Enable |

### LER — Latch Enable

Write a 1 to bit N to latch MR(N) on next PWM cycle:
```c
LPC_PWM1->LER |= (1 << 1);  // Latch MR1 (duty cycle)
```
