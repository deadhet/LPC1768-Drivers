# OLED Driver — LPC1768

## 1. Driver Overview

This driver controls a **128×64 pixel OLED display** with an **SSD1306** controller via I2C. The OLED is composed of organic LEDs that emit their own light — no backlight required, providing excellent contrast.

**Real-world applications:** Wearables, IoT dashboards, status indicators, portable instruments, embedded UI.

**Dependencies:** Requires `i2c.c` / `i2c.h` (I2C0 driver) and `oled_font.c` / `oled_font.h` (5×7 character font).

---

## 2. Hardware Interface

| Signal | Port | Pin | I2C Address | Description |
|--------|------|-----|-------------|-------------|
| I2C SDA | PORT0 | P0.27 | 0x78 | OLED data |
| I2C SCL | PORT0 | P0.28 | — | OLED clock |

**Note:** 0x78 = 0x3C (7-bit address) shifted left by 1 (write mode).

---

## 3. OLED Page Architecture

The SSD1306 display memory is organized as **8 pages** of 128 bytes:

```
Page 0:  [128 bytes] ← top 8 rows
Page 1:  [128 bytes]
...
Page 7:  [128 bytes] ← bottom 8 rows

Total: 8 × 128 = 1024 bytes = 128×64 pixels
Each byte = 8 vertical pixels (column-oriented)
```

---

## 4. Driver Architecture

```
OLED_Init()
  └─ Sends 27-byte initialization sequence via I2C
     (sleep off, memory mode, contrast, segment remap,
      scan direction, display offset, clock div, etc.)

OLED_SetCursor(x, page)
  ├─ Write: 0xB0 + page  (page address command)
  ├─ Write: ((x & 0xF0) >> 4) | 0x10  (high column)
  └─ Write: (x & 0x0F) | 0x01         (low column)

OLED_DisplayChar(c)
  └─ Write 5 bytes from Font5x7[c-32][] + 0x00 spacer

OLED_Clear()
  └─ Write 0x00 to all 8 pages × 128 columns
```

---

## 5. Code Walkthrough

```c
static void OLED_WriteCommand(uint8_t cmd)
{
    I2C0_Start();
    I2C0_Write(OLED_I2C_ADDR);  // 0x78 = device address + write bit
    I2C0_Write(0x00);            // Control byte: 0x00 = command follows
    I2C0_Write(cmd);             // The SSD1306 command byte
    I2C0_Stop();
}

static void OLED_WriteData(uint8_t data)
{
    I2C0_Start();
    I2C0_Write(OLED_I2C_ADDR);
    I2C0_Write(0x40);            // Control byte: 0x40 = data follows
    I2C0_Write(data);            // Pixel column data
    I2C0_Stop();
}

void OLED_DisplayChar(char c)
{
    uint8_t i;
    for(i=0; i<5; i++)
    {
        OLED_WriteData(Font5x7[c-32][i]); // 5 column bytes from font table
    }
    OLED_WriteData(0x00); // 1 pixel gap between characters
}
```

---

## 6. Test Program (main.c)

Initializes I2C0, clears OLED, sets cursor to top-left, displays "Hello OLED" text.

---

## 7. Hardware Testing Procedure

### Expected Output
OLED shows "Hello OLED" text on the first page (top-left corner).

### Init Flow Diagram
```
I2C0_Init(100000)
      │
OLED_Init() — 27 I2C command bytes
      │
OLED_Clear() — fills 8×128 = 1024 pixels with 0x00
      │
OLED_SetCursor(0, 0) — top-left
      │
OLED_DisplayString("Hello OLED")
      │
Text visible on OLED
```

### Debugging
- No output: verify I2C address = 0x78 via I2C scanner
- Garbled: verify I2C clock ≤ 400 kHz
- Partial display: check page addressing commands in OLED_SetCursor
