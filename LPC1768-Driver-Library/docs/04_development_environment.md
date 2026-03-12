# Document 04 — Development Environment Setup

This document provides a complete, step-by-step guide to setting up the development environment for LPC1768 driver development. Follow every step in order.

---

## Required Software

| Software | Version | Download | Purpose |
|---------|---------|---------|---------|
| Keil µVision | 4 or 5 | [keil.com/mdk5](https://www.keil.com/download/product/) | IDE — compile, build, debug |
| Flash Magic | Latest | [flashmagictool.com](https://www.flashmagictool.com) | Flash HEX to board via UART |
| FTDI Drivers | 2.12.36 | [ftdichip.com/drivers](https://ftdichip.com/drivers/vcp-drivers/) | USB-to-Serial COM port |
| PuTTY | 0.79+ | [putty.org](https://putty.org) | Serial terminal for UART testing |

---

## Part 1 — Installing Keil µVision

### Step 1: Download Keil MDK
1. Go to [keil.com/mdk5](https://www.keil.com/download/product/)
2. Download **MDK-ARM** (free evaluation supports up to 32KB code — sufficient for these drivers)
3. Run the installer with default options

### Step 2: Install LPC1768 Device Pack
1. Open Keil µVision
2. Go to **Project → Manage → Pack Installer**
3. In the search bar, search for `LPC1768`
4. Find **NXP LPC17xx Device Support** package
5. Click **Install**

> If using Keil MDK 4, the LPC1768 support is installed by default during the IDE setup.

---

## Part 2 — Creating a New LPC1768 Project

### Step 1: Create Project
1. Open Keil µVision
2. **Project → New µVision Project**
3. Create a folder, e.g., `C:\Projects\my_driver\`
4. Name the project file (e.g., `gpio.uvproj`)
5. Click **Save**

### Step 2: Select Device
1. In the device selection dialog:
   - Vendor: **NXP (founded by Philips)**
   - Device: **LPC1768**
2. Click **OK**

### Step 3: Add Startup Files (CMSIS)
Keil will prompt to add runtime files. Click **Yes** to add:
- `startup_LPC17xx.s` — Reset handler, vector table, stack/heap
- `system_LPC17xx.c` — SystemInit(), PLL and clock setup

> These files are also available in the `common/` folder of this repository.

### Step 4: Configure Project Settings
1. **Project → Options for Target**
2. **Target tab**:
   - Xtal (MHz): `12.0`
   - IRAM1 Size: `0x10000` (64 KB)
   - IROM1 Size: `0x80000` (512 KB)
3. **Output tab**:
   - Check ✅ **Create HEX File**
   - Name: e.g., `gpio`
4. **C/C++ tab**:
   - Optimization: **Level 1 (-O1)**
   - Define: `__USE_CMSIS`
   - Include Paths: add your project folder

### Step 5: Add Source Files
1. In the **Project** pane, right-click **Source Group 1 → Add Existing Files**
2. Add:
   - `driver.c` (e.g., `gpio.c`)
   - `driver.h`
   - `main.c`
   - `startup_LPC17xx.s`
   - `system_LPC17xx.c`

---

## Part 3 — Building the Project

### Step 1: Build
Press **F7** or **Project → Build Target**

### Step 2: Check Build Output
The **Build Output** window at the bottom should show:
```
Build target 'Target 1'
linking...
Program Size: Code=XXXX RO-data=XXX RW-data=XX ZI-data=XXXX
FromELF: creating hex file...
"gpio.hex" - 0 Error(s), 0 Warning(s).
```

- ✅ **0 Errors** — you can flash
- ⚠️ Warnings are acceptable but fix them for production code
- ❌ Errors — fix before proceeding

### Step 3: Locate the HEX File
The `.hex` file is generated in your project folder:
```
C:\Projects\my_driver\gpio.hex
```

---

## Part 4 — Installing FTDI USB-to-Serial Drivers

1. Download **FTDI Virtual COM Port driver** from [ftdichip.com](https://ftdichip.com/drivers/vcp-drivers/)
2. Run the installer
3. Connect the trainer board via USB
4. Open **Device Manager** → expand **Ports (COM & LPT)**
5. Note the COM port number (e.g., `COM3`)

---

## Part 5 — Flashing with Flash Magic

### Step 1: Install Flash Magic
1. Download from [flashmagictool.com](https://www.flashmagictool.com)
2. Install with default options

### Step 2: Configure Flash Magic

Open Flash Magic and configure:

| Setting | Value |
|---------|-------|
| Device | LPC1768 |
| COM Port | Your COM port (e.g., COM3) |
| Baud Rate | 9600 (use 115200 for faster flashing) |
| Interface | None (ISP) |
| Oscillator | 12 MHz |

### Step 3: Prepare Board for ISP Mode

The LPC1768 enters ISP (In-System Programming) mode when:
- Pin P2.10 (ISP button) is held LOW during reset

On the RDL trainer kit:
1. **Hold down the ISP button**
2. **Press and release RESET**
3. **Release the ISP button**

The board is now in bootloader mode.

### Step 4: Flash the HEX

1. In Flash Magic:
   - Section 1 (Communications): verify COM port and baud rate
   - Section 2 (Erase): check ✅ **Erase blocks used by HEX file**
   - Section 3 (Hex File): **Browse** → select your `.hex` file
   - Section 4 (Options): check ✅ **Verify after programming**
2. Click **Start**
3. Progress bar fills — wait for "Finished" message

### Step 5: Run the Program

1. Press **RESET** on the trainer board (without holding ISP)
2. The board exits bootloader mode and runs your program

---

## Part 6 — Serial Terminal (PuTTY)

For drivers that output via UART (ADC, I2C, RTC, UART):

### PuTTY Configuration

1. Open PuTTY
2. Connection type: **Serial**
3. Serial line: `COM3` (your port)
4. Speed: **9600**
5. Click **Open**

### Serial Settings Verification

In PuTTY → Serial settings:
- Data bits: **8**
- Stop bits: **1**
- Parity: **None**
- Flow control: **None**

---

## Part 7 — Debugging in Keil (Optional)

For debugging without hardware (simulation):
1. **Project → Options for Target → Debug tab**
2. Select: **Use Simulator** (left side)
3. Press **Debug → Start/Stop Debug Session** (Ctrl+F5)
4. Use Register window, Watch, Memory to inspect state

For debugging with hardware (JTAG/SWD):
1. Connect ULINK2 or J-Link to the 20-pin JTAG header
2. Select the appropriate debug adapter in Options → Debug
3. Press Ctrl+F5 to download and start debug session

See [`tools/keil_debugging.md`](../tools/keil_debugging.md) for detailed debug instructions.

---

## Common Issues & Solutions

| Problem | Cause | Solution |
|---------|-------|---------|
| "No device found" in Flash Magic | Board not in ISP mode | Hold ISP button, press RESET |
| Wrong COM port | Multiple FTDI devices | Check Device Manager for correct COM |
| HEX file not generated | "Create HEX" not checked | Enable in Project Options → Output |
| Build error: undefined symbol | Missing source file | Add all .c files to project |
| 0 output on serial terminal | Wrong baud rate | Match PuTTY baud to UART0_Init() value |
