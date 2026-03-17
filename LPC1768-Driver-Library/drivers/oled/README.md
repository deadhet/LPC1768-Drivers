# OLED Driver — LPC1768

## 1. Driver Overview

This driver controls a **128×64 pixel OLED display** with an **SSD1306** controller via the I2C bus. OLED (Organic Light-Emitting Diode) displays emit their own light from individual pixels — unlike LCDs, they require no backlight, giving superior contrast ratios and wider viewing angles.

The driver communicates with the SSD1306 over I2C at address 0x78. Each display pixel is addressed through a page-based memory model: the display is divided into 8 horizontal "pages" of 8 pixels tall each, with each byte representing 8 vertical pixels in a column.

**Dependencies:** This driver requires `i2c.c`/`i2c.h` (the I2C0 driver) and `oled_font.c`/`oled_font.h` (a 5×7 pixel ASCII font table).

**Real-world applications:**
- Wearable device displays
- IoT sensor dashboards
- Status indicators in embedded instruments
- Embedded UI panels in portable equipment

## 2. Hardware Interface

| Signal | Port | Pin | I2C Address | Description |
|--------|------|-----|-------------|-------------|
| I2C SDA | PORT0 | P0.27 | — | OLED data line |
| I2C SCL | PORT0 | P0.28 | — | OLED clock line |
| OLED device | — | — | 0x78 | SSD1306 write address (0x3C << 1) |

**Note:** 0x78 = 0x3C shifted left by 1, which is the 7-bit I2C address with the R/W bit appended as 0 (write). The SSD1306 also supports address 0x7A (for modules with SA0 pin pulled HIGH).

## 3. OLED Page Architecture

The SSD1306's display memory (GDDRAM) has 8 pages. Each page is 128 bytes wide:

```
Page 0:  Byte[0] ... Byte[127]   ← top 8 rows (pixels 0–7)
Page 1:  Byte[0] ... Byte[127]   ← rows 8–15
Page 2:  Byte[0] ... Byte[127]
Page 3:  Byte[0] ... Byte[127]
Page 4:  Byte[0] ... Byte[127]
Page 5:  Byte[0] ... Byte[127]
Page 6:  Byte[0] ... Byte[127]
Page 7:  Byte[0] ... Byte[127]   ← bottom 8 rows (pixels 56–63)
```

Each byte represents 8 vertical pixels in one column. Bit 0 is the topmost pixel of the 8-pixel column, bit 7 is the bottommost. So writing 0x01 to a column turns on only the top pixel; 0xFF turns on all 8 pixels in that column.

## 4. SSD1306 Initialization Sequence

The OLED_Init() function sends 26 configuration commands to bring the SSD1306 from its default powered-down state to an operational display. Key commands:

| Command | Value | Effect |
|---------|-------|--------|
| Display OFF | 0xAE | Turn off display while configuring |
| Memory Mode | 0x20 + 0x10 | Page addressing mode |
| Page Start | 0xB0 | Start at page 0 |
| Scan DIR | 0xC8 | Vertical scan direction (flip) |
| Column Low | 0x00 | Low nibble of column start = 0 |
| Column High | 0x10 | High nibble of column start = 0 |
| Display Offset | 0x40 | Display RAM start line = 0 |
| Contrast | 0x81 + 0x7F | Contrast = 127 (half brightness) |
| Segment Remap | 0xA1 | Column address 127 = SEG0 |
| Normal Display | 0xA6 | Pixel = 1 → lit (not inverted) |
| Multiplex Ratio | 0xA8 + 0x3F | 64-row display (0x3F = 63) |
| All On | 0xA4 | Display from GDDRAM |
| Display Offset | 0xD3 + 0x00 | No vertical offset |
| Clock Div | 0xD5 + 0xF0 | Max clock, no divide |
| Pre-charge | 0xD9 + 0x22 | Pre-charge period |
| COM Pins | 0xDA + 0x12 | Alternative COM config for 128×64 |
| VCOMH Deselect | 0xDB + 0x20 | VCOMH = 0.77 × VCC |
| Charge Pump | 0x8D + 0x14 | Enable internal charge pump |
| Display ON | 0xAF | Turn display on |

## 5. Function Reference

| Function | Description |
|----------|-------------|
| `OLED_Init(void)` | Send 26-byte initialization sequence via I2C |
| `OLED_SetCursor(uint8_t x, uint8_t page)` | Set column x and page address for writing |
| `OLED_Clear(void)` | Fill all 8 pages × 128 columns with 0x00 |
| `OLED_DisplayChar(char c)` | Write 5-column font data + 1-pixel gap |
| `OLED_DisplayString(char *str)` | Display null-terminated string |

