# GPIO Driver: LPC1768

## 1. Driver Overview

**GPIO (General Purpose Input/Output)** is the most fundamental peripheral of any microcontroller. It allows the LPC1768 to directly drive (write) or read logic signals on individual pins. Unlike peripherals such as UART or SPI which have their own dedicated hardware, GPIO pins are directly controlled by software at the register level.

The LPC1768 implements a "Fast GPIO" architecture. Unlike the older LPC21xx-series GPIO which required multiple clock cycles to change, the fast GPIO registers are memory-mapped directly into the AHB bus, allowing single-cycle read and write operations.

**Real-world applications:**
- Controlling LEDs, buzzers, relays, and relay-driven motors
- Reading push buttons, DIP switches, and tactile sensors
- Bit-banging communication protocols where hardware UART/SPI is not available
- Driving digital logic signals to external ICs

This driver abstracts all four GPIO ports (PORT0–PORT3) behind a clean API using port number and pin bitmask parameters.

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| LED1 | PORT1 | P1.18 | Output | Onboard LED — Active HIGH |

**Connection in test program:**
```
LPC1768 P1.18 → Onboard LED1 (through current limiting resistor on board)
```

No external wiring needed LED is soldered on the trainer board. P1.18 drives the LED anode through a series resistor; asserting HIGH lights the LED.

## 3. Registers Used

### FIODIR: Fast I/O Direction Register

Each GPIO port has its own FIODIR register (LPC_GPIO0->FIODIR, LPC_GPIO1->FIODIR, etc.). Each bit in FIODIR corresponds to one pin on that port:

- Bit = 0: Pin is an input (high-impedance, reads external signal)
- Bit = 1: Pin is an output (driven by the MCU)

FIODIR is 32 bits wide. The LPC1768 has ports with varying numbers of implemented pins, but the register is always 32 bits. Unused bits simply have no hardware effect.

**Key property:** The GPIO peripheral clock is always active on the LPC1768: no PCONP bit needs to be set. GPIO is always available.

### FIOSET: Fast I/O Set Register

Writing a 1 to a bit in FIOSET drives the corresponding output pin HIGH. Writing a 0 to a bit has no effect it does not pull the pin LOW. This means you can set multiple pins HIGH simultaneously without affecting other pins.

This is an important safety property. If you wrote directly to FIOPIN to set pins HIGH, you would have to read the current state, OR in your bits, then write back (read-modify-write). FIOSET and FIOCLR eliminate this requirement for atomic pin control.

### FIOCLR: Fast I/O Clear Register

Writing a 1 to a bit in FIOCLR drives the corresponding output pin LOW. Writing a 0 has no effect. Like FIOSET, this allows atomic clearing of specific pins without affecting others.

### FIOPIN: Fast I/O Pin Register

This register reflects the actual current state of the port pins. Reading it returns the current logic level on all 32 pins, regardless of whether they are configured as inputs or outputs. Writing to FIOPIN sets the output value of all output pins simultaneously (inputs are unaffected), but this is rarely used: FIOSET and FIOCLR are preferred for safety.

## 4. Driver Architecture

```
Initialization Flow:
GPIO_Init_Output(port, pins)
        │
Select GPIO port struct (PORT0/1/2/3)
        │
Set FIODIR bits → pins become outputs
        │
Done — pin now controlled by MCU

Output Control:
GPIO_Set(port, pins)    GPIO_Clear(port, pins)
     │                               │
FIOSET = pins                 FIOCLR = pins
(pins go HIGH)                (pins go LOW)

Reading Pins:
GPIO_Read(port)
     │
Return FIOPIN register value
```

**Files:**

| File | Purpose |
|------|---------|
| `gpio.c` | Driver implementation |
| `gpio.h` | Function declarations, includes |
| `main.c` | LED blink test program |

## 5. Function Reference

### `GPIO_Init_Output(uint8_t port, uint32_t pins)`

| Item | Detail |
|------|--------|
| Description | Configure specified pins on a port as GPIO outputs |
| `port` | Port number: 0, 1, 2, or 3 |
| `pins` | Bitmask of pins to configure (e.g., `(1 << 18)` for P1.18) |
| Returns | void |
| Registers Modified | `LPC_GPIOx->FIODIR` |

### `GPIO_Set(uint8_t port, uint32_t pins)`

Drives specified pins HIGH. Uses FIOSET for atomic operation.

### `GPIO_Clear(uint8_t port, uint32_t pins)`

