# Timer Driver — LPC1768

## 1. Driver Overview

The **Timer/Counter** peripheral on the LPC1768 is a hardware counter that increments at a configurable rate without requiring CPU time. Once configured and started, it counts completely independently and can trigger an interrupt precisely when its count reaches a target value. This is fundamentally different from software delays, which waste CPU cycles in a busy loop.

The LPC1768 has four independent timer peripherals: Timer0, Timer1, Timer2, and Timer3. Each has its own presaler, match registers, and capture registers. This driver uses **Timer0** with NVIC integration to toggle an LED every 500 milliseconds via a hardware interrupt.

**Real-world applications:**
- Precise time delays without wasting CPU cycles
- Periodic event scheduling (sensor polling, display refresh)
- Manual PWM generation on arbitrary pins
- Debounce timing for buttons
- Motor step pulse timing for stepper motors

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| LED1 | PORT1 | P1.18 | Output | Toggled by Timer0 ISR every 500 ms |

Timer0 is purely internal — no external pin is needed for the counter itself. The match event is delivered internally to the NVIC which then calls the interrupt handler.

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power Control

| Bit | Value | Description |
|-----|-------|-------------|
| 1 | 1 | Enable Timer0 power and clock |

### LPC_SC→PCLKSEL0 — Peripheral Clock Select

| Bits | Value | Clock | Description |
|------|-------|-------|-------------|
| [3:2] | 00 | CCLK/4 = 25 MHz | Timer0 peripheral clock (default) |

### LPC_TIM0→TCR — Timer Control Register

| Bit | Field | Value | Effect |
|-----|-------|-------|--------|
| 0 | CEN | 1 | Counter Enable — start counting |
| 1 | CRST | 1 | Counter Reset — reset and hold TC and PC at 0 |

### LPC_TIM0→PR — Prescaler Register

The Prescaler Register divides the peripheral clock before it reaches the timer counter. The effective divider is PR+1. When the prescaler counter (PC) reaches PR, TC increments and PC resets to 0.

```
PR = 25000 - 1 = 24999

TC increments every:
  (PR + 1) / PCLK = 25000 / 25,000,000 = 1 ms per count
```

### LPC_TIM0→MR0 — Match Register 0

When the Timer Counter (TC) equals this value, an event is triggered. With TC incrementing every 1 ms:

```
MR0 = 500  →  interrupt triggered every 500 × 1 ms = 500 ms
```

### LPC_TIM0→MCR — Match Control Register

Each match register (MR0–MR3) has 3 associated control bits in MCR:

| Bit | Field | Effect |
|-----|-------|--------|
| 0 | MR0I | Generate interrupt when TC = MR0 |
| 1 | MR0R | Reset TC to 0 when TC = MR0 |
| 2 | MR0S | Stop timer when TC = MR0 |

This driver uses `(1 << 0) | (1 << 1)` = MR0I + MR0R: interrupt on match, then reset timer. This creates a repeating 500 ms interval.

### LPC_TIM0→IR — Interrupt Register

Each match register has its own interrupt flag bit. The timer sets the bit when it fires an interrupt. The ISR must clear this bit manually by writing 1 to it. If the bit is not cleared, the interrupt fires again immediately.

| Bit | Field | Description |
|-----|-------|-------------|
| 0 | MR0INT | Match 0 interrupt — set by hardware, cleared by writing 1 |

### NVIC_EnableIRQ(TIMER0_IRQn)

The NVIC (Nested Vectored Interrupt Controller) is the Cortex-M3 component that manages all interrupts. Even if the timer generates a match event and sets the IR flag, the NVIC must be told to route that interrupt to the CPU. `NVIC_EnableIRQ(TIMER0_IRQn)` enables the Timer0 interrupt vector in the NVIC.

## 4. Driver Architecture

```
Timer0_Init()
  ├─ PCONP[1] = 1 (power on)
  ├─ PCLKSEL0[3:2] = 00 (25 MHz PCLK)
  ├─ TCR[1] = 1 (reset counter)
  ├─ PR = 24999 (TC increments every 1 ms)
  ├─ MR0 = 500 (interrupt every 500 ms)
  ├─ MCR = interrupt + reset on MR0 match
  ├─ NVIC_EnableIRQ(TIMER0_IRQn)
  └─ TCR[0] = 1 (start counting)

TIMER0_IRQHandler()  [called by NVIC every 500 ms]
  ├─ Check IR[0] (confirm MR0 interrupt)
  ├─ IR = 1 (clear interrupt flag — must be done first)
  └─ timer_flag ^= 1 (toggle software flag)

main() polls timer_flag:
  if (timer_flag) → LED ON
  else            → LED OFF
```

## 5. Function Reference

### `Timer0_Init(void)`
Configures Timer0 to fire an interrupt every 500 ms. Enables NVIC interrupt. Starts the timer.

