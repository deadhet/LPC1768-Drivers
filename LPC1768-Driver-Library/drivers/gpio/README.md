# GPIO Driver — LPC1768

## 1. Driver Overview

**GPIO (General Purpose Input/Output)** is the most fundamental peripheral of any microcontroller. It allows the LPC1768 to directly drive (write) or read logic signals on individual pins.

**Real-world applications:**
- Controlling LEDs, buzzers, relays, motors
- Reading push buttons, DIP switches
- Bit-banging communication protocols
- Driving digital logic circuits

This driver abstracts all four GPIO ports (PORT0–PORT3) behind a clean API with port number and pin bitmask parameters.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| LED1 | PORT1 | P1.18 | Output | Onboard LED — Active HIGH |

**Connection in test program:**
```
LPC1768 P1.18 → Onboard LED1 (through current limiting resistor on board)
```

**No external wiring needed** — LED is soldered on the trainer board.

---

## 3. Registers Used

### FIODIR — Fast I/O Direction Register

| Field | Bit Range | R/W | Description |
|-------|----------|-----|-------------|
| DIR | [31:0] | R/W | 1 = Output, 0 = Input (per-pin) |

**Usage in driver:** `LPC_GPIO1->FIODIR |= pins;` — sets specified pins as outputs.

### FIOSET — Fast I/O Set Register

| Field | Bit Range | R/W | Description |
|-------|----------|-----|-------------|
| SET | [31:0] | W | Write 1 → drive pin HIGH. Write 0 → no change. |

**Usage:** `LPC_GPIO1->FIOSET = pins;` — drives pins HIGH atomically.

### FIOCLR — Fast I/O Clear Register

| Field | Bit Range | R/W | Description |
|-------|----------|-----|-------------|
| CLR | [31:0] | W | Write 1 → drive pin LOW. Write 0 → no change. |

**Usage:** `LPC_GPIO1->FIOCLR = pins;` — drives pins LOW atomically.

### FIOPIN — Fast I/O Pin Register

| Field | Bit Range | R/W | Description |
|-------|----------|-----|-------------|
| PIN | [31:0] | R/W | Read: actual pin level. Write: set all output bits. |

**Usage:** `GPIO_Read()` returns `LPC_GPIO0->FIOPIN` etc.

---

## 4. Driver Architecture

```
Initialization Flow:
─────────────────────────────
GPIO_Init_Output(port, pins)
        │
        ▼
Select GPIO port struct (PORT0/1/2/3)
        │
        ▼
Set FIODIR bits (pins = OUTPUT)
        │
        ▼
Done — pin now drives output

Output Control Flow:
──────────────────────────────
GPIO_Set(port, pins)          GPIO_Clear(port, pins)
     │                               │
     ▼                               ▼
FIOSET = pins              FIOCLR = pins
(pins go HIGH)             (pins go LOW)
```

**Files:**

| File | Purpose |
|------|---------|
| `gpio.c` | Driver implementation |
| `gpio.h` | Function declarations, includes |
| `main.c` | LED blink test program |

---

## 5. Function Reference

### `GPIO_Init_Output(uint8_t port, uint32_t pins)`

| Item | Detail |
|------|--------|
| Description | Configure specified pins on a port as GPIO outputs |
| `port` | Port number: 0, 1, 2, or 3 |
| `pins` | Bitmask of pins to configure (e.g., `(1 << 18)` for P1.18) |
| Returns | void |
| Registers Modified | `LPC_GPIOx->FIODIR` |
| Flow | Select port → set FIODIR bits |

### `GPIO_Set(uint8_t port, uint32_t pins)`

| Item | Detail |
|------|--------|
| Description | Drive specified pins HIGH (logic 1) |
| `port` | Port number: 0–3 |
| `pins` | Bitmask of pins to set HIGH |
| Returns | void |
| Registers Modified | `LPC_GPIOx->FIOSET` |

