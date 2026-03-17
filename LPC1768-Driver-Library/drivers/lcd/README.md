# LCD Driver — LPC1768

## 1. Driver Overview

This driver controls a standard **16×2 HD44780 character LCD** using the LPC1768's GPIO port in **4-bit mode**, saving 4 GPIO lines compared to 8-bit mode.

The HD44780 is a parallel interface LCD controller. It expects commands and data as 8-bit values, but in 4-bit mode these are split into two 4-bit transfers (nibbles), sending the high nibble first. The LCD has three control signals: RS (Register Select) to distinguish commands from data, EN (Enable) which is pulsed to latch data into the LCD, and R/W (Read/Write) which is tied LOW for write-only mode.

**Real-world applications:**
- Status display in embedded devices
- Menu navigation systems
- Sensor data readouts
- Debug output without UART

## 2. Hardware Interface

| Signal | Port | Pin | Direction | Description |
|--------|------|-----|-----------|-------------|
| RS | PORT1 | P1.0 | Output | 0=Command, 1=Data |
| EN | PORT1 | P1.1 | Output | Enable strobe (pulse HIGH→LOW to latch) |
| D4 | PORT1 | P1.9 | Output | Data bit 4 |
| D5 | PORT1 | P1.10 | Output | Data bit 5 |
| D6 | PORT1 | P1.14 | Output | Data bit 6 |
| D7 | PORT1 | P1.15 | Output | Data bit 7 |

**LCD connections:** VDD → 3.3V, VSS → GND, R/W pin → GND (always write mode). The contrast pin (V0) connects to the wiper of a 10K potentiometer between 3.3V and GND.

The pin selections (P1.0, P1.1, P1.9, P1.10, P1.14, P1.15) are non-consecutive due to the trainer board layout. The driver maps these physical pins to the HD44780 interface using individual FIOSET/FIOCLR operations.

## 3. HD44780 Commands Reference

| Command | Code | Description |
|---------|------|-------------|
| Return Home | 0x02 | Move cursor to column 0, row 0 |
| Function Set | 0x28 | 4-bit mode, 2 lines, 5×7 font |
| Display ON | 0x0C | Display on, cursor off, blink off |
| Entry Mode | 0x06 | Cursor moves right after each character, no display shift |
| Clear Display | 0x01 | Clear all characters, cursor to home |
| Row 0 col N | 0x80 + col | Set DDRAM address for row 0 |
| Row 1 col N | 0xC0 + col | Set DDRAM address for row 1 |

The HD44780 has two internal memories: DDRAM (Display Data RAM, stores characters) and CGRAM (Character Generator RAM, for custom characters). Normal text output writes to DDRAM. The "Set DDRAM address" command moves the cursor to a specific character position.

## 4. Driver Architecture

```
LCD_SendNibble(nibble)
  ├─ Clear D4-D7 lines (FIOCLR)
  ├─ Set D4-D7 based on nibble bits (FIOSET per bit)
  ├─ Pulse EN HIGH (FIOSET)
  └─ Pulse EN LOW (FIOCLR) → LCD latches the 4 bits

LCD_Command(cmd)
  ├─ RS = LOW (FIOCLR) — command mode
  ├─ LCD_SendNibble(cmd >> 4) — upper nibble first
  └─ LCD_SendNibble(cmd & 0x0F) — lower nibble second

LCD_Data(data)
  ├─ RS = HIGH (FIOSET) — data mode
  ├─ LCD_SendNibble(data >> 4)
  └─ LCD_SendNibble(data & 0x0F)
```

## 5. Function Reference

| Function | Description |
|----------|-------------|
| `LCD_Init(void)` | Configure GPIO pins, run HD44780 initialization sequence |
| `LCD_Command(unsigned char cmd)` | Send a control command byte |
| `LCD_Data(unsigned char data)` | Send a character byte to display |
| `LCD_String(char *str)` | Display a null-terminated string |
| `LCD_Clear(void)` | Clear all characters |
| `LCD_SetCursor(row, col)` | Position cursor at row (0 or 1) and column |
| `LCD_SendNibble(unsigned char nibble)` | Internal: send 4 bits with EN pulse |

## 6. Code Walkthrough

### Configuring GPIO Pins

```c
LPC_PINCON->PINSEL3 &= ~(0x3 << 0);    // P1.0 = GPIO
LPC_PINCON->PINSEL3 &= ~(0x3 << 2);    // P1.1 = GPIO
LPC_PINCON->PINSEL3 &= ~(0x3 << 18);   // P1.9 = GPIO
LPC_PINCON->PINSEL3 &= ~(0x3 << 20);   // P1.10 = GPIO
LPC_PINCON->PINSEL3 &= ~(0x3 << 28);   // P1.14 = GPIO
LPC_PINCON->PINSEL3 &= ~(0x3 << 30);   // P1.15 = GPIO
```

PINSEL3 controls P1.16–P1.31, and overlaps with some bits for lower port pins on some headers. The masks clear 2 bits per pin to 00, placing each pin in default GPIO mode. This is necessary because some of these pins can be connected to alternate functions (JTAG, Ethernet, etc.) and must be explicitly set to GPIO before use.

Then all six pins are configured as outputs:

```c
LPC_GPIO1->FIODIR |= RS | EN | D4 | D5 | D6 | D7;
```

The macros expand to their respective bit masks: RS=(1<<0), EN=(1<<1), D4=(1<<9), D5=(1<<10), D6=(1<<14), D7=(1<<15). The single `|=` sets all six direction bits simultaneously.

