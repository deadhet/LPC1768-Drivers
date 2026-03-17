# 7-Segment Display Driver — LPC1768

## 1. Driver Overview

This driver drives a **7-segment LED display** using GPIO PORT2. A 7-segment display has seven LED segments (A through G) arranged in a figure-8 pattern, plus an optional decimal point (DP). By selectively lighting different combinations of segments, digits 0–9 and some letters can be displayed.

The driver uses a pre-computed lookup table to map each digit to its corresponding segment pattern. Writing a single 8-bit value to the GPIO output register drives all eight lines simultaneously.

**Real-world applications:**
- Digital clocks and timers
- Odometer and counter displays
- Digital voltmeter readouts
- Score displays in embedded games

## 2. Hardware Interface

| Signal | Port | Pin | Description |
|--------|------|-----|-------------|
| Segment A | PORT2 | P2.0 | Top horizontal bar |
| Segment B | PORT2 | P2.1 | Upper right vertical |
| Segment C | PORT2 | P2.2 | Lower right vertical |
| Segment D | PORT2 | P2.3 | Bottom horizontal bar |
| Segment E | PORT2 | P2.4 | Lower left vertical |
| Segment F | PORT2 | P2.5 | Upper left vertical |
| Segment G | PORT2 | P2.6 | Middle horizontal bar |
| Decimal Point | PORT2 | P2.7 | Decimal point |

```
 _
|_|    ← each side = one segment
|_| .  ← dot = decimal point

Segments:
    AAA      P2.0
   F   B     P2.5, P2.1
    GGG      P2.6
   E   C     P2.4, P2.2
    DDD   DP P2.3,  P2.7
```

> **Important:** P2.0 is shared with PWM1.1. When using the 7-segment display, ensure PINSEL4[1:0] = 00 (GPIO mode). If PWM was previously used, this must be explicitly cleared.

## 3. Registers Used

### LPC_GPIO2→FIODIR — Direction Register

```c
LPC_GPIO2->FIODIR |= 0xFF;
```

This sets bits 0 through 7 of PORT2's direction register to 1, making all eight pins (P2.0–P2.7) outputs. `0xFF` in binary is `11111111` — all 8 bits set to 1.

### LPC_GPIO2→FIOPIN — Pin Output Register

```c
LPC_GPIO2->FIOPIN = seg_pattern[digit];
```

Writing to FIOPIN sets all output pins at once. The lower 8 bits (bits 7:0) drive P2.7–P2.0 respectively. Each bit directly controls the corresponding segment LED.

No PCONP register access is needed for GPIO — the GPIO peripheral is always powered.

## 4. Segment Encoding Table

Each digit's pattern byte encodes which segments are ON. The bit mapping is:

| Bit | Port Pin | Segment |
|-----|---------|---------|
| 0 | P2.0 | A (top) |
| 1 | P2.1 | B (upper right) |
| 2 | P2.2 | C (lower right) |
| 3 | P2.3 | D (bottom) |
| 4 | P2.4 | E (lower left) |
| 5 | P2.5 | F (upper left) |
| 6 | P2.6 | G (middle) |
| 7 | P2.7 | DP (decimal point) |

| Digit | Segments ON | Binary (bit7→bit0) | Hex |
|-------|------------|---------------------|-----|
| 0 | A,B,C,D,E,F | 00111111 | 0x3F |
| 1 | B,C | 00000110 | 0x06 |
| 2 | A,B,D,E,G | 01011011 | 0x5B |
| 3 | A,B,C,D,G | 01001111 | 0x4F |
| 4 | B,C,F,G | 01100110 | 0x66 |
| 5 | A,C,D,F,G | 01101101 | 0x6D |
| 6 | A,C,D,E,F,G | 01111101 | 0x7D |
| 7 | A,B,C | 00000111 | 0x07 |
| 8 | A,B,C,D,E,F,G | 01111111 | 0x7F |
| 9 | A,B,C,D,F,G | 01101111 | 0x6F |

**Verification example for digit 2 (0x5B = 01011011):**
- Bit 0 = 1 → A ON (top horizontal)
- Bit 1 = 1 → B ON (upper right)
- Bit 2 = 0 → C OFF (lower right)
- Bit 3 = 1 → D ON (bottom horizontal)
- Bit 4 = 1 → E ON (lower left)
- Bit 5 = 0 → F OFF (upper left)
- Bit 6 = 1 → G ON (middle horizontal)
- Result: top bar, upper-right, bottom bar, lower-left, middle bar → forms the digit 2

## 5. Function Reference

### `SevenSeg_Init(void)`
Sets P2.0–P2.7 as outputs.

### `SevenSeg_Display(uint16_t number)`
Extracts the units digit of `number` (using `% 10`) and writes the corresponding segment pattern to PORT2.

## 6. Code Walkthrough

### Initializing the Display Pins

```c
void SevenSeg_Init(void)
{
    LPC_GPIO2->FIODIR |= 0xFF;
}
```

`0xFF` is the hexadecimal value 255. In binary: `1111 1111`. Writing this using `|=` to FIODIR bits [7:0] makes all 8 pins outputs simultaneously. This is the only hardware setup required for 7-segment. No clock enable, no pin function select — the PORT2 GPIO pins default to GPIO mode.

No PCONP bit is needed. GPIO functionality is always active on the LPC1768 and does not require power-enable.

### Writing a Digit to the Display

```c
void SevenSeg_Display(uint16_t number)
{
    uint8_t digit = number % 10;           // Extract units digit
    LPC_GPIO2->FIOPIN = seg_pattern[digit]; // Write pattern to port
}
```

`number % 10` extracts the rightmost decimal digit (0–9). For example, if number=47, digit=7. The lookup table `seg_pattern[digit]` returns the pre-computed 8-bit segment map for that digit.

Writing to FIOPIN sets all 8 output pins atomically. Unlike using FIOSET/FIOCLR, which require separate operations to turn some segments on and others off, writing to FIOPIN sets every pin in one operation — those with a 1 in the mask go HIGH (LED on) and those with a 0 go LOW (LED off). This prevents any intermediate states where wrong segments might flash briefly.

For digit 7: `seg_pattern[7] = 0x07 = 0000 0111`
- P2.0 = 1 (A on)
- P2.1 = 1 (B on)
- P2.2 = 1 (C on)
- All other pins = 0

Result: only segments A, B, C are lit — which is the correct representation of 7 (top, upper-right, lower-right).

## 7. Test Program Explanation

The main.c cycles digits 0–9 with a ~500 ms delay between each, repeating indefinitely. This demonstrates all 10 digit patterns in sequence.

## 8. Hardware Testing Procedure

### Expected Output
Digits 0 through 9 appear in sequence on the display, each visible for approximately 500 ms.

### Init Flow Summary
```
FIODIR PORT2 bits[7:0] = 0xFF (all outputs)
      │
Loop digit 0 to 9:
  FIOPIN = seg_pattern[i] → segments light accordingly
  delay()
```

### Debugging in Keil
- **Peripheral → GPIO Port 2:** watch FIOPIN bits change with each digit written
- **Verify PINSEL4[1:0] = 00:** if P2.0 is not GPIO, segment A will not respond to FIOPIN writes
- If some segments don't light: verify segment connections and that the display is common-cathode or common-anode as expected

## 9. Expected Output

```
Display shows:  0 → 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → (repeat)
Each digit visible for ~500 ms
```