### `GPIO_Clear(uint8_t port, uint32_t pins)`

| Item | Detail |
|------|--------|
| Description | Drive specified pins LOW (logic 0) |
| `port` | Port number: 0–3 |
| `pins` | Bitmask of pins to clear |
| Returns | void |
| Registers Modified | `LPC_GPIOx->FIOCLR` |

### `GPIO_Read(uint8_t port)`

| Item | Detail |
|------|--------|
| Description | Read the current state of all pins on a port |
| `port` | Port number: 0–3 |
| Returns | `uint32_t` — bitmask of all pin states |
| Registers Modified | None — read only (`LPC_GPIOx->FIOPIN`) |

---

## 6. Code Walkthrough

```c
void GPIO_Init_Output(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIODIR |= pins;  // PORT0: set pins as output
    if(port == 1) LPC_GPIO1->FIODIR |= pins;  // PORT1: set pins as output
    if(port == 2) LPC_GPIO2->FIODIR |= pins;  // PORT2: set pins as output
    if(port == 3) LPC_GPIO3->FIODIR |= pins;  // PORT3: set pins as output
}
// "|=" preserves other pin directions — only sets the specified bits.
// No PCONP needed: GPIO clock is always ON on LPC1768.

void GPIO_Set(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIOSET = pins;  // Atomic set HIGH
    // Writing to FIOSET: only pins with 1 in the bitmask go HIGH.
    // Pins with 0 in bitmask are NOT changed.
    if(port == 1) LPC_GPIO1->FIOSET = pins;
    // ... etc.
}

uint32_t GPIO_Read(uint8_t port)
{
    if(port == 0) return LPC_GPIO0->FIOPIN;  // Return all 32 bits of PORT0
    // Caller uses bit masking to check individual pins:
    //   uint32_t state = GPIO_Read(1);
    //   if(state & (1 << 18)) { /* P1.18 is HIGH */ }
    return 0;
}
```

---

## 7. Test Program (main.c)

The test program blinks **LED1 at P1.18** at approximately 1 Hz.

**Program flow:**
1. `SystemInit()` — set PLL, configure 100 MHz clock
2. `GPIO_Init_Output(1, LED_PIN)` — configure P1.18 as output
3. Infinite loop:
   - `GPIO_Set(1, LED_PIN)` → LED ON
   - `delay()` → software delay (~800ms)
   - `GPIO_Clear(1, LED_PIN)` → LED OFF
   - `delay()` → software delay (~800ms)

---

## 8. Hardware Testing Procedure

### Build Steps
1. Open `drivers/gpio/` folder in Keil — open or recreate project
2. Add: `gpio.c`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c`
3. Enable HEX output in **Project → Options → Output → Create HEX File**
4. Press **F7** → build should complete with 0 errors

### Flashing Steps
5. Put board in ISP mode: hold ISP button → press RESET → release ISP
6. Open Flash Magic: Device = LPC1768, COM port, Baud = 9600
7. Browse to `gpio.hex`
8. Click **Start** → wait for "Finished"
9. Press **RESET** on board

### Expected Output
- **LED at P1.18 blinks ON/OFF** at ~1 Hz
- ON for ~800ms, OFF for ~800ms

### Init Flow Diagram
```
SystemInit() — 100 MHz PLL
       │
       ▼
GPIO_Init_Output(1, P1.18) — FIODIR bit 18 = 1
       │
       ▼
Loop forever:
  GPIO_Set → LED ON → delay → GPIO_Clear → LED OFF → delay
```

### Debugging Guide
- **Peripheral view:** Open GPIO Port 1 → watch FIODIR bit 18 = 1 after init
- **Peripheral view:** Watch FIOPIN bit 18 toggle during loop
- **Breakpoint:** Set on `GPIO_Set(1, LED_PIN)` → step → observe FIOPIN change
- **Watch:** *(no user variables)* — use Peripheral → GPIO Port 1 directly
