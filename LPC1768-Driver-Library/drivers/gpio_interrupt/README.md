# GPIO Interrupt Driver — LPC1768

## 1. Driver Overview

This driver demonstrates **interrupt-driven GPIO input** using the LPC1768's external interrupt system. Instead of continuously polling a button pin in a loop (which wastes CPU time and may miss fast events), the hardware automatically notifies the CPU via an interrupt the moment a pin changes state. The CPU can then respond immediately in the Interrupt Service Routine (ISR).

On the LPC1768, GPIO interrupts on PORT0 and PORT2 are routed through the **EINT3** interrupt vector. This means the ISR function must be named `EINT3_IRQHandler` for the startup code to correctly link it to the interrupt vector table.

**Real-world applications:**
- Emergency stop button handling
- Door or tamper detection sensors
- Encoder event counting (rotary encoders)
- Wake-from-sleep triggers in low-power applications

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| User Button | PORT2 | P2.10 | Input | Interrupt source |
| LED1 | PORT1 | P1.18 | Output | Toggled by ISR |

**Button behavior on trainer board:** The user button at P2.10 pulls the pin LOW when pressed. When released, the pin rises back to HIGH due to the pull-up resistor. This driver configures a **rising edge** interrupt, which fires when the button is released (LOW → HIGH transition).

## 3. Registers Used

### LPC_GPIO2→FIODIR — Direction Register

```c
LPC_GPIO2->FIODIR &= ~(1 << 10);
```

Clearing bit 10 of FIODIR makes P2.10 an input. `~(1 << 10)` is the bitwise complement of `0x00000400`, which is `0xFFFFFBFF`. The `&=` clears only bit 10 while leaving all other direction bits unchanged.

### LPC_GPIOINT→IO2IntEnR — Port 2 Rising Edge Interrupt Enable

The `LPC_GPIOINT` peripheral block controls interrupt configuration for GPIO ports 0 and 2. The `IO2IntEnR` register enables rising edge interrupt detection per pin for PORT2.

```c
LPC_GPIOINT->IO2IntEnR |= (1 << 10);
```

Setting bit 10 in `IO2IntEnR` arms the rising edge interrupt detector for P2.10. When this pin transitions from LOW to HIGH, the hardware sets a pending interrupt flag and signals the NVIC.

### NVIC_EnableIRQ(EINT3_IRQn)

The NVIC is the Cortex-M3 interrupt controller. Even when the GPIO interrupt is enabled and triggered, the NVIC must also allow it through. `NVIC_EnableIRQ(EINT3_IRQn)` enables the EINT3 interrupt in the NVIC interrupt enable register, connecting the GPIO hardware event to the CPU interrupt mechanism.

PORT0 and PORT2 GPIO interrupts share the EINT3 vector. Within the ISR, the code must check status registers to determine which port and which pin caused the interrupt.

### LPC_GPIOINT→IO2IntStatR — Port 2 Rising Edge Status

This read-only register shows which PORT2 pin(s) triggered a rising edge interrupt.

```c
if (LPC_GPIOINT->IO2IntStatR & (1 << 10))
```

Checking bit 10 confirms that P2.10 was the source of the interrupt, not some other pin that may also be configured with interrupts.

### LPC_GPIOINT→IO2IntClr — Port 2 Interrupt Clear

```c
LPC_GPIOINT->IO2IntClr = (1 << 10);
```

Writing 1 to bit 10 of `IO2IntClr` clears the interrupt pending flag for P2.10. This is mandatory inside the ISR. If the flag is not cleared, the NVIC will immediately re-enter the ISR after it returns, causing an infinite interrupt loop.

## 4. Driver Architecture

```
GPIO2_Interrupt_Init()
  ├─ FIODIR P2.10 = Input (clear bit 10)
  ├─ IO2IntEnR[10] = 1 (rising edge interrupt enable)
  └─ NVIC_EnableIRQ(EINT3_IRQn)

CPU runs main() (or sleeps):
  while(1) { /* wait for interrupt */ }

User button released → P2.10 rising edge:
  └─ EINT3_IRQHandler() called by NVIC
         │
         ├─ Check IO2IntStatR[10] = 1 (confirm P2.10 source)
         ├─ IO2IntClr = (1<<10)    (MUST clear before any action)
         ├─ FIOPIN ^= (1<<18)      (toggle LED P1.18)
         └─ gpio_irq_flag ^= 1     (toggle software flag)
```

