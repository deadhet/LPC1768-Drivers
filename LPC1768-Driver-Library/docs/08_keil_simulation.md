# Document 08 — Keil Simulation Guide

**No hardware? No problem.** Keil µVision includes a powerful software simulator that can execute your LPC1768 code, observe register state changes, simulate interrupts, and even display communication waveforms — all without a physical board.

---

## Setting Up the Simulator

### Step 1: Open Project Options

1. Open your Keil project
2. Go to **Project → Options for Target** (or press Alt+F7)

### Step 2: Configure Debug Tab

1. Click the **Debug** tab
2. On the **left side**, select **Use Simulator**
3. Uncheck "Load Application at Startup" if you want to manually control loading
4. Click **OK**

### Step 3: Start Debug Session

1. Press **Ctrl+F5** or **Debug → Start/Stop Debug Session**
2. Keil will compile, link, and load the program into the simulator
3. The debug toolbar will appear at the top

---

## Debug Toolbar

```
[Run] [Stop] [Step] [Step Over] [Step Out] [Run to Cursor]
 F5    Stop   F11     F10         Ctrl+F11    Ctrl+F10
```

| Button | Shortcut | Action |
|--------|---------|--------|
| Run | F5 | Execute until breakpoint or stop |
| Stop | Esc | Halt execution |
| Step | F11 | Execute one instruction (enter functions) |
| Step Over | F10 | Execute one line (skip over functions) |
| Reset | — | Reset simulation to reset vector |

---

## 1. Peripheral Register View

The **Peripheral** window shows all hardware registers updating in real time during simulation.

### Opening Peripheral View

1. While in debug mode, go to **Peripherals** menu
2. Select the peripheral you want to observe:
   - `Peripherals → GPIO → GPIO Port 0`
   - `Peripherals → UART → UART0`
   - `Peripherals → ADC → ADC`
   - `Peripherals → Timer → Timer 0`
   - `Peripherals → PWM → PWM1`
   - `Peripherals → RTC`

### What to Observe

**GPIO Driver:**
```
Open: Peripherals → GPIO → GPIO Port 1
Step through GPIO_Init_Output(1, LED_PIN)
Watch: FIODIR bit 18 → changes from 0 to 1 ✓

Step through GPIO_Set(1, LED_PIN)
Watch: FIOPIN bit 18 → changes to 1 ✓

Step through GPIO_Clear(1, LED_PIN)
Watch: FIOPIN bit 18 → changes to 0 ✓
```

**UART Driver:**
```
Open: Peripherals → UART → UART0
Step through UART0_Init(9600)
Watch: LCR → DLAB bit set then cleared
Watch: DLL/DLM → divisor values written
Watch: FCR → FIFO enable bits set

Then call UART0_SendChar('A')
Watch: THR → 0x41 loaded
Watch: LSR → THRE bit toggles
```

**ADC Driver:**
```
Open: Peripherals → ADC
Step through ADC_Init()
Watch: ADCR → PDN bit (bit 21) changes to 1
Watch: ADCR → SEL bits change to select channel

Step through ADC_Read(1)
Watch: ADCR → START bits (24) set to 001
Watch: ADGDR → DONE bit (31) goes to 1
Watch: ADGDR → RESULT field [15:4] = conversion result
```

---

## 2. Watch Window

The Watch window lets you monitor C variables in real-time.

### Opening Watch Window

1. In debug mode: **View → Watch Windows → Watch 1**

### Adding Variables

1. Click in the "Name" column of the Watch window
2. Type the variable name and press Enter

**Variables to watch per driver:**

| Driver | Variables to Add |
|--------|-----------------|
| GPIO | *(none — use Peripheral view)* |
| UART | *(none — check LSR via Peripheral view)* |
| ADC | `adc_value` |
| I2C | `data` |
| RTC | `hr`, `min`, `sec` |
| Timer | `timer_flag` |
| GPIO Interrupt | `gpio_irq_flag` |
| LCD | `count` |

### Example: Watching Timer Flag