## 6. Code Walkthrough

### The I2C Control Byte

```c
static void OLED_WriteCommand(uint8_t cmd)
{
    I2C0_Start();
    I2C0_Write(OLED_I2C_ADDR);   // 0x78 = device address + write bit
    I2C0_Write(0x00);             // Control byte: 0x00 means "command follows"
    I2C0_Write(cmd);              // The command byte
    I2C0_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C0_Start();
    I2C0_Write(OLED_I2C_ADDR);
    I2C0_Write(0x40);             // Control byte: 0x40 means "data follows"
    I2C0_Write(data);             // Pixel column byte
    I2C0_Stop();
}
```

Every I2C transaction to the SSD1306 starts with the device address (0x78), followed by a **control byte**, and then the actual payload. The control byte tells the SSD1306 how to interpret what follows:

- `0x00` = the next byte is a command register write
- `0x40` = the next byte is GDDRAM data (pixel data)

This two-byte header structure is specific to the SSD1306 protocol. Without the correct control byte, the display controller doesn't know whether to store the byte in its configuration registers or in display memory.

### Setting the Cursor Position

```c
void OLED_SetCursor(uint8_t x, uint8_t page)
{
    OLED_WriteCommand(0xB0 + page);                    // Page address command
    OLED_WriteCommand(((x & 0xF0) >> 4) | 0x10);      // High nibble of column
    OLED_WriteCommand((x & 0x0F) | 0x01);              // Low nibble of column
}
```

In page addressing mode, the cursor is set with three commands:
1. `0xB0 + page`: Sets the current page (0–7). `0xB0` is the "Set Page Start Address" command prefix.
2. Column high nibble: `0x10` is the "Set Higher Column Start Address" command prefix. `(x & 0xF0) >> 4` extracts bits [7:4] of x.
3. Column low nibble: `0x00` is the "Set Lower Column Start Address" prefix. `(x & 0x0F)` extracts bits [3:0] of x. The `| 0x01` adds the command prefix.

For example, to set column 32, page 2: send 0xB2 (page 2), 0x12 (high nibble=2), 0x00 (low nibble=0).

### Clearing the Display

```c
void OLED_Clear(void)
{
    uint8_t i, page;
    for(page = 0; page < 8; page++)
    {
        OLED_SetCursor(0, page);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);   // Each 0x00 turns off 8 pixels
        }
    }
}
```

The entire display has 8 × 128 = 1024 bytes of GDDRAM. Writing 0x00 to each byte turns off all pixels. This is 1024 individual I2C transactions. Optimized drivers use a burst write (sending multiple data bytes in a single I2C transaction), but this simple implementation sends one byte at a time.

### Displaying a Character

```c
void OLED_DisplayChar(char c)
{
    uint8_t i;
    for(i = 0; i < 5; i++)
    {
        OLED_WriteData(Font5x7[c - 32][i]);   // 5 column bytes from font array
    }
    OLED_WriteData(0x00);   // 1 column of blank pixels between characters
}
```

Each character in the font table is 5 bytes wide (5 columns). Writing those 5 bytes consecutively to GDDRAM draws the character sprite column by column from left to right. The `-32` subtracts the ASCII offset — the font table starts at space (ASCII 32).

After the 5 character columns, a `0x00` spacer byte creates a 1-pixel gap between adjacent characters, improving readability. Each character thus takes 6 pixels of horizontal space.

## 7. Test Program Explanation

Main.c initializes I2C0 at 100 kHz, calls OLED_Init(), clears the display, positions the cursor at page 0 column 0 (top-left), then calls OLED_DisplayString("Hello OLED"). The string is displayed one character at a time, each character taking 6 pixels wide.

## 8. Hardware Testing Procedure

### Expected Output
OLED displays "Hello OLED" in the top-left corner.

### Init Flow Summary
```
I2C0_Init(100000) — I2C at 100 kHz
      │
OLED_Init() — 26 I2C command bytes (wake, configure, display ON)
      │
OLED_Clear() — 8×128 = 1024 bytes of 0x00
      │
OLED_SetCursor(0, 0) — top-left (column 0, page 0)
      │
OLED_DisplayString("Hello OLED") — draws characters
      │
Text visible on OLED
```

### Debugging
- No output: run an I2C scanner to verify device responds at 0x78
- Garbled display: verify I2C clock is ≤ 400 kHz and SDA/SCL lines have pull-ups
- Partial text: check OLED_SetCursor addresses before writing characters