## 5. Function Reference

### `GPIO2_Interrupt_Init(void)`
Configures P2.10 as input, enables rising edge interrupt, enables NVIC.

### `EINT3_IRQHandler(void)` (ISR)
Called automatically by NVIC on P2.10 rising edge. Clears interrupt, toggles LED.

## 6. Code Walkthrough

### Setting Pin Direction

```c
LPC_GPIO2->FIODIR &= ~(1 << 10);
```

`(1 << 10)` is binary 100 0000 0000 (bit 10 = 1, all others = 0). The `~` inverts it: 1111 1011 1111 1111 (bit 10 = 0, all others = 1). The `&=` clears only bit 10 in FIODIR, making P2.10 an input. After this, the pin floats HIGH via the board's pull-up resistor.

### Enabling Rising Edge Interrupt

```c
LPC_GPIOINT->IO2IntEnR |= (1 << 10);
```

The GPIOINT peripheral has separate registers for PORT0 and PORT2, and separate registers for rising and falling edges. This line enables rising edge detection on P2.10. There is a corresponding `IO2IntEnF` register for falling edge detection, which is not used here.

Rising edge detection means the interrupt fires on the LOW-to-HIGH transition. Since the button pulls LOW when pressed, the interrupt fires when the button is released — this is a common design choice to avoid triggering during the press-hold period.

### Enabling the NVIC

```c
NVIC_EnableIRQ(EINT3_IRQn);
```

This CMSIS function sets bit 17 in the NVIC Interrupt Set-Enable Register (ISER0), enabling the EINT3 interrupt vector. `EINT3_IRQn` is a C enum value (= 21) that identifies the EINT3 interrupt number in the NVIC.

After this call, when P2.10 rises, the hardware:
1. Sets IO2IntStatR bit 10
2. Asserts the EINT3 interrupt to the NVIC
3. NVIC suspends the current code and calls EINT3_IRQHandler

### The ISR — Interrupt Service Routine

```c
void EINT3_IRQHandler(void)
{
    if (LPC_GPIOINT->IO2IntStatR & (1 << 10))
    {
        LPC_GPIOINT->IO2IntClr = (1 << 10);  // Clear interrupt FIRST

        LPC_GPIO1->FIOPIN ^= (1 << 18);       // Toggle LED P1.18
        gpio_irq_flag ^= 1;                   // Toggle flag
    }
}
```

The ISR begins by checking IO2IntStatR to confirm P2.10 caused the interrupt. This check is important because PORT0 and PORT2 share EINT3 — if a PORT0 pin also had an interrupt enabled and fired simultaneously, both would need to be handled.

The interrupt is cleared immediately after confirmation, before any processing. Writing 1 to IO2IntClr bit 10 clears the pending flag in the GPIOINT hardware. The NVIC automatically runs only one interrupt handler per event, but if the flag is not cleared, re-entry will occur after return.

`LPC_GPIO1->FIOPIN ^= (1 << 18)` uses XOR to toggle bit 18. If bit 18 was 1 (LED on), XOR with 1 makes it 0 (LED off). If it was 0, XOR makes it 1. The `^=` is a read-modify-write operation on FIOPIN — since the FIOPIN register reflects actual pin states, reading it gives the current output value, XOR flips the target bit, and writing it back updates the output.

`gpio_irq_flag ^= 1` toggles a software variable. Main code can read this flag to know how many button presses occurred.

## 7. Test Program Explanation

Main.c sets P1.18 as an output and enters an empty infinite loop. All functionality is in the ISR. Each button press generates one interrupt, which toggles the LED. The main loop serves only as a "background" context for the CPU to be in while waiting.

## 8. Hardware Testing Procedure

### Expected Output
Each press of the user button at P2.10 toggles LED P1.18. Main loop does nothing — all action is in the ISR.

### Init Flow Summary
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
Clear IO2IntClr → Toggle LED → Return
```

### Debugging in Keil
- **Breakpoint in `EINT3_IRQHandler`:** confirms ISR fires on button release
- **Watch `gpio_irq_flag`:** toggles between 0 and 1 with each button press
- **Peripheral → GPIO Port 2:** watch FIOPIN bit 10 transition from 0→1 on button release
- **If ISR not firing:** verify `NVIC_EnableIRQ(EINT3_IRQn)` was called and IO2IntEnR bit 10 is set
