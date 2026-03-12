# Timer Driver — LPC1768

## 1. Driver Overview

The **Timer/Counter** peripheral generates precise time delays and periodic interrupts without consuming CPU cycles in a polling loop. This driver uses **Timer0** with NVIC integration to toggle an LED every 500 milliseconds.

**Real-world applications:** Precise time delays, event scheduling, PWM generation (manual), watchdog-style monitoring, motor step timing.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| LED1 | PORT1 | P1.18 | Output | Toggled by Timer0 ISR |

Timer0 is purely internal — no external pin needed for the counter itself.

---

## 3. Registers Used

| Register | Value | Description |
|---------|-------|-------------|
| `PCONP` bit 1 | `(1 << 1)` | Enable Timer0 power |
| `PCLKSEL0` [3:2] | `00` | Timer0 PCLK = CCLK/4 = 25 MHz |
| `LPC_TIM0->TCR` | `(1<<1)` then `1` | Reset, then start |
| `LPC_TIM0->PR` | 25000-1 | Prescaler: TC increments every 1 ms |
| `LPC_TIM0->MR0` | 500 | Match at 500 → interrupt every 500 ms |
| `LPC_TIM0->MCR` | `(1<<0)\|(1<<1)` | Interrupt on MR0 + reset TC |
| `NVIC_EnableIRQ` | TIMER0_IRQn | Enable Timer0 interrupt in NVIC |

**Prescaler Calculation:**
```
PCLK = 25 MHz
PR = 25000 - 1  →  TC increments every (PR+1)/PCLK = 1 ms
MR0 = 500       →  interrupt every 500 ms
```

---

## 4. Driver Architecture

```
Timer0_Init()
  ├─ PCONP[1] = 1 (enable)
  ├─ PCLKSEL0[3:2] = 00 (PCLK/4)
  ├─ TCR[1] = 1 (reset counter)
  ├─ PR = 24999 (1ms tick)
  ├─ MR0 = 500 (500ms interval)
  ├─ MCR = interrupt + reset on MR0
  ├─ NVIC_EnableIRQ(TIMER0_IRQn)
  └─ TCR[0] = 1 (start counting)

TIMER0_IRQHandler()  [called by NVIC every 500 ms]
  ├─ Check IR[0] (MR0 match interrupt)
  ├─ Clear IR = 1 (acknowledge interrupt)
  └─ timer_flag ^= 1 (toggle flag)

main() polls timer_flag:
  if (timer_flag) → LED ON
  else            → LED OFF
```

---

## 5. Code Walkthrough

```c
void Timer0_Init(void)
{
    LPC_SC->PCONP |= (1 << 1);           // Enable Timer0 power
    LPC_SC->PCLKSEL0 &= ~(3 << 2);       // Timer0 PCLK = CCLK/4 (clear = 00)
    LPC_TIM0->TCR = (1 << 1);            // Reset counter and prescaler
    LPC_TIM0->PR = 25000 - 1;            // Each TC increment = 1 ms
    LPC_TIM0->MR0 = 500;                 // Match every 500 increments = 500 ms
    LPC_TIM0->MCR = (1 << 0) | (1 << 1); // MR0I=1 (interrupt), MR0R=1 (reset TC)
    NVIC_EnableIRQ(TIMER0_IRQn);         // Enable Timer0 in NVIC
    LPC_TIM0->TCR = 1;                   // Start timer (CEN=1)
}

void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)     // Confirm MR0 match interrupt
    {
        LPC_TIM0->IR = 1;     // Clear interrupt by writing 1 to IR[0]
        timer_flag ^= 1;      // Toggle flag (1 XOR 1 = 0, 0 XOR 1 = 1)
    }
}
```

---

## 6. Test Program (main.c)

Polls `timer_flag` in main loop — LED on P1.18 toggles every 500 ms via the ISR.

---

## 7. Hardware Testing Procedure

### Expected Output
LED at P1.18 **blinks** at exactly 1 Hz (500 ms ON, 500 ms OFF) driven by hardware timer interrupt, not software delay.

### Init Flow Diagram
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
Every 500ms → TIMER0_IRQHandler → timer_flag toggles
```

### Debugging
- **Peripheral → Timer 0:** watch TC increment, reset at 500
- **Watch `timer_flag`:** toggles 0/1 every 500ms
- **Breakpoint in `TIMER0_IRQHandler`:** confirms ISR is called
