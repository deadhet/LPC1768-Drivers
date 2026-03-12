# PWM Driver — LPC1768

## 1. Driver Overview

**PWM (Pulse Width Modulation)** generates a square wave with a programmable ON/OFF ratio (duty cycle). By changing the duty cycle, you can control the power delivered to a load — without dissipating energy in a linear regulator.

**Real-world applications:** LED brightness control, DC motor speed control, servo motor position, buzzer tone generation, switch-mode power supplies.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| PWM1.1 | PORT2 | P2.0 | Output | PWM signal output |

> ⚠️ P2.0 is shared with 7-segment segment A. Do not use both simultaneously.

**PINSEL4 configuration:** `LPC_PINCON->PINSEL4 |= (1 << 0)` → P2.0 = PWM1.1 function

---

## 3. Registers Used

| Register | Value | Description |
|---------|-------|-------------|
| `PCONP` bit 6 | `(1 << 6)` | Enable PWM1 power |
| `PCLKSEL0` [13:12] | `00` | PWM1 PCLK = CCLK/4 |
| `PINSEL4` [1:0] | `01` | P2.0 = PWM1.1 |
| `LPC_PWM1->TCR` | reset, then `(1<<0)\|(1<<3)` | Reset, then counter+PWM enable |
| `LPC_PWM1->PR` | 0 | No prescaler (25 MHz resolution) |
| `LPC_PWM1->MR0` | period | Period register (e.g., 25000 for 1 kHz) |
| `LPC_PWM1->MR1` | duty | Duty cycle: MR1 = MR0 × (duty%/100) |
| `LPC_PWM1->MCR` | `(1 << 1)` | Reset TC on MR0 match |
| `LPC_PWM1->PCR` | `(1 << 9)` | Enable PWM1.1 output |
| `LPC_PWM1->LER` | `(1<<0)\|(1<<1)` | Latch MR0 and MR1 |

**Period calculation:**
```
PCLK = 25 MHz, Frequency = 1000 Hz
Period = PCLK / Frequency = 25,000
Duty 50%: MR1 = 25,000 × 50 / 100 = 12,500
```

---

## 4. Code Walkthrough

```c
void PWM1_SetDuty(uint8_t duty_percent)
{
    uint32_t period = LPC_PWM1->MR0;    // Read current period
    LPC_PWM1->MR1 = (period * duty_percent) / 100; // Calculate new duty
    LPC_PWM1->LER |= (1 << 1);         // Latch MR1 → takes effect next PWM cycle
    // Without LER, MR1 changes at the wrong time and glitches the waveform
}
```

---

## 5. Hardware Testing Procedure

### Expected Output
Connect oscilloscope to P2.0 (PWM header on trainer board). Observe 50% duty cycle square wave at the configured frequency. Call `PWM1_SetDuty()` with different values to observe duty cycle change.

### Init Flow Diagram
```
PCONP[6] = 1
      │
PCLKSEL0: PWM = CCLK/4
      │
PINSEL4: P2.0 = PWM1.1
      │
TCR reset → PR=0 → MR0=period → MR1=50%
      │
MCR: reset on MR0
      │
PCR[9] = 1 (enable PWM1.1 output)
      │
LER: latch MR0, MR1
      │
TCR: Counter + PWM enable
      │
PWM Running (50% duty)
```
