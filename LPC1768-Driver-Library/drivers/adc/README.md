# ADC Driver — LPC1768

## 1. Driver Overview

**ADC (Analog-to-Digital Converter)** converts a continuous analog voltage into a discrete digital number. The LPC1768's built-in 12-bit ADC lets you measure real-world signals such as temperature, pressure, light, or potentiometer position.

**Real-world applications:**
- Reading sensor outputs (temperature, humidity, pressure)
- Potentiometer position sensing
- Battery voltage monitoring
- Audio input level metering

This driver reads from **ADC Channel 1** connected to the onboard potentiometer at **P0.24**.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| AD0.1 (Potentiometer) | PORT0 | P0.24 | Analog Input | 0–3.3V from onboard potentiometer |
| UART0 TX (debug) | PORT0 | P0.2 | Output | Prints ADC value to serial terminal |

**Connection:**
```
Potentiometer wiper ──→ P0.24 (AD0.1)
(Onboard — no external wiring needed on trainer kit)
```

**Output range:** 0 (0V input) to 4095 (3.3V input)

---

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power
| Bit | Value | Description |
|-----|-------|-------------|
| 12 | 1 | Enable ADC0 power and clock |

### LPC_SC→PCLKSEL0 — Peripheral Clock Select
| Bits | Value | Description |
|------|-------|-------------|
| [25:24] | 00 | ADC PCLK = CCLK/4 = 25 MHz (default) |

### LPC_PINCON→PINSEL1 — Pin Select
| Bits | Pin | Value | Function |
|------|-----|-------|---------|
| [17:16] | P0.24 | 01 | AD0.1 (analog input mode) |

### LPC_ADC→ADCR — ADC Control Register
| Bits | Field | Value Used | Description |
|------|-------|-----------|-------------|
| [7:0] | SEL | `(1 << ch)` | Select channel (bit N = channel N) |
| [15:8] | CLKDIV | 4 | ADC clock = PCLK / (4+1) = 5 MHz |
| [21] | PDN | 1 | 0=power down, 1=operational |
| [26:24] | START | 001 | Start conversion now |

### LPC_ADC→ADGDR — ADC Global Data Register
| Bits | Field | Description |
|------|-------|-------------|
| [15:4] | RESULT | 12-bit conversion result (0–4095) |
| [26:24] | CHN | Channel number that produced this result |
| [31] | DONE | 1 = conversion complete |

---

## 4. Driver Architecture

```
Initialization:
─────────────────────────────────────────────────────
ADC_Init()
  ├─ Enable ADC power (PCONP bit 12)
  ├─ Set PCLKSEL0: ADC clock = CCLK/4
  ├─ Configure P0.24 as AD0.1 (PINSEL1[17:16]=01)
  └─ Set ADCR: channel 1 selected, divider=4, PDN=1 (enable)

Conversion:
─────────────────────────────────────────────────────
ADC_Read(channel)
  ├─ Clear SEL bits in ADCR
  ├─ Set SEL bit for requested channel
  ├─ Set START=001 (start conversion)
  ├─ Poll ADGDR bit 31 (DONE) until = 1
  ├─ Read ADGDR[15:4] = 12-bit result
  ├─ Clear START bits
  └─ Return result
```

---

## 5. Function Reference

### `ADC_Init(void)`
Powers on ADC, configures P0.24 as analog input, enables ADC at 5 MHz clock.

### `ADC_Read(uint8_t channel)`
Triggers a single conversion on the given channel and returns the 12-bit result.

| Parameter | Type | Description |
|-----------|------|-------------|
| `channel` | uint8_t | ADC channel number (0–7) |
| Returns | uint16_t | 12-bit result: 0 (0V) to 4095 (3.3V) |

---

## 6. Code Walkthrough

```c
void ADC_Init(void)
{
    LPC_SC->PCONP |= (1 << 12);         // Enable ADC clock (bit 12)
    LPC_SC->PCLKSEL0 &= ~(3 << 24);     // ADC PCLK = CCLK/4 (clear bits = 00)

    LPC_PINCON->PINSEL1 &= ~(3 << 16);  // Clear P0.24 function bits
    LPC_PINCON->PINSEL1 |=  (1 << 16);  // Set to 01 = AD0.1 analog input

    LPC_ADC->ADCR = (1 << 1)            // Select channel AD0.1 (bit 1)
                  | (4 << 8)            // CLKDIV = 4 → ADC clock = 5 MHz
                  | (1 << 21);          // PDN = 1: power ON and operational
}

uint16_t ADC_Read(uint8_t channel)
{
    LPC_ADC->ADCR &= ~(0xFF);           // Clear all channel select bits
    LPC_ADC->ADCR |= (1 << channel);    // Select the requested channel

    LPC_ADC->ADCR |= (1 << 24);         // START[26:24] = 001: start now

    while (!(LPC_ADC->ADGDR & (1UL << 31))); // Poll DONE bit (bit 31)

    result = LPC_ADC->ADGDR;            // Read entire ADGDR register

    LPC_ADC->ADCR &= ~(7 << 24);        // Clear START bits (stop conversion)

    return (result >> 4) & 0xFFF;       // Shift result: bits[15:4] → bits[11:0]
                                        // Mask to 12 bits: & 0xFFF
}
```

---

## 7. Test Program (main.c)

Reads potentiometer continuously and prints value via UART:
```
ADC Value: 2047
ADC Value: 2100
ADC Value: 3012
...
```
Turning the potentiometer changes the value between 0 and 4095.

---

## 8. Hardware Testing Procedure

### Build & Flash
1. Add: `adc.c`, `uart.c`, `main.c`, `startup_LPC17xx.s`, `system_LPC17xx.c`
2. **F7** → 0 errors → flash `adc.hex` via Flash Magic
3. Press RESET

### Expected Output
Open PuTTY (9600 8N1):
```
ADC Value: XXXX
```
Rotate the onboard potentiometer → number changes from 0 to 4095.

### Init Flow Diagram
```
PCONP bit 12 = 1 (ADC ON)
      │
PCLKSEL0[25:24] = 00 (PCLK/4)
      │
PINSEL1[17:16] = 01 (P0.24 = AD0.1)
      │
ADCR: SEL=ch1, CLKDIV=4, PDN=1
      │
ADC Ready
```

### Debugging
- **Peripheral view → ADC:** after init, ADCR PDN bit = 1
- **Watch `adc_value`:** observe values update
- **Memory: 0x40034004** (ADGDR) — bit 31 goes HIGH when conversion done
