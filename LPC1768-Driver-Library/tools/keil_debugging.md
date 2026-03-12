# Keil µVision Debugging Guide

This guide covers all debug tools available in Keil µVision for hardware-level debugging of LPC1768 drivers — whether using the built-in simulator or a real J-Link/ULINK debug adapter connected to the trainer board.

---

## Setting Up Debug Mode

### For Software Simulation (no hardware)
1. **Project → Options for Target → Debug tab**
2. Left side: select **☑ Use Simulator**
3. Press **OK**
4. Press **Ctrl+F5** to start

### For Hardware Debug (J-Link / ULINK)
1. Connect J-Link/ULINK2 to the 20-pin JTAG header on the trainer board
2. **Project → Options → Debug tab**
3. Right side: select **☑ Use: ULINK2/ME Cortex Debugger** (or J-Link)
4. Click **Settings** → verify connection
5. Press **Ctrl+F5** to download and start debug

---

## 1. Peripheral Register Window

The most powerful tool for verifying register configuration.

### How to Open
In debug mode: **Peripherals → [select category] → [select peripheral]**

Available peripheral categories:
```
Peripherals
├── GPIO
│   ├── GPIO Port 0
│   ├── GPIO Port 1
│   ├── GPIO Port 2
│   └── GPIO Port 3
├── UART
│   └── UART0
├── Timer
│   ├── Timer 0
│   └── Timer 1
├── ADC
├── I2C
│   └── I2C0
├── SPI
├── PWM
│   └── PWM1
└── RTC
```

### What to Verify in Each Driver

**GPIO Driver:**
- Open GPIO Port 1
- After `GPIO_Init_Output(1, LED_PIN)`: **FIODIR bit 18 = 1** ✓
- After `GPIO_Set(1, LED_PIN)`: **FIOPIN bit 18 = 1** ✓
- After `GPIO_Clear(1, LED_PIN)`: **FIOPIN bit 18 = 0** ✓

**UART Driver:**
- Open UART0
- After `UART0_Init(9600)`: **LCR = 0x03** (8N1, DLAB=0) ✓
- During `UART0_SendChar()`: **THR** shows loaded byte ✓
- **LSR bit 5 (THRE)** = 1 when transmitter is empty ✓

**ADC Driver:**
- Open ADC
- After `ADC_Init()`: **ADCR bit 21 (PDN)** = 1 (operational) ✓
- After starting: **ADCR bits [26:24]** = 001 (start) ✓
- When done: **ADGDR bit 31 (DONE)** = 1 ✓
- **ADGDR bits [15:4]** = conversion result ✓

**I2C Driver:**
- Open I2C0
- After `I2C0_Init(100000)`: **I2SCLH** and **I2SCLL** = 125 each ✓
- After `I2C0_Start()`: **I2CONSET bit 5 (STA)** then clears ✓
- **I2STAT** shows status codes (0x08 = START, 0x18 = SLA+W+ACK) ✓

**Timer Driver:**
- Open Timer 0
- After `Timer0_Init()`: **TCR bit 0** = 1 (running) ✓
- Watch **TC** (Timer Counter) increment each clock ✓
- When TC = MR0: **IR bit 0** sets, ISR fires, then clears ✓

**PWM Driver:**
- Open PWM1
- After `PWM1_Init(1000)`: **MR0** = 25000 (period) ✓
- **MR1** = 12500 (50% duty cycle) ✓
- **TCR bits [3,0]** = 1,1 (PWM + counter enabled) ✓

---

## 2. Watch Window

Monitor C variables in real time.

### Opening
**View → Watch Windows → Watch 1**

### How to Add Variables
1. Click in the "Name" column
2. Type the variable name
3. Press Enter

### Recommended Watch Variables

| Project | Variables to Watch |
|---------|-------------------|
| GPIO | `LED_PIN` |
| Timer | `timer_flag` |
| GPIO Interrupt | `gpio_irq_flag` |
| ADC | `adc_value` |
| I2C | `data` |
| RTC | `hr`, `min`, `sec` |
| LCD | `count` |

### Watch Window Tips

- Right-click a variable → **Add to Watch** (from source editor)
- Use `*ptr` to dereference pointers in Watch window
- Expressions also work: `LPC_GPIO1->FIOPIN & (1 << 18)` → shows LED state as 0 or 1

---

## 3. Memory Window

