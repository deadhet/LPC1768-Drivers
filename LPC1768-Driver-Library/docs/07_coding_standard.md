# Document 07 — Coding Standard

This document defines the coding conventions used across all drivers in this repository. Following these standards ensures consistency, readability, and maintainability — essential qualities in professional embedded firmware.

---

## 1. Naming Conventions

### Functions

Format: `PERIPHERAL_Action()` — all meaningful words capitalized, no underscores between peripheral and action.

```c
// ✅ Correct
UART0_Init(uint32_t baudrate);
GPIO_Set(uint8_t port, uint32_t pins);
ADC_Read(uint8_t channel);
I2C0_WriteByte(uint8_t addr, uint8_t data);
LCD_SetCursor(unsigned char row, unsigned char col);
PWM1_SetDuty(uint8_t duty_percent);
Timer0_Init(void);
RTC_GetTime(uint8_t *hr, uint8_t *min, uint8_t *sec);
SevenSeg_Display(uint16_t number);
Keypad_GetKey(KEYPAD_Handle_t *keypad);
OLED_DisplayString(char *str);

// ❌ Incorrect
uart_init();          // lowercase
initUART();           // wrong order
UART_init_function(); // redundant word
```

### Constants and Macros

Format: `ALL_CAPS_WITH_UNDERSCORES`

```c
// ✅ Correct
#define LED_PIN     (1 << 18)
#define I2C_SUCCESS 1
#define I2C_ERROR   0
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// ❌ Incorrect
#define ledpin 18
#define success 1
```

### Type Definitions

Format: `PascalCase_t` — descriptive name, `_t` suffix

```c
// ✅ Correct
typedef struct {
    LPC_GPIO_TypeDef *rowPort;
    LPC_GPIO_TypeDef *colPort;
    uint8_t rowPins[4];
    uint8_t colPins[4];
} KEYPAD_Handle_t;

// ❌ Incorrect
typedef struct { ... } keypad;
typedef struct { ... } KEYPAD;
```

### Variables

Format: `camelCase` for local variables, `snake_case` for global flags

```c
// ✅ Correct (local)
uint32_t pclk, divisor;
uint16_t adcValue;
uint8_t slaveAddr;

// ✅ Correct (global ISR flag)
volatile uint32_t timer_flag = 0;
volatile uint32_t gpio_irq_flag = 0;
```

---

## 2. File Organization

### File Naming

| File Type | Convention | Example |
|-----------|-----------|---------|
| Driver source | `lowercase.c` | `uart.c`, `gpio.c` |
| Driver header | `lowercase.h` | `uart.h`, `gpio.h` |
| Test program | `main.c` | `main.c` |
| Font data | `driver_font.c/h` | `oled_font.c` |

### Header File Guard

Every `.h` file must use include guards:

```c
// ✅ Correct pattern
#ifndef __UART_H__
#define __UART_H__

// ... declarations ...

#endif  // __UART_H__
```

### Inclusion Order in `.c` files

```c
#include "LPC17xx.h"       // 1. CMSIS device header (always first)
#include "driver.h"         // 2. This driver's own header
#include "dependency.h"     // 3. Any dependency driver headers
#include <stdint.h>         // 4. Standard library headers last
#include <stdio.h>
```

---

## 3. Register Access Style

### Mandatory Pattern: Clear Before Set

When configuring a multi-bit field, **always clear first, then set**:

```c
// ✅ Correct — atomic, safe
LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));  // Clear bits [5:4] and [7:6]
LPC_PINCON->PINSEL0 |=  ((1 << 4) | (1 << 6));  // Set to function 01

// ❌ Incorrect — may corrupt adjacent bits
LPC_PINCON->PINSEL0 = 0x50;  // Magic number — unreadable
```

### Use Bit Shifts, Not Magic Numbers

```c
// ✅ Correct — self-documenting
LPC_SC->PCONP |= (1 << 3);         // Enable UART0 power (bit 3)
LPC_TIM0->MCR = (1 << 0) | (1 << 1);  // MR0 interrupt + reset

// ❌ Incorrect — obscure
LPC_SC->PCONP |= 0x8;
LPC_TIM0->MCR = 0x3;
```

### Atomic GPIO Operations

Always use `FIOSET`/`FIOCLR` instead of `FIOPIN` read-modify-write for outputs:

```c
// ✅ Correct — atomic, interrupt-safe
LPC_GPIO1->FIOSET = (1 << 18);   // Set P1.18 HIGH
LPC_GPIO1->FIOCLR = (1 << 18);   // Set P1.18 LOW

// ❌ Incorrect — read-modify-write, not interrupt-safe
LPC_GPIO1->FIOPIN |= (1 << 18);
LPC_GPIO1->FIOPIN &= ~(1 << 18);
```

---

## 4. Comment Style

### Function Header Comments

```c
/*
 * UART0_Init() — Initialize UART0 with the given baud rate.
 *
 * Parameters:
 *   baudrate — desired baud rate in bits per second (e.g., 9600)
 *
 * Registers modified:
 *   LPC_SC->PCONP, LPC_PINCON->PINSEL0
 *   LPC_UART0->LCR, LPC_UART0->DLL, LPC_UART0->DLM, LPC_UART0->FCR
 */
void UART0_Init(uint32_t baudrate)
{
    // ...
}
```

### Inline Comments

```c
LPC_SC->PCONP |= (1 << 3);      /* Enable UART0 power supply */
LPC_UART0->LCR = (1 << 7);      /* DLAB = 1: access divisor registers */
LPC_UART0->DLL = divisor & 0xFF; /* Lower 8 bits of baud divisor */
```

---

## 5. ISR (Interrupt Service Routine) Rules

```c
// ✅ Correct ISR structure
void TIMER0_IRQHandler(void)
{
    if (LPC_TIM0->IR & 1)          // Check correct interrupt source
    {
        LPC_TIM0->IR = 1;           // Clear interrupt FIRST
        timer_flag ^= 1;            // Do minimal work in ISR
    }
}
```

Rules:
1. **Check the source flag** first (avoid false triggers)
2. **Clear the interrupt** immediately after checking
3. **Minimize ISR duration** — set a flag, do work in main loop
4. **Declare ISR-shared variables as `volatile`**

---

## 6. Delay Functions

```c
// ✅ Correct — compiler won't optimize away
void delay(void)
{
    volatile int i;
    for(i = 0; i < 500000; i++);
}

// ❌ Incorrect — compiler may eliminate
void delay(void)
{
    int i;
    for(i = 0; i < 500000; i++);
}
```

Always use `volatile` for loop counter in software delays.

---

## 7. Summary Checklist

- [ ] Function names follow `PERIPHERAL_Action()` format
- [ ] Constants are `ALL_CAPS`
- [ ] Header files have include guards
- [ ] Clear-before-set pattern used for multi-bit fields
- [ ] Bit shifts used (no magic numbers)
- [ ] `FIOSET`/`FIOCLR` used for GPIO output (not FIOPIN)
- [ ] ISR clears interrupt flag immediately
- [ ] ISR-shared variables declared `volatile`
- [ ] Software delay counters declared `volatile`
