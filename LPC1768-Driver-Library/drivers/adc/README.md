# ADC Driver: LPC1768

## 1. Driver Overview

**ADC (Analog-to-Digital Converter)** converts a continuous analog voltage into a discrete digital number. The LPC1768 contains a single 12-bit ADC with 8 multiplexed input channels (AD0.0 through AD0.7). "12-bit" means the converter produces values in the range 0 to 4095 (2^12 = 4096 steps).

This is essential because microcontrollers operate entirely in the digital domain. Without an ADC, it is impossible to read real-world signals like temperature sensor outputs, potentiometer positions, or battery voltage levels.

**How it works internally:** A successive approximation ADC compares the input voltage against a reference using a precision internal comparator. It tries each bit from the most significant to the least significant, building up the result one bit at a time. The LPC1768 ADC takes approximately 65 ADC clock cycles to complete a conversion.

**Real-world applications:**
- Reading sensor outputs (temperature, humidity, light intensity)
- Potentiometer position sensing for volume or brightness control
- Battery voltage monitoring in portable devices
- Audio input sampling

This driver reads from **ADC Channel 1** connected to the onboard 10K potentiometer at **P0.24**.

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| AD0.1 (Potentiometer) | PORT0 | P0.24 | Analog Input | 0–3.3V from onboard potentiometer |
| UART0 TX (debug) | PORT0 | P0.2 | Output | Prints ADC value to serial terminal |

**Connection:**
```
Potentiometer wiper ──→ P0.24 (AD0.1)
(Onboard - no external wiring needed on trainer kit)
```

**Output range:** 0 (0V input) to 4095 (3.3V input)

**Voltage formula:**
```
Voltage = (ADC_Value / 4095) × 3.3V
```

## 3. Registers Used

### LPC_SC→PCONP: Peripheral Power Control

| Bit | Value | Description |
|-----|-------|-------------|
| 12 | 1 | Enable ADC0 power and clock |

Without enabling bit 12 in PCONP, the ADC hardware has no clock and all register writes are ignored.

### LPC_SC→PCLKSEL0: Peripheral Clock Select

| Bits | Value | PCLK divisor | Resulting PCLK |
|------|-------|-------------|----------------|
| [25:24] | 00 | CCLK/4 | 25 MHz (default) |
| [25:24] | 01 | CCLK/1 | 100 MHz |
| [25:24] | 10 | CCLK/2 | 50 MHz |
| [25:24] | 11 | CCLK/8 | 12.5 MHz |

This driver clears bits [25:24] to 00, giving ADC a **25 MHz** input clock.

### LPC_PINCON→PINSEL1: Pin Select Register

PINSEL1 controls P0.16 through P0.31. Each pin uses 2 bits. P0.24 occupies bits [17:16]:

| Bits | Pin | Value | Function |
|------|-----|-------|---------|
| [17:16] | P0.24 | 01 | AD0.1 (analog input mode) |

Setting these bits to 01 connects pin P0.24 to the ADC analog input mux. Importantly, this also disables the digital input buffer on that pin, the pin is now purely analog and will not interfere with the ADC measurement.

### LPC_ADC→ADCR: ADC Control Register

This is the primary control register for the ADC. It configures the channel, clock, power state, and conversion triggers.

| Bits | Field | Value Used | Description |
|------|-------|-----------|-------------|
| [7:0] | SEL | `(1 << ch)` | Channel select — set bit N to select channel N |
| [15:8] | CLKDIV | 4 | ADC clock = PCLK / (CLKDIV+1) = 25MHz / 5 = 5 MHz |
| [21] | PDN | 1 | 0=power down, 1=operational mode |
| [26:24] | START | 001 | Start conversion immediately |

The CLKDIV value controls the ADC clock. The LPC1768 ADC requires a clock ≤ 13 MHz. With PCLK=25 MHz and CLKDIV=4, the ADC clock is 25/5 = 5 MHz, which is safely within spec.

### LPC_ADC→ADGDR: ADC Global Data Register

After conversion completes, the result is available in ADGDR:

| Bits | Field | Description |
|------|-------|-------------|
| [3:0] | — | Reserved |
| [15:4] | RESULT | 12-bit conversion result (0–4095) |
| [26:24] | CHN | Channel number that produced this result |
| [31] | DONE | 1 = conversion complete |

The result is in bits [15:4] which is shifted left by 4 bits. To extract just the 12-bit result, shift right by 4 and mask with 0xFFF.

## 4. Driver Architecture

```
Initialization:
ADC_Init()
  ├─ Enable ADC power (PCONP bit 12)
  ├─ Set PCLKSEL0: ADC clock = CCLK/4 = 25 MHz
  ├─ Configure P0.24 as AD0.1 (PINSEL1[17:16]=01)
  └─ Set ADCR: channel 1 selected, divider=4, PDN=1 (power on)

Conversion:
ADC_Read(channel)
  ├─ Clear SEL bits [7:0] in ADCR
  ├─ Set SEL bit for requested channel
  ├─ Set START[26:24]=001 (start conversion now)
  ├─ Poll ADGDR bit 31 (DONE) until = 1
  ├─ Read ADGDR
  ├─ Clear START bits
  └─ Return ADGDR[15:4] >> 4 (12-bit result)
```

## 5. Function Reference

### `ADC_Init(void)`
Powers on ADC, configures P0.24 as analog input, enables ADC at 5 MHz clock.