Inspect raw memory at any address.

### Opening
**View → Memory Windows → Memory 1**

### Using Memory Window

1. In the **Address** box: type address (e.g., `0x2009C034`)
2. Select display format: `8 Bits`, `16 Bits`, or `32 Bits`
3. Double-click a value to edit it (forces register writes during debug!)

### Critical Addresses

| Address | Register | Usage |
|---------|---------|-------|
| `0x2009C034` | GPIO1 FIOPIN | LED state — check bits 18–21 |
| `0x2009C000` | GPIO0 FIOPIN | P0 pin states |
| `0x2009C054` | GPIO2 FIOPIN | P2.10 button + 7-segment |
| `0x4000C014` | UART0 LSR | Bit 5 = TX ready, Bit 0 = RX ready |
| `0x40034004` | ADC ADGDR | Bits [15:4] = 12-bit result |
| `0x40024020` | RTC SEC | Running seconds value |
| `0x40004008` | TIM0 TC | Running timer counter |
| `0x40018018` | PWM1 MR0 | Period register value |

---

## 4. Logic Analyzer (Simulator Only)

Keil's simulator includes a Logic Analyzer that plots variable values over time as a waveform.

### Opening
**View → Analysis Windows → Logic Analyzer**

### Plotting GPIO Blink

1. In Logic Analyzer, click **Setup** (or paste into address box)
2. Add signal: `LPC_GPIO1->FIOPIN & 0x40000` (bit 18 mask → 0 or 0x40000)
3. Press **Run** (F5)
4. Watch the waveform toggle as LED blinks

### Plotting Timer Flag

Add: `timer_flag` — see it toggle 0/1 every 500 ms (simulated)

### Plotting ADC Value

Add: `LPC_ADC->ADGDR` — see the value update after each conversion

---

## 5. Breakpoints

### Setting a Breakpoint
- Click grey margin to the left of a source line
- Or: position cursor on the line and press **F9**
- Red dot = active breakpoint

### Breakpoint Types

| Type | How to Set | Use Case |
|------|-----------|---------|
| Line breakpoint | F9 on line | Pause at specific code line |
| Conditional | Right-click BP marker → Edit | Pause only when expression is true |
| Access | **Debug → Breakpoints → Access** | Pause when memory address is read/written |

### ISR Breakpoints (Critical)

To confirm interrupt handlers are firing:

```c
void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)
    {       ← Set breakpoint here
        LPC_TIM0->IR = 1;
        timer_flag ^= 1;
    }
}

void EINT3_IRQHandler(void)
{
    if (LPC_GPIOINT->IO2IntStatR & (1 << 10))
    {       ← Set breakpoint here
        LPC_GPIOINT->IO2IntClr = (1 << 10);
        LPC_GPIO1->FIOPIN ^= (1 << 18);
    }
}
```

When the breakpoint is hit, execution freezes — you can inspect all registers and variables at that exact moment.

---

## 6. Call Stack Window

Shows the current function call chain.

**View → Call Stack**

Useful for:
- Tracing which function called the current function
- Observing ISR entry (will show `TIMER0_IRQHandler` at the top of stack)
- Debugging infinite loops (shows where you're stuck)

---

## 7. Serial Debug (UART Printf)

For register-level debugging on real hardware, use `UART0_Printf()`:

```c
// Before a critical operation
UART0_Printf("Before: ADCR = %08X\r\n", LPC_ADC->ADCR);

// After
UART0_Printf("After:  ADCR = %08X\r\n", LPC_ADC->ADCR);
UART0_Printf("Result: %d\r\n", adc_value);
```

This is the most reliable debug technique for verifying register values on real hardware when a JTAG adapter is not available.

---

## 8. Common Debug Scenarios

| Problem | Debug Action |
|---------|-------------|
| LED not blinking | Check FIODIR bit 18 = 1 in Peripheral view |
| UART output garbage | Verify DLL/DLM divisor values in UART0 peripheral view |
| ADC reads 0 | Verify ADCR PDN bit = 1, verify DONE bit in ADGDR |
| I2C no ACK | Check I2STAT for error codes (0x20 = SLA+W NACK) |
| Timer ISR not firing | Check MCR interrupt enable bits, check NVIC_EnableIRQ call |
| SPI transfer wrong | Verify SPCR bit 5 (MSTR=1), check SPCCR ≥ 8 and even |
