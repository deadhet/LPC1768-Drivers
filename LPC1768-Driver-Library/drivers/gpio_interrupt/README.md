# GPIO Interrupt Driver — LPC1768

## 1. Driver Overview

This driver demonstrates **interrupt-driven GPIO input** using the LPC1768's external interrupt system (EINT3) mapped to PORT2. Instead of polling a button in a loop, the CPU is interrupted automatically when the button state changes.

**Real-world applications:** Emergency stop buttons, door sensors, encoder counting, event capture, power button handling.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| User Button | PORT2 | P2.10 | Input | Interrupt source (active LOW) |
| LED1 | PORT1 | P1.18 | Output | Toggled by ISR |

**On the trainer board:** User button at P2.10 is active LOW — pressed = LOW, released = HIGH. The driver uses **rising edge** interrupt (LOW → HIGH = "button released").

---

## 3. Registers Used

| Register | Value | Description |
|---------|-------|-------------|
| `LPC_GPIO2->FIODIR` | `&= ~(1<<10)` | P2.10 = Input (clear bit) |
| `LPC_GPIOINT->IO2IntEnR` | `\|(1<<10)` | Enable rising edge interrupt on P2.10 |
| `NVIC_EnableIRQ(EINT3_IRQn)` | — | Enable EINT3 in NVIC (Port2 GPIO IRQ vector) |
| `LPC_GPIOINT->IO2IntStatR` | `& (1<<10)` | Check if P2.10 was the interrupt source |
| `LPC_GPIOINT->IO2IntClr` | `= (1<<10)` | Clear interrupt pending flag |

**Note:** GPIO interrupt on **PORT2** shares the **EINT3** vector. The ISR is named `EINT3_IRQHandler`.

---

## 4. Driver Architecture

```
GPIO2_Interrupt_Init()
  ├─ FIODIR P2.10 = Input (0)
  ├─ IO2IntEnR[10] = 1 (rising edge enable)
  └─ NVIC_EnableIRQ(EINT3_IRQn)

CPU runs main() doing nothing (or other work):
  while(1) { /* wait for interrupt */ }

Button pressed → released (rising edge on P2.10):
  └─ EINT3_IRQHandler() triggered by NVIC
         │
         ├─ Check IO2IntStatR[10] = 1 (confirm source)
         ├─ IO2IntClr = (1<<10)  (clear flag FIRST)
         ├─ GPIO1->FIOPIN ^= (1<<18) (toggle LED)
         └─ gpio_irq_flag ^= 1 (toggle software flag)
```

---

## 5. Code Walkthrough

```c
void GPIO2_Interrupt_Init(void)
{
    LPC_GPIO2->FIODIR &= ~(1 << 10);       // P2.10 as input
    LPC_GPIOINT->IO2IntEnR |= (1 << 10);   // Enable rising edge on P2.10
    NVIC_EnableIRQ(EINT3_IRQn);            // Enable EINT3 in NVIC
    // EINT3 = Port 0 and Port 2 GPIO interrupts share this vector
}

void EINT3_IRQHandler(void)
{
    if (LPC_GPIOINT->IO2IntStatR & (1 << 10)) // Was it P2.10? (port 2, pin 10)
    {
        LPC_GPIOINT->IO2IntClr = (1 << 10);   // Clear interrupt FIRST
        LPC_GPIO1->FIOPIN ^= (1 << 18);        // Toggle LED P1.18 using XOR
        gpio_irq_flag ^= 1;                    // Toggle software flag
    }
}
```

---

## 6. Hardware Testing Procedure

### Expected Output
Each press of the user button (P2.10) toggles the LED at P1.18. The main loop does nothing — all action is in the ISR.

### Init Flow Diagram
```
P2.10 = Input (FIODIR[10] = 0)
      │
IO2IntEnR[10] = 1 (rising edge enable)
      │
NVIC_EnableIRQ(EINT3_IRQn)
      │
CPU waits in while(1)
      │
Button released → P2.10 rising edge
      │
EINT3_IRQHandler fires
      │
Clear IO2IntClr → Toggle LED → return
```

### Debugging
- **Breakpoint in `EINT3_IRQHandler`:** confirms ISR fires on button press
- **Watch `gpio_irq_flag`:** toggles 0/1 with each press
- **Peripheral → GPIO Port 2:** observe FIOPIN bit 10 go HIGH when button released