### `ADC_Read(uint8_t channel)`
Triggers a single conversion on the given channel and returns the 12-bit result.

| Parameter | Type | Description |
|-----------|------|-------------|
| `channel` | uint8_t | ADC channel number (0–7) |
| Returns | uint16_t | 12-bit result: 0 (0V) to 4095 (3.3V) |

## 6. Code Walkthrough

### Enabling ADC Power

```c
LPC_SC->PCONP |= (1 << 12);
```

Bit 12 of the PCONP register is the PCADC bit. Setting it to 1 enables the clock to the ADC block. The `|=` preserves all other peripheral power settings. This must be done before any ADC register access.

### Configuring the ADC Clock

```c
LPC_SC->PCLKSEL0 &= ~(3 << 24);
```

PCLKSEL0 bits [25:24] control the ADC peripheral clock divider. `(3 << 24)` creates a mask with bits 25 and 24 set. The `&= ~` clears both bits to 00, selecting PCLK = CCLK/4 = 25 MHz. This is the default setting but is explicitly set here for clarity.

### Configuring P0.24 as Analog Input

```c
LPC_PINCON->PINSEL1 &= ~(3 << 16);
LPC_PINCON->PINSEL1 |=  (1 << 16);
```

P0.24 sits in PINSEL1 at bits [17:16]. The first line clears those bits to 00. The second line sets bit 16, resulting in `01` at bits [17:16], which connects P0.24 to the AD0.1 analog input. This disconnects the digital GPIO circuitry from the pin so it doesn't interfere with the sensitive analog measurement.

### Configuring the ADC Control Register

```c
LPC_ADC->ADCR = (1 << 1)       // Select channel AD0.1
              | (4 << 8)        // CLKDIV = 4 → ADC clock = 5 MHz
              | (1 << 21);      // PDN = 1: power ON, operational
```

Bit 1 of the SEL field (bits [7:0]) selects channel AD0.1. Each bit in the SEL field enables one channel. Since we're enabling bit 1, channel 1 (AD0.1, connected to P0.24) is selected.

`(4 << 8)` places the value 4 into bits [15:8] (the CLKDIV field). ADC clock = PCLK / (CLKDIV + 1) = 25 MHz / 5 = 5 MHz.

Bit 21 is PDN (Power Down). Setting it to 1 brings the ADC out of power-down mode and into operational mode. Without this, the ADC will not perform any conversions.

### Starting a Conversion and Reading Result

```c
LPC_ADC->ADCR &= ~(0xFF);           // Clear all channel select bits
LPC_ADC->ADCR |= (1 << channel);    // Select the requested channel

LPC_ADC->ADCR |= (1 << 24);         // START[26:24] = 001: start now

while (!(LPC_ADC->ADGDR & (1UL << 31)));  // Poll DONE bit

result = LPC_ADC->ADGDR;            // Read entire ADGDR register

LPC_ADC->ADCR &= ~(7 << 24);        // Clear START bits

return (result >> 4) & 0xFFF;       // Extract 12-bit result from bits [15:4]
```

First, all 8 channel select bits are cleared, then only the requested channel bit is set. This allows the function to be called with any channel number 0–7.

Setting bit 24 of ADCR (START field = 001) triggers an immediate conversion. The hardware begins sampling the selected analog channel.

The DONE bit (bit 31 of ADGDR) is monitored in a polling loop. The `1UL << 31` uses `unsigned long` to ensure the shift doesn't overflow (regular `int` is 32 bits and bit 31 would be the sign bit).

Once DONE=1, the result register is read. The result occupies bits [15:4], so shifting right by 4 moves it into bits [11:0]. Masking with 0xFFF (binary: 000011111111111) ensures only 12 bits are kept, discarding any upper bits that might be set from the channel number field.

## 7. Test Program Explanation

The main.c continuously reads Channel 1 and sends the value via UART:

1. `SystemInit()` — 100 MHz clock
2. `UART0_Init(9600)` — initialize serial output
3. `ADC_Init()` — power on ADC, configure P0.24
4. Loop: `ADC_Read(1)` → print via `UART0_Printf()`

As the onboard potentiometer is rotated, the voltage on P0.24 changes from 0V to 3.3V, and the printed value changes from 0 to 4095.

## 8. Hardware Testing Procedure

### Build & Flash
1. Add: `adc.c`, `uart.c`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c`
2. **F7** → 0 errors → flash `adc.hex` via Flash Magic
3. Press RESET

### Expected Output
Open PuTTY (9600 baud, 8N1):
```
ADC Value: 0
ADC Value: 512
ADC Value: 2047
ADC Value: 4095
...
```
Rotating the onboard potentiometer changes the displayed value.

### Init Flow Summary
```
PCONP bit 12 = 1 (ADC ON)
      │
PCLKSEL0[25:24] = 00 (PCLK/4 = 25 MHz)
      │
PINSEL1[17:16] = 01 (P0.24 = AD0.1 analog)
      │
ADCR: SEL=ch1, CLKDIV=4, PDN=1
      │
ADC Ready — call ADC_Read() to convert
```

### Debugging in Keil
- **Peripheral view → ADC:** after `ADC_Init()`, verify ADCR PDN bit = 1
- **Watch `adc_value`:** observe values update every loop iteration
- **Memory: 0x40034004 (ADGDR):** bit 31 goes HIGH when conversion is done, then the 12-bit result is in bits [15:4]