### `TIMER0_IRQHandler(void)` (ISR)
Called automatically by the NVIC every 500 ms. Clears the interrupt flag and toggles `timer_flag`.

## 6. Code Walkthrough

### Enabling Timer0 Power

```c
LPC_SC->PCONP |= (1 << 1);
```

Bit 1 of PCONP is PCTIM0. Setting it enables the Timer0 peripheral clock. If this is not done, Timer0 registers are inert.

### Setting the Peripheral Clock

```c
LPC_SC->PCLKSEL0 &= ~(3 << 2);
```

Bits [3:2] of PCLKSEL0 control the Timer0 PCLK divider. Clearing both bits to 00 selects CCLK/4 = 25 MHz. This is the default, but clearing it explicitly ensures the timer calculation is correct.

### Resetting the Counter

```c
LPC_TIM0->TCR = (1 << 1);
```

Writing 1 to bit 1 of TCR (CRST) resets the Timer Counter (TC) and the Prescaler Counter (PC) to 0, and holds them at 0. This ensures the timer starts from a known state.

### Setting the Prescaler for 1 ms Resolution

```c
LPC_TIM0->PR = 25000 - 1;
```

The Prescaler Register value of 24999 causes the hardware prescaler counter to count from 0 to 24999 (25,000 cycles) before incrementing TC by 1. This converts the 25 MHz PCLK into a 1,000 Hz TC clock — one TC increment per millisecond.

### Setting the Match Value for 500 ms Interval

```c
LPC_TIM0->MR0 = 500;
```

With TC incrementing every 1 ms, a match value of 500 means the interrupt fires when TC = 500, which occurs 500 ms after the timer starts (or after the last auto-reset). After the match event, MCR's MR0R bit causes TC to reset to 0 automatically, starting the next 500 ms interval.

### Configuring Match Control

```c
LPC_TIM0->MCR = (1 << 0) | (1 << 1);
```

Bit 0 (MR0I): generate an interrupt when TC = MR0. Bit 1 (MR0R): reset TC when TC = MR0. Together, these create a repeating, interrupt-driven 500 ms tick.

### Enabling the NVIC and Starting the Timer

```c
NVIC_EnableIRQ(TIMER0_IRQn);
LPC_TIM0->TCR = 1;
```

`NVIC_EnableIRQ()` is a CMSIS function that sets the enable bit for the TIMER0 interrupt in the NVIC. Without this, the timer would reach MR0 and set the IR flag, but the CPU would never be interrupted.

Writing 1 to TCR bit 0 (CEN) starts the timer. The counter begins incrementing immediately.

### The Interrupt Handler

```c
void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)
    {
        LPC_TIM0->IR = 1;     // Clear interrupt flag (write 1 to clear)
        timer_flag ^= 1;      // Toggle flag
    }
}
```

The function name `TIMER0_IRQHandler` is special — it must match exactly this name for the NVIC to call it. The function checks bit 0 of IR to confirm the interrupt is from MR0 (not a capture event or another match). The IR bit is cleared by writing 1 to it. This is a "write-1-to-clear" mechanism — writing 0 has no effect. The `^= 1` operation toggles the timer_flag between 0 and 1 on each call.

## 7. Test Program Explanation

The main.c sets P1.18 as an output, then polls `timer_flag` in the main loop. The timer ISR runs autonomously in the background:

- At 0 ms: timer starts, timer_flag = 0 (LED OFF)
- At 500 ms: ISR fires, timer_flag = 1 (LED ON)
- At 1000 ms: ISR fires again, timer_flag = 0 (LED OFF)
- Continues indefinitely at exactly 500 ms intervals

This demonstrates interrupt-driven control: the LED blink timing is entirely hardware-determined and is accurate to within one PCLK cycle.

## 8. Hardware Testing Procedure

### Expected Output
LED at P1.18 blinks at exactly 1 Hz (500 ms ON, 500 ms OFF), driven by hardware timer interrupt — not software delay.

### Init Flow Summary
```
PCONP[1] = 1
      │
PCLKSEL0 = 00 (25 MHz)
      │
TCR reset → PR=24999 → MR0=500
      │
MCR: interrupt + reset on match
      │
NVIC_EnableIRQ(TIMER0_IRQn)
      │
TCR[0] = 1 (start)
      │
Every 500ms → TIMER0_IRQHandler fires → timer_flag toggles
```

### Debugging in Keil
- **Peripheral → Timer 0:** watch TC increment continuously, TC resets at 500
- **Watch `timer_flag`:** toggles between 0 and 1 every 500 ms
- **Breakpoint in `TIMER0_IRQHandler`:** confirms ISR fires at correct interval
- **Peripheral → NVIC:** verify TIMER0_IRQn is enabled

## 9. Expected Output

```
LED P1.18: ON (500ms) → OFF (500ms) → ON (500ms) → ...
(Controlled entirely by hardware timer interrupt)
```
