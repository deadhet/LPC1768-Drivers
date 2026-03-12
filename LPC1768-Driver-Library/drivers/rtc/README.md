# RTC Driver — LPC1768

## 1. Driver Overview

The **Real-Time Clock (RTC)** maintains accurate time and date even when the main CPU is reset or powered off (when a backup battery is connected). It runs from a dedicated 32.768 kHz oscillator.

**Real-world applications:** Digital clocks, data loggers with timestamps, alarms, calendar applications, event schedulers.

---

## 2. Hardware Interface

| Signal | Description |
|--------|-------------|
| RTC peripheral (internal) | No external pins needed for time-keeping |
| UART0 (P0.2/P0.3) | Debug output — prints current time to serial terminal |

---

## 3. Registers Used

| Register | Address | Description |
|---------|---------|-------------|
| `LPC_SC->PCONP` bit 9 | 0x400FC0C4 | Enable RTC clock |
| `LPC_RTC->CCR` | 0x40024008 | Clock Control: bit0=enable, bit1=reset |
| `LPC_RTC->SEC` | 0x40024020 | Seconds (0–59) |
| `LPC_RTC->MIN` | 0x40024024 | Minutes (0–59) |
| `LPC_RTC->HOUR` | 0x40024028 | Hours (0–23) |
| `LPC_RTC->DOM` | 0x4002402C | Day of Month (1–31) |
| `LPC_RTC->MONTH` | 0x40024038 | Month (1–12) |
| `LPC_RTC->YEAR` | 0x4002403C | Year (0–4095) |

---

## 4. Driver Architecture

```
RTC_Init()
  ├─ PCONP[9] = 1 (power ON)
  ├─ CCR = 0x00  (stop clock)
  ├─ CCR[1] = 1  (reset counters)
  ├─ CCR[1] = 0  (release reset)
  └─ CCR[0] = 1  (start clock)

RTC_SetTime(hr, min, sec) → write HOUR, MIN, SEC directly
RTC_SetDate(day, mon, yr) → write DOM, MONTH, YEAR
RTC_GetTime(*hr, *min, *sec) → read HOUR, MIN, SEC
```

---

## 5. Code Walkthrough

```c
void RTC_Init(void)
{
    LPC_SC->PCONP |= (1 << 9);   // Enable RTC power
    LPC_RTC->CCR = 0x00;         // Stop RTC before configuring
    LPC_RTC->CCR = (1 << 1);     // Reset all time counters (SEC, MIN, HOUR...)
    LPC_RTC->CCR &= ~(1 << 1);   // Release reset (self-clearing but explicit is safer)
    LPC_RTC->CCR |= (1 << 0);    // Start RTC clock
}
void RTC_GetTime(uint8_t *hr, uint8_t *min, uint8_t *sec)
{
    *hr  = LPC_RTC->HOUR;        // Read hours register directly
    *min = LPC_RTC->MIN;         // Read minutes register
    *sec = LPC_RTC->SEC;         // Read seconds register
    // RTC hardware updates these registers every second automatically
}
```

---

## 6. Test Program (main.c)

Sets date to 21/5/2026, time to 10:30:00, then prints `Time: HH:MM:SS` via UART every ~1 second.

---

## 7. Hardware Testing Procedure

### Expected Output (PuTTY 9600)
```
Time: 10:30:00
Time: 10:30:01
Time: 10:30:02
...
```

### Init Flow Diagram
```
PCONP[9] = 1
      │
CCR = 0 (stop)
      │
CCR[1] = 1 → 0 (reset)
      │
SET: HOUR, MIN, SEC, DOM, MONTH, YEAR
      │
CCR[0] = 1 (start)
      │
RTC Counting
```
