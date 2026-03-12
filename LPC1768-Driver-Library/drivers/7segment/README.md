# 7-Segment Display Driver — LPC1768

## 1. Driver Overview

This driver drives a **7-segment display** using GPIO PORT2. A lookup table maps digits 0–9 to their binary segment patterns (a–g).

**Real-world applications:** Digital clocks, counters, voltmeters, odometers, score displays.

---

## 2. Hardware Interface

| Signal | Port | Pin | Description |
|--------|------|-----|-------------|
| Segment A | PORT2 | P2.0 | Top horizontal |
| Segment B | PORT2 | P2.1 | Upper right |
| Segment C | PORT2 | P2.2 | Lower right |
| Segment D | PORT2 | P2.3 | Bottom horizontal |
| Segment E | PORT2 | P2.4 | Lower left |
| Segment F | PORT2 | P2.5 | Upper left |
| Segment G | PORT2 | P2.6 | Middle horizontal |
| Decimal | PORT2 | P2.7 | Decimal point |

> ⚠️ P2.0 is shared with PWM1.1. Configure as GPIO (PINSEL4 clear) when using 7-segment.

---

## 3. Segment Encoding Table

| Digit | gfedcba | Hex | Description |
|-------|---------|-----|-------------|
| 0 | 0011 1111 | 0x3F | All segments except G |
| 1 | 0000 0110 | 0x06 | B and C only |
| 2 | 0101 1011 | 0x5B | A,B,D,E,G |
| 3 | 0100 1111 | 0x4F | A,B,C,D,G |
| 4 | 0110 0110 | 0x66 | B,C,F,G |
| 5 | 0110 1101 | 0x6D | A,C,D,F,G |
| 6 | 0111 1101 | 0x7D | A,C,D,E,F,G |
| 7 | 0000 0111 | 0x07 | A,B,C |
| 8 | 0111 1111 | 0x7F | All segments |
| 9 | 0110 1111 | 0x6F | A,B,C,D,F,G |

---

## 4. Code Walkthrough

```c
const uint8_t seg_pattern[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

void SevenSeg_Init(void)
{
    LPC_GPIO2->FIODIR |= 0xFF; // P2.0–P2.7 all as outputs (0xFF = all 8 bits)
}

void SevenSeg_Display(uint16_t number)
{
    uint8_t digit = number % 10;         // Extract rightmost digit (0–9)
    LPC_GPIO2->FIOPIN = seg_pattern[digit]; // Write pattern to all 8 bits at once
    // FIOPIN write sets ALL output pins simultaneously — clean, glitch-free
}
```

---

## 5. Hardware Testing Procedure

### Expected Output
Digits 0–9 cycle on the 7-segment display with a ~500ms delay between each.

### Init Flow Diagram
```
FIODIR PORT2 bits[7:0] = 0xFF (all outputs)
      │
Loop digit 0–9:
  SevenSeg_Display(i) → FIOPIN = seg_pattern[i]
  delay()
```