```
1. Open drivers/timer/main.c project
2. Start simulation (Ctrl+F5)
3. Open Watch 1 → add "timer_flag"
4. Press Run (F5)
5. Observe timer_flag toggling between 0 and 1
   as Timer0 ISR fires every 500 ms
```

---

## 3. Memory Window

The Memory window lets you directly inspect memory at any address.

### Opening Memory Window

**View → Memory Windows → Memory 1**

### Useful Addresses to Inspect

| Address | What You See |
|---------|-------------|
| `0x2009C034` | GPIO PORT1 FIOPIN — see LED state at bit 18 |
| `0x2009C054` | GPIO PORT2 FIOPIN — see button/7-seg state |
| `0x2009C000` | GPIO PORT0 FIOPIN |
| `0x4000C014` | UART0 LSR — line status (bit 5 = TX empty) |
| `0x40034004` | ADC ADGDR — conversion result register |
| `0x40024020` | RTC SEC — seconds register ticking |
| `0x40004008` | Timer0 TC — counter incrementing |

### How to Use

1. Type an address in the **Address** box
2. Select display format: **8-bit** (for bytes), **32-bit** (for registers)
3. Watch values update as simulation runs

---

## 4. Setting Breakpoints

Breakpoints pause execution at a specific line so you can inspect state.

### Toggle a Breakpoint

Click in the grey margin to the left of any source line, or press **F9**.

```
A red dot appears → breakpoint active
Press F5 → simulator runs until that line
```

### Useful Breakpoints to Set

**GPIO Driver:**
```c
GPIO_Set(1, LED_PIN);      // ← Breakpoint here
// Step once → observe FIOSET written → FIOPIN changes
GPIO_Clear(1, LED_PIN);    // ← Breakpoint here
```

**UART Driver:**
```c
LPC_UART0->THR = ch;       // ← Breakpoint here: observe byte loaded
```

**Timer ISR:**
```c
void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)
    {
        LPC_TIM0->IR = 1;   // ← Breakpoint: ISR entry confirmed
        timer_flag ^= 1;
    }
}
```

---

## 5. Simulating GPIO Blink (Full Example)

```
1. Open drivers/gpio project in Keil
2. Project → Options → Debug → Use Simulator → OK
3. Press Ctrl+F5 (start debug)
4. Set breakpoint on: GPIO_Set(1, LED_PIN);
5. Open Peripherals → GPIO → GPIO Port 1
6. Press F5 — execution stops at GPIO_Set()
7. Press F10 (Step Over) — observe FIOPIN bit 18 = 1 (LED ON)
8. Continue to GPIO_Clear() breakpoint
9. Press F10 — observe FIOPIN bit 18 = 0 (LED OFF)
10. Press F5 to run continuously — watch bit 18 toggle
```

---

## 6. Simulating Timer Interrupts

The simulator can trigger peripheral interrupts automatically.

```
1. Open drivers/timer project
2. Start simulation (Ctrl+F5)
3. Add "timer_flag" to Watch window
4. Open Peripherals → Timer → Timer 0
5. Press Run (F5)
6. Watch TC (Timer Counter) increment
7. When TC reaches MR0 (2000), TIMER0_IRQHandler fires
8. Watch timer_flag toggle in Watch window
9. Watch IR (IR register) clear automatically
```

---

## 7. Limitations of Simulation

| Feature | Simulated? | Notes |
|---------|-----------|-------|
| GPIO bit toggling | ✅ Yes | Visible in Peripheral view |
| Timer counting | ✅ Yes | TC increments correctly |
| UART register access | ✅ Yes | THR/RBR updates visible |
| ADC conversion | ⚠️ Partial | ADGDR result is simulated |
| I2C frames | ⚠️ Partial | Register write/read works, not actual I2C bus |
| Physical LED blinking | ❌ No | Requires real hardware |
| Serial terminal output | ❌ No | Requires real UART/board |
| SPI waveform | ❌ No | Not fully simulated |

**Simulation is excellent for:** Register configuration verification, logic flow, interrupt timing, and register-level debugging.

**For actual I/O behavior:** Flash to real hardware — see [`tools/flashing_guide.md`](../tools/flashing_guide.md).
