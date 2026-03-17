# PWM Driver — LPC1768

## 1. Driver Overview

**PWM (Pulse Width Modulation)** generates a square wave signal where the ratio of the ON time to the total period (the duty cycle) is programmable. Although the output is purely digital (only 0V or 3.3V), varying the duty cycle effectively controls the average power delivered to a load. This is how LED brightness is smoothly varied, how DC motor speed is controlled, and how servo motors are positioned.

The LPC1768 has a dedicated PWM1 timer peripheral that generates hardware-accurate PWM waveforms without software overhead. Once configured, the hardware generates the waveform indefinitely and the CPU is free to handle other tasks.

**Real-world applications:**
- LED brightness control (lighting, dimmers)
- DC motor speed control
- Servo motor position control (1ms–2ms pulse at 50 Hz)
- Audio tone generation via piezo buzzers
- Switching power supply control signals

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| PWM1.1 | PORT2 | P2.0 | Output | PWM waveform output |

> **Note:** P2.0 is shared with the 7-segment display Segment A. Do not use both simultaneously. When using PWM, verify PINSEL4[1:0] = 01 (PWM function). When using 7-segment, PINSEL4[1:0] must be cleared to 00 (GPIO).

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power Control

| Bit | Value | Description |
|-----|-------|-------------|
| 6 | 1 | Enable PWM1 peripheral power and clock |

### LPC_SC→PCLKSEL0 — Peripheral Clock Select

| Bits | Value | PCLK | Description |
|------|-------|------|-------------|
| [13:12] | 00 | CCLK/4 = 25 MHz | Default, used by this driver |

### LPC_PINCON→PINSEL4 — Pin Select for P2.0–P2.15

| Bits | Pin | Value | Function |
|------|-----|-------|---------|
| [1:0] | P2.0 | 01 | PWM1.1 output |

### LPC_PWM1→TCR — Timer Control Register

| Bit | Field | Value | Effect |
|-----|-------|-------|--------|
| 0 | CEN | 1 | Counter Enable — start counting |
| 1 | CRST | 1 | Counter Reset — hold counter at 0 |
| 3 | PWMEN | 1 | PWM Enable — activate PWM output logic |

During initialization, TCR is first set to `(1 << 1)` to reset the counter, then later set to `(1 << 0) | (1 << 3)` to both enable counting and enable PWM output simultaneously.

### LPC_PWM1→PR — Prescaler Register

The prescaler divides the peripheral clock before it reaches the timer counter. With PR=0, the prescaler adds 1 to the division, so TC increments at PCLK frequency (25 MHz). This gives a 40 ns resolution per count.

### LPC_PWM1→MR0 — Match Register 0 (Period)

When the timer counter (TC) reaches MR0, it resets to zero (if configured to do so via MCR). MR0 defines the total period of the PWM waveform.

```
Period = MR0 / PCLK
For 1 kHz: MR0 = 25,000,000 / 1000 = 25,000
```

### LPC_PWM1→MR1 — Match Register 1 (Duty Cycle)

When TC reaches MR1, the PWM output goes LOW. The output stays HIGH from TC=0 until TC=MR1.

```
Duty cycle = MR1 / MR0
For 50%: MR1 = 25,000 / 2 = 12,500
For 75%: MR1 = 25,000 × 75 / 100 = 18,750
```

### LPC_PWM1→MCR — Match Control Register

| Bits | Field | Value | Description |
|------|-------|-------|-------------|
| [1:0] | MR0I, MR0R | `(1 << 1)` = 0b10 | Reset TC on MR0 match (no interrupt) |

Setting bit 1 (MR0R) causes the timer counter to automatically reset to 0 when it reaches MR0 value. This creates the repeating PWM period.

### LPC_PWM1→PCR — PWM Control Register

Each PWM output channel (PWM1.1 through PWM1.6) must be individually enabled in PCR:

| Bit | Value | Description |
|-----|-------|-------------|
| 9 | 1 | Enable PWM1.1 output (corresponds to MR1) |

The PCR bit numbering starts at 9 for PWM1.1, 10 for PWM1.2, and so on.

### LPC_PWM1→LER — Load Enable Register

Match register values written to MR0, MR1, etc. do not take effect immediately. They are held in shadow registers. Writing a 1 to the corresponding bit in LER causes the new value to be latched into the hardware at the next period boundary (when TC resets on MR0 match). This prevents glitches in the PWM waveform when updating the duty cycle.

| Bit | Description |
|-----|-------------|
| 0 | Latch MR0 at next period |
| 1 | Latch MR1 at next period |

## 4. Driver Architecture

```
PWM1_Init(frequency)
  ├─ PCONP[6] = 1 (power on)
  ├─ PCLKSEL0[13:12] = 00 (25 MHz)
  ├─ PINSEL4[1:0] = 01 (P2.0 = PWM1.1)
  ├─ TCR[1] = 1 (reset counter)
  ├─ PR = 0 (no prescaler)
  ├─ MR0 = PCLK / frequency (set period)
  ├─ MR1 = MR0 / 2 (50% initial duty)
  ├─ MCR[1] = 1 (reset TC on MR0 match)
  ├─ PCR[9] = 1 (enable PWM1.1 output)
  ├─ LER[0,1] = 1 (latch MR0 and MR1)
  └─ TCR = (1<<0)|(1<<3) (start counter + enable PWM)

PWM1_SetDuty(duty_percent)
  ├─ Read MR0 for current period
  ├─ MR1 = (MR0 × duty_percent) / 100
  └─ LER[1] = 1 (latch new MR1 at next period)
```

