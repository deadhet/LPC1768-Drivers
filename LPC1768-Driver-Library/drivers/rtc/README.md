# RTC Driver: LPC1768

## 1. Driver Overview

The **Real-Time Clock (RTC)** is a specialized peripheral that maintains accurate time and date information. Unlike Timer0 which counts general-purpose timer ticks, the RTC is designed specifically to count seconds, minutes, hours, days, months, and years. The RTC has dedicated registers for each time unit, and the hardware automatically handles the increment logic seconds roll over at 60 and trigger a minute increment, minutes roll over at 60 and trigger an hour increment, and so on.

The LPC1768 RTC can continue running from a separate 3.3V backup battery even when the main system is powered off, preserving time across power cycles. On the trainer board, the RTC is powered from the main supply.

**Real-world applications:**
- Digital clocks and watches
- Data loggers that need timestamps on recorded values
- Alarm systems with scheduled events
- Calendar applications in embedded devices

## 2. Hardware Interface

| Signal | Description |
|--------|-------------|
| RTC peripheral (internal) | No external pins needed for timekeeping |
| UART0 (P0.2/P0.3) | Debug output: prints current time to serial terminal |

The RTC is entirely internal. It is driven by the system's 32.768 kHz crystal oscillator (XTAL1/XTAL2), though it can also be clocked from the main PLL.

## 3. Registers Used

### LPC_SC→PCONP — Peripheral Power Control

| Bit | Value | Description |
|-----|-------|-------------|
| 9 | 1 | Enable RTC clock and interface |

### LPC_RTC→CCR — Clock Control Register

The CCR is the main control register for the RTC. It starts, stops, and resets the clock counters.

| Bit | Field | Description |
|-----|-------|-------------|
| 0 | CLKEN | 1 = enable RTC counting, 0 = stop |
| 1 | CTCRST | 1 = reset all time counters to 0 |
| 4 | CCALEN | 1 = enable calendar (date) registers |

### Time Registers

These registers are directly readable and writable. The RTC hardware automatically increments them in order.

| Register | Range | Description |
|----------|-------|-------------|
| `LPC_RTC->SEC` | 0–59 | Seconds |
| `LPC_RTC->MIN` | 0–59 | Minutes |
| `LPC_RTC->HOUR` | 0–23 | Hours (24-hour format) |
| `LPC_RTC->DOM` | 1–31 | Day of Month |
| `LPC_RTC->MONTH` | 1–12 | Month |
| `LPC_RTC->YEAR` | 0–4095 | Year |

Writing to these registers sets the initial time before starting the RTC. Reading them at any time returns the current value. The hardware updates them automatically every second once CLKEN is set.

## 4. Driver Architecture

```
RTC_Init()
  ├─ PCONP[9] = 1 (power on RTC)
  ├─ CCR = 0x00 (stop RTC)
  ├─ CCR[1] = 1 (reset all counters)
  ├─ CCR[1] = 0 (release reset)
  └─ CCR[0] = 1 (start RTC clock)

RTC_SetTime(hr, min, sec) → Write HOUR, MIN, SEC registers
RTC_SetDate(day, mon, yr) → Write DOM, MONTH, YEAR registers
RTC_GetTime(*hr, *min, *sec) → Read HOUR, MIN, SEC registers
```

## 5. Function Reference

### `RTC_Init(void)`
Powers on RTC, resets all counters, starts the clock.

### `RTC_SetTime(uint8_t hr, uint8_t min, uint8_t sec)`
Sets hours, minutes, seconds. Must be called before starting the RTC, or while it is running (values take effect immediately).

### `RTC_SetDate(uint8_t day, uint8_t month, uint16_t year)`
Sets day of month, month, and year.

### `RTC_GetTime(uint8_t *hr, uint8_t *min, uint8_t *sec)`
Reads current hours, minutes, and seconds into the provided pointers.

## 6. Code Walkthrough

### Enabling RTC Power

