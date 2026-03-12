# Flash Magic Flashing Guide

This guide covers the complete process of flashing a compiled HEX file to the LPC1768 trainer board using Flash Magic.

---

## Prerequisites

- Keil project built successfully → `.hex` file generated (see `keil_setup.md`)
- FTDI USB-to-Serial drivers installed
- Board connected to PC via USB

---

## Step 1: Install Flash Magic

1. Download from: https://www.flashmagictool.com
2. Run installer with default settings

---

## Step 2: Identify Your COM Port

1. Connect the trainer board to your PC via USB
2. Open **Device Manager** (right-click Start → Device Manager)
3. Expand **Ports (COM & LPT)**
4. Look for: `USB Serial Port (COM3)` or similar
5. Note the COM number (e.g., **COM3**)

> If no port appears, reinstall FTDI drivers.

---

## Step 3: Put the Board into ISP Mode

The LPC1768 has a built-in UART bootloader. To activate it:

1. **Hold down the ISP button** on the trainer board
2. While holding, **press and release RESET**
3. **Release the ISP button**

The board is now in bootloader mode — it will accept a new HEX file.

---

## Step 4: Configure Flash Magic

Open Flash Magic. Fill in each section:

**Step 1 — Communications:**
```
Device:     LPC1768
COM Port:   COM3 (or your port)
Baud Rate:  9600
Interface:  None (ISP)
Oscillator: 12 MHz
```

**Step 2 — Erase:**
```
☑ Erase blocks used by Hex File
```

**Step 3 — Hex File:**
```
Browse → navigate to your project folder → select .hex file
e.g., C:\Projects\uart\uart.hex
```

**Step 4 — Options:**
```
☑ Verify after programming
```

---

## Step 5: Flash

Click **Start**.

Progress bar fills from left to right:
```
Erasing... → Programming... → Verifying... → Finished
```

Flash Magic will show **"Finished"** at the bottom when complete.

---

## Step 6: Run the Program

1. Press **RESET** on the trainer board (do NOT hold ISP)
2. The board exits bootloader and runs your new program

---

## Faster Flashing (Optional)

For faster uploads, use **115200 baud**:
- Set baud to 115200 in Flash Magic
- Ensure no other serial programs are using the COM port

---

## Troubleshooting

| Problem | Solution |
|---------|---------|
| "No device found" | Board not in ISP mode — repeat step 3 |
| "Cannot open COM port" | Close PuTTY or other programs using the port |
| "Verification failed" | Re-flash; try lower baud rate (9600) |
| COM port not visible | Reinstall FTDI VCP drivers |
| Flash Magic crashes | Run as Administrator |
