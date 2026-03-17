# Keypad Driver — LPC1768

## 1. Driver Overview

This driver scans a **4×4 matrix keypad** using the row-scan column-detect technique. In a matrix keypad, 16 keys are arranged in a 4×4 grid with only 8 wires — 4 for rows and 4 for columns. Each key connects one row wire to one column wire. The driver determines which key is pressed by driving one row LOW at a time and checking which column goes LOW.

The driver is implemented using a hardware abstraction structure (`KEYPAD_Handle_t`) that holds the port pointers and pin numbers. This makes the driver portable — changing the wiring only requires updating the handle configuration, not the driver logic.

**Real-world applications:**
- Security PIN entry panels
- Menu navigation for embedded UIs
- Numeric data entry for industrial equipment
- ATM-style keypads

**Dependencies:** Requires `lcd.c`/`lcd.h` in the test program to display pressed keys.

## 2. Hardware Interface

The 4×4 keypad has 8 physical connector wires: rows R0–R3 and columns C0–C3.

```
Key layout:           Wiring principle:
┌──┬──┬──┬──┐        Rows (output): drive LOW one at a time
│ 1│ 2│ 3│ 4│        Cols (input):  read which column goes LOW
├──┼──┼──┼──┤
│ 5│ 6│ 7│ 8│        Pressing key at [row=1, col=2] connects
├──┼──┼──┼──┤        row 1 to column 2.
│ 9│ 0│ A│ B│
├──┼──┼──┼──┤        keyMap[row][col] = proper character
│ C│ D│ E│ F│
└──┴──┴──┴──┘
```

Pin assignments are passed via the `KEYPAD_Handle_t` struct in main.c. Columns use the MCU's internal pull-up logic: when no button is pressed, columns float HIGH. Pressing a button when the row is LOW pulls the column pin LOW.

## 3. Driver Architecture — Handle-Based Design

The driver avoids hardcoded pin numbers. Instead, a struct carries all hardware configuration:

```c
typedef struct {
    LPC_GPIO_TypeDef *rowPort;   // GPIO port for row lines
    LPC_GPIO_TypeDef *colPort;   // GPIO port for column lines
    uint8_t rowPins[4];          // Pin numbers for rows 0-3
    uint8_t colPins[4];          // Pin numbers for cols 0-3
} KEYPAD_Handle_t;
```

**Scan Algorithm:**
```
For row = 0 to 3:
  1. Drive ALL rows HIGH (deselect all)
  2. Drive only current row LOW
  3. Wait (debounce settle)
  4. For col = 0 to 3:
     Read column pin: if LOW → key pressed at [row][col]
       → Wait again (debounce)
       → Wait until col goes HIGH (key released)
       → Return keyMap[row][col]
Return 0 (no key pressed)
```

## 4. Function Reference

### `Keypad_Init(KEYPAD_Handle_t *keypad)`
Configures row pins as outputs (default HIGH) and column pins as inputs.

### `Keypad_GetKey(KEYPAD_Handle_t *keypad)`
Performs one complete scan of the 4×4 matrix. Returns the pressed key character, or 0 if no key is pressed.

## 5. Code Walkthrough

### Initialization

```c
void Keypad_Init(KEYPAD_Handle_t *keypad)
{
    // Configure rows as output
    for(i = 0; i < 4; i++)
    {
        keypad->rowPort->FIODIR |= (1 << keypad->rowPins[i]);   // Row = output
        keypad->rowPort->FIOSET |= (1 << keypad->rowPins[i]);   // Default HIGH
    }

    // Configure columns as input
    for(i = 0; i < 4; i++)
    {
        keypad->colPort->FIODIR &= ~(1 << keypad->colPins[i]);  // Col = input
    }
}
```

For each row pin, `FIODIR |= (1 << rowPins[i])` sets that bit to 1 (output direction). Then `FIOSET = (1 << rowPins[i])` drives it HIGH as the idle state. For column pins, `FIODIR &= ~(1 << colPins[i])` clears the direction bit to 0, making it an input. As inputs, the column pins read the logic level from the keypad matrix.