Drives specified pins LOW. Uses FIOCLR for atomic operation.

### `GPIO_Read(uint8_t port)`

Returns `uint32_t`: the full 32-bit pin state of the port. Use bit masking to extract individual pins.

## 6. Code Walkthrough

### Setting Pin Direction as Output

```c
void GPIO_Init_Output(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIODIR |= pins;
    if(port == 1) LPC_GPIO1->FIODIR |= pins;
    if(port == 2) LPC_GPIO2->FIODIR |= pins;
    if(port == 3) LPC_GPIO3->FIODIR |= pins;
}
```

The function uses `|=` (bitwise OR assignment) to set only the specified bits without changing others. For example, when the test program calls `GPIO_Init_Output(1, (1 << 18))`:

The value `(1 << 18)` is binary:
```
Bit:    31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 ...
Value:   0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0 ...
```

The `|=` operation sets bit 18 of LPC_GPIO1->FIODIR to 1, making P1.18 an output. All other pins on PORT1 retain their previous direction settings. Without `|=` (if plain `=` were used instead), all other pins on the port would be set to inputs, potentially breaking other parts of the system.

No PCONP step is needed here because GPIO clock is always on.

### Driving a Pin HIGH

```c
void GPIO_Set(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIOSET = pins;
    if(port == 1) LPC_GPIO1->FIOSET = pins;
    ...
}
```

Writing `(1 << 18)` to LPC_GPIO1->FIOSET causes only bit 18 of the port to go HIGH. The FIOSET register is write-only in effect bits written as 0 are ignored. This means the operation is inherently atomic: no read-then-modify step is needed.

When FIOSET sets a pin HIGH on an output pin, the internal driver circuit pulls that pin up to the supply voltage (3.3V on the LPC1768). For LED1 at P1.18, this forward-biases the LED through its series resistor, causing it to emit light.

### Driving a Pin LOW

```c
void GPIO_Clear(uint8_t port, uint32_t pins)
{
    if(port == 0) LPC_GPIO0->FIOCLR = pins;
    ...
}
```

Writing to FIOCLR clears the specified bits. This drives the pin to 0V (GND). Bits written as 0 in the mask have no effect. The hardware results in the LED turning off since the voltage difference across it drops to near zero.

### Reading Pin State

```c
uint32_t GPIO_Read(uint8_t port)
{
    if(port == 0) return LPC_GPIO0->FIOPIN;
    if(port == 1) return LPC_GPIO1->FIOPIN;
    if(port == 2) return LPC_GPIO2->FIOPIN;
    if(port == 3) return LPC_GPIO3->FIOPIN;
    return 0;
}
```

FIOPIN returns all 32 pin states simultaneously. The caller uses masking to extract specific pins:

```c
uint32_t state = GPIO_Read(1);
if (state & (1 << 18))
{
    // P1.18 is currently HIGH
}
```

### The LED Blink Loop

```c
#define LED_PIN (1 << 18)

GPIO_Init_Output(1, LED_PIN);

while(1)
{
    GPIO_Set(1, LED_PIN);    // LED ON
    delay();
    GPIO_Clear(1, LED_PIN);  // LED OFF
    delay();
}
```

`(1 << 18)` evaluates to the integer value 262144 (0x00040000). This constant is used consistently as the bitmask for pin 18. The `delay()` function runs a software loop for approximately 800ms to create a ~1 Hz blink rate.

## 7. Test Program Explanation

The program:
1. Calls `SystemInit()` to configure the PLL for 100 MHz operation
2. Sets P1.18 as output via `GPIO_Init_Output(1, LED_PIN)`
3. Enters an infinite loop alternately calling `GPIO_Set` and `GPIO_Clear` with `delay()` between them

The LED visibly blinks at approximately 1 Hz. This test confirms the GPIO driver can both configure pin direction and control output state correctly.

## 8. Hardware Testing Procedure

### Build Steps
1. Open Keil and create a new project targeting LPC1768
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
- LED at P1.18 blinks ON/OFF at ~1 Hz
- ON for ~800ms, OFF for ~800ms

### Debugging in Keil
- **Peripheral view → GPIO Port 1:** watch FIODIR bit 18 = 1 after `GPIO_Init_Output()`
- **Peripheral view:** watch FIOPIN bit 18 toggle during the loop
- **Breakpoint on `GPIO_Set(1, LED_PIN)`:** step through and observe FIOPIN bit 18 change