## 5. Function Reference

### `PWM1_Init(uint32_t frequency)`
Initializes PWM1 channel 1 at the given frequency with 50% initial duty cycle.

| Parameter | Type | Description |
|-----------|------|-------------|
| `frequency` | uint32_t | Output frequency in Hz (e.g., 1000 for 1 kHz) |

### `PWM1_SetDuty(uint8_t duty_percent)`
Changes the duty cycle without stopping the PWM output. Change takes effect at the next period boundary.

| Parameter | Type | Description |
|-----------|------|-------------|
| `duty_percent` | uint8_t | Duty cycle: 0 (always LOW) to 100 (always HIGH) |

## 6. Code Walkthrough

### Enabling PWM1 Power and Peripheral Clock

```c
LPC_SC->PCONP |= (1 << 6);
LPC_SC->PCLKSEL0 &= ~(3 << 12);
```

Bit 6 of PCONP enables the PWM1 peripheral. PCLKSEL0 bits [13:12] control the PWM1 clock divider. Clearing both bits to 00 selects CCLK/4 = 25 MHz as the PWM clock source.

### Configuring the PWM Output Pin

```c
LPC_PINCON->PINSEL4 &= ~(3 << 0);
LPC_PINCON->PINSEL4 |=  (1 << 0);
```

PINSEL4 controls P2.0–P2.15. P2.0 uses bits [1:0]. Setting them to 01 routes P2.0 to PWM1.1, disconnecting it from GPIO. The physical pin now outputs the hardware-generated PWM waveform.

### Resetting the Counter and Setting Prescaler

```c
LPC_PWM1->TCR = (1 << 1);    // Reset counter
LPC_PWM1->PR = 0;             // No prescaler
```

Setting bit 1 (CRST) holds the timer counter at 0 while configuration continues. This prevents the counter from running with incorrect match values. PR=0 means the effective prescaler divider is PR+1 = 1, so TC increments at the full PCLK rate (25 MHz, 40 ns per count).

### Calculating and Setting the Period and Duty

```c
pclk = SystemCoreClock / 4;
period = pclk / frequency;

LPC_PWM1->MR0 = period;
LPC_PWM1->MR1 = period / 2;  // 50% default duty
```

For 1 kHz: period = 25,000,000 / 1000 = 25,000 counts. MR0=25,000 means the counter runs from 0 to 24,999 (25,000 counts) before resetting. MR1=12,500 means the output goes LOW at the halfway point, giving exactly 50% duty cycle.

### Configuring the Match Control Register

```c
LPC_PWM1->MCR = (1 << 1);
```

Bit 1 (MR0R) in MCR means "reset timer counter when it matches MR0." This is what creates the periodic waveform. Without this, the counter would count up to 0xFFFFFFFF and overflow, and only one PWM pulse would ever be generated.

### Enabling the PWM Output and Latching Values

```c
LPC_PWM1->PCR |= (1 << 9);               // Enable PWM1.1 output drive
LPC_PWM1->LER |= (1 << 0) | (1 << 1);   // Latch MR0 and MR1
LPC_PWM1->TCR = (1 << 0) | (1 << 3);    // CEN=1, PWMEN=1
```

PCR bit 9 enables the PWM1.1 output pin. Without this, the PWM hardware runs internally but the pin stays constant.

Writing to LER bits 0 and 1 signals that the new MR0 and MR1 values should be loaded into the active match hardware on the next period reset. This is the shadow register mechanism that prevents mid-period changes from causing glitches.

Finally, TCR is written to clear CRST and set both CEN (counting) and PWMEN (PWM output enable). The PWM waveform begins immediately.

### Changing Duty Cycle at Runtime

```c
void PWM1_SetDuty(uint8_t duty_percent)
{
    uint32_t period = LPC_PWM1->MR0;
    LPC_PWM1->MR1 = (period * duty_percent) / 100;
    LPC_PWM1->LER |= (1 << 1);
}
```

Reading MR0 gets the current period count. Multiplying by the percentage and dividing by 100 gives the new match value. Writing to LER bit 1 schedules the update to take effect at the next period reset. This is critical — without writing LER, the new MR1 value sits in the shadow register and is never applied.

## 7. Test Program Explanation

The main.c cycles duty cycle from 0% to 100% in 10% steps, pausing at each step. An LED or oscilloscope on P2.0 shows the brightness changing or the duty cycle varying.

## 8. Hardware Testing Procedure

### Expected Output
Connect oscilloscope to P2.0. Observe square wave at configured frequency. Duty cycle changes from 0% to 100% in steps.

### Init Flow Summary
```
PCONP[6] = 1
      │
PCLKSEL0 = 00 (25 MHz)
      │
PINSEL4: P2.0 = PWM1.1
      │
TCR reset → PR=0 → MR0=25000 → MR1=12500 (50%)
      │
MCR: reset TC on MR0 match
      │
PCR[9] = 1 (enable PWM1.1 output)
      │
LER: latch MR0, MR1
      │
TCR: Counter + PWM enable
      │
PWM Running (50% duty at 1 kHz)
```

### Debugging in Keil
- **Peripheral → PWM1:** verify MR0 = 25000, MR1 = 12500 after init
- **Watch MR1:** changes when PWM1_SetDuty() is called
- **Oscilloscope on P2.0:** T = 1ms (1 kHz), high time = 500µs (50%)

## 9. Expected Output

PWM output waveform at 1 kHz, 50% duty cycle (if LED connected):
- LED dim at low duty (10–20%)
- LED medium brightness at 50%
- LED fully bright at 90–100%