### HD44780 Initialization Sequence

```c
LCD_Command(0x02);   // Return home (enters 4-bit mode)
LCD_Command(0x28);   // 4-bit, 2-line, 5×7
LCD_Command(0x0C);   // Display ON, no cursor
LCD_Command(0x06);   // Auto-increment cursor
LCD_Clear();         // Clear display
```

The HD44780 starts in an undefined state. A specific initialization sequence is required. The `0x02` command (Return Home) is particularly important — it causes the LCD controller to accept 4-bit mode. After this, 0x28 (Function Set) formally configures 4-bit communication, 2-line display, and 5×7 character font.

0x0C turns on the display (bit 2 = display enable, bit 1 = cursor on/off, bit 0 = cursor blink). With 0x0C, the display is on with no visible cursor.

0x06 (Entry Mode) sets direction: bit 1=1 means cursor moves right after each character, bit 0=0 means the display does not shift.

### Sending a 4-Bit Nibble

```c
void LCD_SendNibble(unsigned char nibble)
{
    LPC_GPIO1->FIOCLR = D4 | D5 | D6 | D7;  // Clear all data pins

    if(nibble & 0x01) LPC_GPIO1->FIOSET = D4; // bit 0 → D4 (P1.9)
    if(nibble & 0x02) LPC_GPIO1->FIOSET = D5; // bit 1 → D5 (P1.10)
    if(nibble & 0x04) LPC_GPIO1->FIOSET = D6; // bit 2 → D6 (P1.14)
    if(nibble & 0x08) LPC_GPIO1->FIOSET = D7; // bit 3 → D7 (P1.15)

    LPC_GPIO1->FIOSET = EN;   // EN high → LCD is ready to read data
    LCD_Delay();
    LPC_GPIO1->FIOCLR = EN;   // EN falling edge → LCD latches the nibble
}
```

The nibble is 4 bits (0–15). Each bit is tested and mapped to a specific non-consecutive GPIO pin. The `FIOCLR = D4|D5|D6|D7` at the start ensures all data pins are LOW before setting the new nibble, preventing carry-over from the previous nibble.

The EN pulse is the critical timing event. The HD44780 reads data on the falling edge of EN. The minimum EN pulse width is about 450 ns; the software delay loop provides much more than this, ensuring reliable operation.

### Sending a Full Byte as Command or Data

```c
void LCD_Command(unsigned char cmd)
{
    LPC_GPIO1->FIOCLR = RS;        // RS=0: command mode
    LCD_SendNibble(cmd >> 4);      // Upper nibble first
    LCD_SendNibble(cmd & 0x0F);    // Lower nibble second
    LCD_Delay();
}

void LCD_Data(unsigned char data)
{
    LPC_GPIO1->FIOSET = RS;        // RS=1: data mode
    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);
    LCD_Delay();
}
```

RS=0 means the LCD controller interprets the incoming byte as a command (cursor position, clear, etc.). RS=1 means it interprets it as ASCII character data to display.

`cmd >> 4` extracts the upper 4 bits. For example, for command 0x28 (binary 00101000): upper nibble = 0010 (0x2), lower nibble = 1000 (0x8). The HD44780 receives 0x2 on D7–D4 first, then 0x8.

### Setting Cursor Position

```c
void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;
    if(row == 0)
        address = 0x80 + col;   // Row 0: DDRAM starts at 0x00, cmd = 0x80+offset
    else
        address = 0xC0 + col;   // Row 1: DDRAM starts at 0x40, cmd = 0xC0+offset
    LCD_Command(address);
}
```

The HD44780 DDRAM has row 0 at addresses 0x00–0x0F and row 1 at addresses 0x40–0x4F. The "Set DDRAM Address" command format is 0x80 OR'd with the DDRAM address. For row 0, column 3: command = 0x80 + 3 = 0x83. For row 1, column 0: command = 0x80 + 0x40 + 0 = 0xC0.

## 7. Test Program Explanation

The main.c runs five sequential LCD tests:
1. Simple message: "LCD DRIVER OK" and "TEST START"
2. Clear test: clears and shows "CLEAR WORKING"
3. Cursor movement test: places text at specific column/row positions
4. Character output test: shows "CHAR TEST"
5. Counter loop: displays an incrementing count 000–999

## 8. Hardware Testing Procedure

### Expected Output
LCD displays "LCD DRIVER OK" on row 0 and "TEST START" on row 1. After a delay, cycles through test patterns and ends with a running counter on row 1.

### Init Flow Summary
```
Configure pins P1.0,1,9,10,14,15 as GPIO outputs
      │
Power-on delay (>40ms)
      │
LCD_Command(0x02) — Home (enter 4-bit mode)
      │
LCD_Command(0x28) — 4-bit, 2-line, 5×7 font
      │
LCD_Command(0x0C) — Display ON, cursor off
      │
LCD_Command(0x06) — Entry mode: auto-increment
      │
LCD_Clear() — clear display
      │
LCD Ready
```

### Debugging in Keil
- If LCD shows only dark squares: LCD is powered but not initialized — adjust contrast potentiometer and verify init sequence
- If LCD shows garbage characters: EN pulse too short or delay insufficient
- If nothing visible: verify FIODIR bits for all 6 pins, check connector wiring

## 9. Expected Output

```
Row 0: LCD DRIVER OK
Row 1: TEST START
(after delay)
Row 0: COUNTER:
Row 1: 000
         001
         002
         ...
```
