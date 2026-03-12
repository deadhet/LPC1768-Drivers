# LCD Driver — LPC1768

## 1. Driver Overview

This driver controls a standard **16×2 HD44780 character LCD** using the LPC1768's GPIO port in **4-bit mode**, saving 4 GPIO lines compared to 8-bit mode.

**Real-world applications:** Status display, menu systems, sensor readouts, clocks, debug output without UART.

---

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| RS | PORT1 | P1.0 | Output | 0=Command, 1=Data |
| EN | PORT1 | P1.1 | Output | Enable strobe |
| D4 | PORT1 | P1.9 | Output | Data bit 4 |
| D5 | PORT1 | P1.10 | Output | Data bit 5 |
| D6 | PORT1 | P1.14 | Output | Data bit 6 |
| D7 | PORT1 | P1.15 | Output | Data bit 7 |

**LCD VDD → 3.3V, VSS → GND, R/W → GND (always write mode)**
**Contrast (V0) → center tap of 10K potentiometer**

---

## 3. LCD Commands Reference

| Command | Code | Description |
|---------|------|-------------|
| Home | 0x02 | Move cursor to position (0,0) |
| 4-bit 2-line | 0x28 | Configure LCD mode |
| Display ON | 0x0C | Turn on LCD, no cursor |
| Entry mode | 0x06 | Cursor auto-increment |
| Clear | 0x01 | Clear all characters |
| Row 0 col N | 0x80+N | Position cursor |
| Row 1 col N | 0xC0+N | Position cursor |

---

## 4. Driver Architecture

```
LCD_SendNibble(nibble)
  ├─ Clear D4-D7
  ├─ Set D4-D7 based on nibble bits
  ├─ Pulse EN HIGH → LOW
  └─ (4 bits transferred to LCD)

LCD_Command(cmd)
  ├─ RS = LOW (command mode)
  ├─ LCD_SendNibble(cmd >> 4)   — upper nibble first
  └─ LCD_SendNibble(cmd & 0x0F) — lower nibble second

LCD_Data(data)
  ├─ RS = HIGH (data mode)
  ├─ LCD_SendNibble(data >> 4)
  └─ LCD_SendNibble(data & 0x0F)
```

---

## 5. Code Walkthrough

```c
void LCD_SendNibble(unsigned char nibble)
{
    LPC_GPIO1->FIOCLR = D4|D5|D6|D7;        // Clear all 4 data pins

    if(nibble & 0x01) LPC_GPIO1->FIOSET = D4; // Set D4 if bit 0 is set
    if(nibble & 0x02) LPC_GPIO1->FIOSET = D5; // Set D5 if bit 1 is set
    if(nibble & 0x04) LPC_GPIO1->FIOSET = D6; // Set D6 if bit 2 is set
    if(nibble & 0x08) LPC_GPIO1->FIOSET = D7; // Set D7 if bit 3 is set

    LPC_GPIO1->FIOSET = EN;  // EN high → LCD latches data
    LCD_Delay();
    LPC_GPIO1->FIOCLR = EN;  // EN low → transfer complete
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;
    if(row == 0) address = 0x80 + col;  // Row 0: DDRAM starts at 0x00 → cmd 0x80+col
    else         address = 0xC0 + col;  // Row 1: DDRAM starts at 0x40 → cmd 0xC0+col
    LCD_Command(address);               // Send as command (RS=0)
}
```

---

## 6. Test Program (main.c)

Runs 5 sequential tests:
1. **Simple message** — "LCD DRIVER OK" / "TEST START"
2. **Clear test** — clears and shows "CLEAR WORKING"
3. **Cursor test** — places text at specific positions
4. **Character test** — shows "CHAR TEST"
5. **Counter loop** — displays incrementing count 000–999

---

## 7. Hardware Testing Procedure

### Expected Output
LCD shows "LCD DRIVER OK" on row 0, "TEST START" on row 1. Then cycles through tests, then a running counter on row 1.

### Init Flow Diagram
```
Configure P1.0, P1.1, P1.9, P1.10, P1.14, P1.15 as GPIO outputs
      │
Power-on delay (>40ms)
      │
LCD_Command(0x02) — Home (activate 4-bit mode)
      │
LCD_Command(0x28) — 4-bit, 2-line, 5×7 font
      │
LCD_Command(0x0C) — Display ON, cursor off
      │
LCD_Command(0x06) — Entry mode: increment, no shift
      │
LCD_Clear() — clear display
      │
LCD Ready
```

### Debugging
- If LCD shows blocks: increase contrast (turn V0 potentiometer)
- If LCD shows garbage: check EN pulsewidth and delay timing
- If nothing shows: verify RS/EN connections; check FIODIR for all pins
