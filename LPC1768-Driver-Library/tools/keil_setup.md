# Keil µVision Setup Guide

This guide walks through the complete installation and configuration of Keil µVision for LPC1768 development. Follow every step in order for a successful setup.

---

## Step 1: Download Keil MDK

1. Visit: https://www.keil.com/download/product/
2. Click **MDK-ARM** → fill in the form to get the download link
3. Download **MDK5** (recommended) or **MDK4** if your instructor specifies it
4. The installer is approximately 900 MB

> **Free Evaluation Limit:** MDK Lite (free) limits code to 32KB. All drivers in this repository compile well within 32KB individually.

---

## Step 2: Install Keil MDK

1. Run the downloaded installer (e.g., `MDK532.exe`)
2. Accept License Agreement
3. Install to default path: `C:\Keil_v5\`
4. Complete the installation
5. Do NOT install the Keil µVision 5 Pack installer separately — it opens automatically

---

## Step 3: Install LPC1768 Device Support

After first launch, the **Pack Installer** opens:

1. In Pack Installer: search for `LPC17`
2. Find: **NXP::LPC17xx_DFP** (Device Family Pack)
3. Click **Install** next to the latest version
4. Wait for download and installation to complete
5. Close Pack Installer

---

## Step 4: Create a New LPC1768 Project

1. **File → New µVision Project**
2. Browse to your desired project folder
3. Name the project (e.g., `uart`)
4. Click **Save**
5. Device selection dialog appears:
   - Filter: type `LPC1768`
   - Select: **NXP → LPC1768**
   - Click **OK**
6. "Manage Run-Time Environment" dialog:
   - Check ✅ **CMSIS → CORE**
   - Check ✅ **Device → Startup**
   - Click **OK**

---

## Step 5: Project Settings

1. **Project → Options for Target 'Target 1'** (Alt+F7)

**Target tab:**
```
Xtal (MHz):   12.0
IRAM1 Start:  0x10000000   Size: 0x10000 (64KB)
IROM1 Start:  0x00000000   Size: 0x80000 (512KB)
```

**Output tab:**
```
☑ Create HEX File
Name of Executable: (project name, e.g., uart)
```

**C/C++ tab:**
```
Optimization: Level 1 (-O1)
Define: __USE_CMSIS
Include Paths: .\  (add your project folder)
```

**Debug tab** (for simulation):
```
☑ Use Simulator
```

---

## Step 6: Add Source Files

Right-click **Source Group 1** in Project pane → **Add Existing Files to Group**

Add these files from your project folder:
- `main.c`
- `(driver).c`  (e.g., `uart.c`)
- `startup_LPC17xx.s`  (from `common/` or project folder)
- `system_LPC17xx.c`  (from `common/` or project folder)

---

## Step 7: Build and Verify

Press **F7** to build.

Expected output in Build window:
```
Build target 'Target 1'
assembling startup_LPC17xx.s...
compiling uart.c...
compiling main.c...
linking...
Program Size: Code=XXXX RO-data=XX RW-data=XX ZI-data=XXXX
FromELF: creating hex file...
"uart.hex" - 0 Error(s), 0 Warning(s).
```

The `.hex` file is now in your project directory, ready to flash.