No PCONP is needed because GPIO is always enabled. No PINSEL changes needed if the pins default to GPIO function.

### The Scan Loop — Row Selection

```c
for(row = 0; row < 4; row++)
{
    // Drive all rows HIGH first
    for(i = 0; i < 4; i++)
        keypad->rowPort->FIOSET = (1 << keypad->rowPins[i]);

    // Drive only this row LOW
    keypad->rowPort->FIOCLR = (1 << keypad->rowPins[row]);

    Keypad_Delay();  // Let signal settle before reading columns
```

In each iteration, all 4 row lines are driven HIGH first using FIOSET. This ensures no stray LOW from a previous iteration affects the new scan. Then only the current row is pulled LOW via FIOCLR. Any key in that row that is pressed will now create a LOW signal on the corresponding column pin (because the key physically connects the row wire to the column wire).

The `Keypad_Delay()` gives the signal time to settle through the physical wires and any capacitance on the lines before the column is read.

### Column Detection and Debouncing

```c
    for(col = 0; col < 4; col++)
    {
        if (!(keypad->colPort->FIOPIN & (1 << keypad->colPins[col])))
        {
            Keypad_Delay();   // Debounce — wait out the contact bounce

            // Wait until key is released
            while(!(keypad->colPort->FIOPIN & (1 << keypad->colPins[col])));

            return keyMap[row][col];
        }
    }
```

Reading FIOPIN returns the current state of all pins on the column port. The bit mask `(1 << keypad->colPins[col])` isolates the specific column pin. The `!` inverts the test: if the bit is 0 (LOW), the condition is true — a key is pressed.

The first `Keypad_Delay()` is the debounce delay. Mechanical key contacts bounce for ~5–20 ms when pressed. Without debouncing, the key release detection might trigger on a transient bounce rather than the genuine release.

The `while(!(...))` waits until the column pin goes HIGH again — meaning the key has been physically released. This prevents a single key press from registering multiple times in the calling loop.

Finally, `keyMap[row][col]` returns the character corresponding to that row/column combination from the pre-defined 2D array.

### The Key Map

```c
static const char keyMap[4][4] =
{
    {'1','2','3','4'},
    {'5','6','7','8'},
    {'9','0','A','B'},
    {'C','D','E','F'}
};
```

This table maps (row, col) coordinates to ASCII character values. `keyMap[0][0] = '1'`, `keyMap[2][1] = '0'`, `keyMap[3][3] = 'F'`, etc. The values can be changed to any character mapping needed by the application.

## 6. Test Program Explanation

Main.c configures the keypad handle with row pins and column pins assigned to PORT1 and PORT2. After `LCD_Init()` and `Keypad_Init()`, the main loop calls `Keypad_GetKey()` and displays the result on the LCD when a key is pressed.

## 7. Hardware Testing Procedure

### Expected Output
LCD row 0 shows "Key Pressed:". LCD row 1 shows the character of the most recently pressed key (1–9, 0, A–F).

### Init Flow Summary
```
Configure KEYPAD_Handle_t with row/col ports and pins
      │
Keypad_Init() — rows=output+HIGH, cols=input
      │
LCD_Init() — LCD ready
      │
Loop: Keypad_GetKey()
  If key != 0 → LCD_SetCursor(1,0) → LCD_Data(key)
```

### Debugging in Keil
- **Watch FIOPIN of column port:** bits for connected col pins go LOW when a key is pressed
- **Watch key variable in main:** should show the ASCII value of the pressed key
- **If no key detected:** verify row pins are configured as outputs, col pins as inputs
- **If multiple keys detected:** increase debounce delay in `Keypad_Delay()`

## 8. Expected Output

```
LCD Row 0: Key Pressed:
LCD Row 1: (pressed key character — e.g., '5', 'A', '0')
```
