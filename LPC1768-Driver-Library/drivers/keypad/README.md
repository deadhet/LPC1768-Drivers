# Keypad Driver — LPC1768

## 1. Driver Overview

This driver scans a **4×4 matrix keypad** using the row-scan column-detect method. The driver uses a configurable handle struct, making it hardware-independent.

**Real-world applications:** Security PINs, menu navigation, numeric data entry, ATM interfaces.

**Dependencies:** Requires `lcd.c` / `lcd.h` — pressed key is displayed on LCD.

---

## 2. Hardware Interface

The 4×4 keypad has 8 wires: 4 rows (output) + 4 columns (input).

```
Key Layout:        Wiring:
┌────┬────┬────┬────┐   Rows (output): drive LOW one at a time
│ 1  │ 2  │ 3  │ 4  │   Cols (input):  read which col is LOW
├────┼────┼────┼────┤
│ 5  │ 6  │ 7  │ 8  │   keyMap[row][col]:
├────┼────┼────┼────┤   {1,2,3,4} {5,6,7,8} {9,0,A,B} {C,D,E,F}
│ 9  │ 0  │ A  │ B  │
├────┼────┼────┼────┤
│ C  │ D  │ E  │ F  │
└────┴────┴────┴────┘
```

---

## 3. Driver Architecture (Handle-Based)

The `KEYPAD_Handle_t` struct holds port and pin assignments, making the driver portable:

```c
typedef struct {
    LPC_GPIO_TypeDef *rowPort;  // Port for row lines
    LPC_GPIO_TypeDef *colPort;  // Port for column lines
    uint8_t rowPins[4];         // Pin numbers for 4 rows
    uint8_t colPins[4];         // Pin numbers for 4 columns
} KEYPAD_Handle_t;
```

**Initialization:** Rows → FIODIR = output, default HIGH. Columns → FIODIR = input.

**Scan Algorithm:**
```
For row = 0 to 3:
  1. Drive ALL rows HIGH
  2. Drive current row LOW only
  3. delay (debounce settle)
  4. For col = 0 to 3:
     If colPort FIOPIN bit = 0 (LOW):
       → Key at [row][col] pressed!
       → debounce delay
       → wait until col goes HIGH (key released)
       → return keyMap[row][col]
Return 0 (no key pressed)
```

---

## 4. Code Walkthrough

```c
char Keypad_GetKey(KEYPAD_Handle_t *keypad)
{
    for(row=0; row<4; row++)
    {
        // Step 1: All rows HIGH (deselect all)
        for(i=0;i<4;i++)
            keypad->rowPort->FIOSET = (1 << keypad->rowPins[i]);

        // Step 2: Drive this row LOW (select it)
        keypad->rowPort->FIOCLR = (1 << keypad->rowPins[row]);

        Keypad_Delay();  // Let signal settle

        // Step 3: Read all columns
        for(col=0; col<4; col++)
        {
            if(!(keypad->colPort->FIOPIN & (1 << keypad->colPins[col])))
            {
                // Column is LOW → key pressed at [row][col]
                Keypad_Delay();  // Debounce: wait again
                while(!(keypad->colPort->FIOPIN & (1 << keypad->colPins[col])));
                // Wait for key release before returning
                return keyMap[row][col];
            }
        }
    }
    return 0; // No key pressed this scan
}
```

---

## 5. Hardware Testing Procedure

### Expected Output
LCD row 0 shows "Key Pressed:". LCD row 1 shows the character of the pressed key (1–9, 0, A–F).

### Init Flow Diagram
```
KEYPAD_Handle_t configured with row/col ports and pins
      │
Keypad_Init() — rows=output+HIGH, cols=input
      │
LCD_Init() — LCD ready
      │
Loop: Keypad_GetKey()
  If key != 0 → LCD_SetCursor(1,0) → LCD_Data(key)
```