```c
LPC_SC->PCONP |= (1 << 9);
```

Bit 9 of PCONP (PCRTC) enables the RTC peripheral clock. The PCONP bit must be set before any RTC register access. For the RTC specifically, the hardware may maintain a low-power sub-clock even when PCONP bit 9 is cleared, but the register interface requires the main clock to be enabled.

### Stopping the RTC Before Configuration

```c
LPC_RTC->CCR = 0x00;
```

Writing 0x00 to CCR clears all control bits, stopping the RTC counter. It is important to stop the RTC before setting time registers to avoid a race condition where the hardware increments a register at the exact moment you write to it.

### Resetting All Time Counters

```c
LPC_RTC->CCR = (1 << 1);
LPC_RTC->CCR &= ~(1 << 1);
```

Setting bit 1 (CTCRST) clears SEC, MIN, HOUR, DOM, MONTH, and YEAR all to their reset values (0 or 1 as appropriate). Then bit 1 is cleared to release the reset. This ensures no stale values remain from a previous run. The clear step uses `&= ~(1 << 1)` to clear only bit 1 while preserving other CCR bits.

### Starting the RTC

```c
LPC_RTC->CCR |= (1 << 0);
```

Setting bit 0 (CLKEN) starts the RTC. From this moment, the hardware begins counting. Every second, SEC increments. When SEC reaches 60, it resets to 0 and MIN increments, and so on throughout the full calendar hierarchy.

### Setting the Time

```c
void RTC_SetTime(uint8_t hr, uint8_t min, uint8_t sec)
{
    LPC_RTC->HOUR = hr;
    LPC_RTC->MIN  = min;
    LPC_RTC->SEC  = sec;
}
```

These registers are simply written with the desired values. The hardware accepts the write immediately. Writing while the RTC is running is allowed, the new value becomes the current count. Care should be taken not to write inconsistent values (e.g., writing SEC = 59, then the hardware increments it to 60, which rolls over and increments MIN before you write MIN).

In practice, it is safest to stop the RTC (CCR[0] = 0), write all time registers, then restart (CCR[0] = 1).

### Reading the Time

```c
void RTC_GetTime(uint8_t *hr, uint8_t *min, uint8_t *sec)
{
    *hr  = LPC_RTC->HOUR;
    *min = LPC_RTC->MIN;
    *sec = LPC_RTC->SEC;
}
```

The time registers are directly readable at any time. The RTC hardware updates them autonomously in the background. Each read returns the current value at that instant. For time critical applications, you should be aware that a rollover could occur between reading SEC and reading MIN, giving an inconsistent snapshot (e.g., SEC=59 read just before rollover, MIN read just after). For a display clock this is typically acceptable.

## 7. Test Program Explanation

The main.c:
1. Initializes RTC
2. Sets the date to 21/5/2026 and time to 10:30:00
3. Starts the RTC
4. Enters a loop that reads and prints the time via UART every ~1 second

As the firmware runs, the seconds increment on screen, demonstrating that the RTC is counting independently.

## 8. Hardware Testing Procedure

### Expected Output (Flash Magic, 9600 baud)
```
Time: 10:30:00
Time: 10:30:01
Time: 10:30:02
...
```

### Init Flow Summary
```
PCONP[9] = 1
      │
CCR = 0 (stop)
      │
CCR[1] = 1 → 0 (reset counters)
      │
SetTime(10, 30, 0) → HOUR=10, MIN=30, SEC=0
SetDate(21, 5, 2026) → DOM=21, MONTH=5, YEAR=2026
      │
CCR[0] = 1 (start)
      │
RTC counting — increments automatically every second
```

### Debugging in Keil
- **Peripheral → RTC:** watch SEC increment every second in real time
- **Watch `LPC_RTC->HOUR`/`MIN`/`SEC`:** live values update as clock counts
- **If time not incrementing:** verify CCR[0]=1 (CLKEN set)
