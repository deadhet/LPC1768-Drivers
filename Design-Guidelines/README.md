This README provides a comprehensive overview of the LPC1768 system design, detailing the register architecture and programming guidelines. It is intended to assist developers in scaling their projects and building application-specific drivers on the LPC1768 platform.

# 🧩 LPC1768 Core Architecture

- ARM Cortex-M3 @ **100 MHz**
- Harvard architecture (separate instruction/data buses)
- NVIC (hardware interrupt controller)
- Flash: up to **512 KB**
- SRAM: **64 KB**

👉 Key insight:

- Everything is **memory mapped**
- No “special function registers” like 8051 — just addresses

# Peripheral Overview (VERY IMPORTANT)
## Communication
- 4 × UART
- 3 × I2C
- 2 × SSP (SPI-like)
- 1 × SPI
- CAN (2 channels)
- USB + Ethernet
## Analog
- 8-channel 12-bit ADC
- 10-bit DAC
## Timers
- **4 General-purpose timers**
- 1 PWM (6 outputs)
- 1 Motor PWM
## Others
- RTC (battery backed)
- Watchdog Timer
- DMA (8 channels)

# System Control

## PCONP
The **Power Control for Peripherals (PCONP)** register (located at address `0x400F C0C4`) allows you to turn off individual peripheral functions to save power by gating off their clock sources.

Each bit in the 32-bit PCONP register controls the power and clock for a specific peripheral. **Writing a** **1** **to a bit enables that peripheral, while writing a** **0** **disables (gates off) its clock**. Importantly, a valid read or write to a peripheral's dedicated register is only possible if that peripheral is currently enabled in the PCONP register.

Here is the specific bit-level structure of the PCONP register:

- **Bit 1 (PCTIM0):** Timer/Counter 0
- **Bit 2 (PCTIM1):** Timer/Counter 1
- **Bit 3 (PCUART0):** UART0
- **Bit 4 (PCUART1):** UART1
- **Bit 6 (PCPWM1):** PWM1
- **Bit 7 (PCI2C0):** I2C0 interface
- **Bit 8 (PCSPI):** SPI interface
- **Bit 9 (PCRTC):** RTC
- **Bit 10 (PCSSP1):** SSP1 interface
- **Bit 12 (PCADC):** A/D converter (ADC)
- **Bit 13 (PCCAN1):** CAN Controller 1
- **Bit 14 (PCCAN2):** CAN Controller 2
- **Bit 15 (PCGPIO):** IOCON, GPIO, and GPIO interrupts
- **Bit 16 (PCRIT):** Repetitive Interrupt Timer
- **Bit 17 (PCMCPWM):** Motor Control PWM
- **Bit 18 (PCQEI):** Quadrature Encoder Interface
- **Bit 19 (PCI2C1):** I2C1 interface
- **Bit 21 (PCSSP0):** SSP0 interface
- **Bit 22 (PCTIM2):** Timer 2
- **Bit 23 (PCTIM3):** Timer 3
- **Bit 24 (PCUART2):** UART 2
- **Bit 25 (PCUART3):** UART 3
- **Bit 26 (PCI2C2):** I2C2 interface
- **Bit 27 (PCI2S):** I2S interface
- **Bit 29 (PCGPDMA):** GPDMA function
- **Bit 30 (PCENET):** Ethernet block
- **Bit 31 (PCUSB):** USB interface

**Bits 0, 5, 11, 20, and 28** are marked as Reserved and should be cleared to `0`.

**Key Usage Notes:**

- **Default State:** Upon reset, the PCONP register contains a default value (`0x03BE`) that leaves several interfaces enabled. For power-saving oriented systems, you should configure this register so that only the peripherals actively used by your application are set to `1`, while all unused and reserved bits are cleared to `0`.
- **Exceptions:** A few peripheral functions cannot be turned off through this register, such as the Watchdog timer, the Pin Connect block, and the System Control block. Additionally, the Digital-to-Analog Converter (DAC) does not have a control bit in the PCONP register; it is enabled instead by configuring the PINSEL1 register for its corresponding pin.
- **ADC Specifics:** When controlling the ADC, you must clear the power-down (PDN) bit in the AD0CR register before clearing the PCADC bit in PCONP, and conversely, set the PCADC bit before setting the PDN bit.

## PCLKSEL0 / PCLKSEL1
The **Peripheral Clock Selection registers (PCLKSEL0 and PCLKSEL1)** control the rate of the clock signal (PCLK) supplied to individual peripherals on the microcontroller.

Each peripheral is assigned a **2-bit field** in either PCLKSEL0 or PCLKSEL1, which determines the peripheral's clock rate relative to the main CPU clock (CCLK).

Before diving into the specific bits, here are the universal 2-bit values used to set the clock rate for almost all peripherals:

- **00**: PCLK_peripheral = CCLK / 4 (This is the default reset value)
- **01**: PCLK_peripheral = CCLK
- **10**: PCLK_peripheral = CCLK / 2
- **11**: PCLK_peripheral = CCLK / 8 _(Exception: For CAN1, CAN2, and CAN filtering, setting "11" selects CCLK / 6 instead)_.

Here is the detailed bit-by-bit breakdown for both registers:

**PCLKSEL0 (Address** **0x400F C1A8**)

This register selects the clock for the following peripherals:

- **Bits 1:0 (PCLK_WDT):** Watchdog Timer (WDT)
- **Bits 3:2 (PCLK_TIMER0):** Timer 0
- **Bits 5:4 (PCLK_TIMER1):** Timer 1
- **Bits 7:6 (PCLK_UART0):** UART0
- **Bits 9:8 (PCLK_UART1):** UART1
- **Bits 11:10:** Reserved
- **Bits 13:12 (PCLK_PWM1):** PWM1
- **Bits 15:14 (PCLK_I2C0):** I2C0
- **Bits 17:16 (PCLK_SPI):** SPI
- **Bits 19:18:** Reserved
- **Bits 21:20 (PCLK_SSP1):** SSP1
- **Bits 23:22 (PCLK_DAC):** DAC
- **Bits 25:24 (PCLK_ADC):** ADC
- **Bits 27:26 (PCLK_CAN1):** CAN1
- **Bits 29:28 (PCLK_CAN2):** CAN2
- **Bits 31:30 (PCLK_ACF):** CAN acceptance filtering

**PCLKSEL1 (Address** **0x400F C1AC**)

This register selects the clock for the remaining peripherals:

- **Bits 1:0 (PCLK_QEI):** Quadrature Encoder Interface (QEI)
- **Bits 3:2 (PCLK_GPIOINT):** GPIO interrupts
- **Bits 5:4 (PCLK_PCB):** Pin Connect Block
- **Bits 7:6 (PCLK_I2C1):** I2C1
- **Bits 9:8:** Reserved
- **Bits 11:10 (PCLK_SSP0):** SSP0
- **Bits 13:12 (PCLK_TIMER2):** Timer 2
- **Bits 15:14 (PCLK_TIMER3):** Timer 3
- **Bits 17:16 (PCLK_UART2):** UART2
- **Bits 19:18 (PCLK_UART3):** UART3
- **Bits 21:20 (PCLK_I2C2):** I2C2
- **Bits 23:22 (PCLK_I2S):** I2S
- **Bits 25:24:** Reserved
- **Bits 27:26 (PCLK_RIT):** Repetitive Interrupt Timer
- **Bits 29:28 (PCLK_SYSCON):** System Control block
- **Bits 31:30 (PCLK_MC):** Motor Control PWM

**Key Usage Notes:**

- **CAN Configuration:** When the CAN function is used, **PCLK_CAN1 and PCLK_CAN2 must be set to the exact same PCLK divide value**.
- **RTC Exception:** The peripheral clock for the Real-Time Clock (RTC) block is fixed at CCLK / 8 and is not controlled by the PCLKSEL registers.

## PLL0

The **Main Phase Locked Loop (PLL0)** is responsible for taking a lower input clock frequency and multiplying it up to a high frequency, which is then divided down to provide the actual system clock (CCLK) used by the CPU, on-chip peripherals, and optionally the USB subsystem.

It allows the microcontroller to operate at its maximum allowed CPU rate (up to 120 MHz on high-speed versions like the LPC1769, or 100 MHz on other versions) without requiring a high-frequency external crystal.

**Input Sources and Frequency Range:**

- PLL0 can use the Internal RC (IRC) oscillator, the main external oscillator, or the 32 kHz RTC oscillator as its input clock source (FIN).
- The input frequency must be in the range of 32 kHz to 50 MHz. _(Note: The datasheet specifies a maximum input of 25 MHz for standard configurations__)._

**Internal Math and Operation:** The operation of PLL0 relies on a pre-divider (`N`) and a multiplier (`M`) connected to a Current Controlled Oscillator (CCO):

- The input frequency is first divided down by the **N** value, which can range from 1 to 256.
- The divided clock is then multiplied by the **M** value (typically ranging from 6 to 512).
- The frequency of the CCO (FCCO) is calculated using the formula: **FCCO = (2 × M × FIN) / N**.
- The resulting FCCO must strictly fall within the frequency range of **275 MHz to 550 MHz**. After the CCO step, the clock is divided down again by the CPU Clock Configuration register (`CCLKCFG`) to set the final CPU clock speed.

**Control Registers and The Feed Sequence:** Because the entire microcontroller operation depends on the system clock, PLL0 is heavily protected against accidental alteration.

- **PLL0CFG:** Holds the `M` and `N` multiplier and divider configuration values.
- **PLL0CON:** Contains the bits to _enable_ and _connect_ the PLL.
- **PLL0FEED:** Changes written to `PLL0CFG` or `PLL0CON` do **not** take effect immediately. They only become active after a strict feed sequence is written to the `PLL0FEED` register. This sequence consists of writing `0xAA` followed directly by `0x55` to the `PLL0FEED` register, with no other register accesses in that address space occurring in between.

**Setup Sequence and Locking:** Upon chip reset or entering Power-down mode, PLL0 is automatically turned off and bypassed to save power. To set it up, you must follow a strict software sequence:

1. Disconnect and disable PLL0 (using the feed sequence).
2. Select the clock source and write the new `M` and `N` values to `PLL0CFG` (making it effective with a feed sequence).
3. Enable PLL0 (with a feed sequence).
4. Wait for PLL0 to lock onto the target frequency. Software determines this by monitoring the **PLOCK0** bit in the **PLL0STAT** register, or by utilizing the `PLOCK0` interrupt.
5. Once locked, connect PLL0 (with a final feed sequence) so it takes over driving the CPU.
## TCR – Timer Control Register

The **Timer Control Register (TCR)** is used to control the operation of the Timer/Counter functions on the microcontroller, specifically allowing you to enable, disable, or reset the counters.

Within the LPC17xx architecture, there are specific TCRs for both the standard General Purpose Timers and the Pulse Width Modulator (PWM).

**General Purpose Timers 0, 1, 2, and 3 (T0TCR, T1TCR, T2TCR, T3TCR)** These registers are located at addresses `0x4000 4004` (Timer 0), `0x4000 8004` (Timer 1), `0x4009 0004` (Timer 2), and `0x4009 4004` (Timer 3). The standard TCR consists of two active control bits:

- **Bit 0 (Counter Enable):** Controls whether the Timer Counter and Prescale Counter are enabled for counting or disabled.
- **Bit 1 (Counter Reset):** When set to `1`, the Timer Counter and the Prescale Counter are **synchronously reset on the next positive edge of PCLK**. Both counters will remain in a reset state until this bit is returned to `0`.

**PWM Timer Control Register (PWM1TCR)** The PWM block utilizes a slightly expanded TCR located at address `0x4001 8004`. In addition to the standard timer controls, it includes a bit specifically for enabling PWM functionality:

- **Bit 0 (Counter Enable):** When set to `1`, the PWM Timer Counter and PWM Prescale Counter are enabled for counting; when `0`, they are disabled.
- **Bit 1 (Counter Reset):** When set to `1`, synchronously resets both PWM counters on the next positive edge of PCLK, holding them in reset until the bit is cleared to `0`.
- **Bit 3 (PWM Enable):** When set to `1`, **PWM mode is enabled and the counter resets to 1**. Enabling this mode causes the shadow registers to operate in connection with the Match registers. When this bit is `0`, standard Timer mode is enabled and the counter resets to 0. _Note: To ensure proper operation, PWM Match Register 0 (which determines the PWM rate) must be set up prior to enabling the PWM mode, otherwise a match event will not occur to make the shadow register contents effective_.
## TC – Timer Counter

Following up on the Timer Control Register (TCR), the **Timer Counter (TC)** is the actual 32-bit counting register that the TCR controls. It serves as the core timing mechanism for the General Purpose Timers, the standard Pulse Width Modulator (PWM), and the Motor Control PWM (MCPWM).

Here is how the Timer Counter operates and interacts with the rest of the microcontroller:

**How the TC Increments**

- **In Timer Mode:** The TC does not simply increment on every single pulse of the peripheral clock (PCLK). Instead, its counting rate is scaled by a Prescale Register (PR) and Prescale Counter (PC). The TC is incremented **every** **PR + 1** **cycles of PCLK**. For example, if the PR is set to 0, the TC increments on every PCLK; if the PR is set to 1, it increments every 2 PCLKs, and so on.
- **In Counter Mode:** Instead of using the internal clock, the TC increments based on external events. You can configure the timer to increment the TC on rising edges, falling edges, or both edges of an external signal applied to a selected Capture (CAP) input pin.

**Counting Range and Wrapping** Because it is a 32-bit register, the general-purpose TC counts all the way up to its maximum limit of `0xFFFF FFFF` and then wraps back around to `0x0000 0000`. **This wrap-around event does not automatically generate an interrupt**. If your application needs to trigger an action upon overflow, you must specifically configure a Match register to detect the upper limit.

**Interaction with Other Registers** The value inside the TC is constantly being monitored and used by other parts of the timer block:

- **Match Registers (MR):** The TC is continuously compared against up to four Match registers. When the TC value exactly equals a Match register value, the hardware can automatically trigger an interrupt, reset the TC back to zero, stop the TC from counting, or toggle an external output pin.
- **Capture Registers (CR):** When an external signal transitions on a designated capture pin, the hardware takes an immediate "snapshot" of the current TC value and stores it in a Capture register. This allows you to accurately measure the time between external events (like a pulse width).

**Motor Control PWM (MCPWM) Specifics** The Motor Control PWM block has three independent channels, each featuring its own dedicated TC (`MCTC0`, `MCTC1`, and `MCTC2`). Instead of counting up to `0xFFFF FFFF`, these specific TCs count up to the value stored in their corresponding Limit register (LIM). Additionally, if you need to explicitly write a value to an MCTC register using software, **you can only do so when that specific channel is stopped**; writes to a running MCTC are ignored.

## PR – Prescale Register

The **Prescale Register (PR)** is a 32-bit register that determines the maximum value for the timer's Prescale Counter (PC). Its primary function is to act as a programmable clock divider, scaling down the peripheral clock (PCLK) before it is applied to the main Timer Counter (TC).

**How It Works:** The Prescale Counter (PC) increments on every single cycle of the peripheral clock. When the value in the PC matches the value you have stored in the Prescale Register (PR), the following occurs on the very next PCLK cycle:

- The **Timer Counter (TC) is incremented** by 1.
- The **Prescale Counter (PC) is reset** back to zero to begin counting again.

**Practical Effect on Timing:** By configuring the PR, you can control the trade-off between the resolution of your timer and the maximum amount of time that can pass before the 32-bit Timer Counter overflows.

- If **PR = 0**, the PC matches the PR immediately, causing the TC to increment on **every** PCLK cycle.
- If **PR = 1**, the TC increments every **2** PCLK cycles.
- If **PR = 2**, the TC increments every **3** PCLK cycles (the PC counts 0, 1, 2, then resets).
- In general, the Timer Counter is incremented every **PR + 1** cycles of PCLK.

**Register Locations:** Each of the four General Purpose Timers has its own dedicated, read/write Prescale Register with a default reset value of `0`:

- **T0PR** (Timer 0): `0x4000 400C`
- **T1PR** (Timer 1): `0x4000 800C`
- **T2PR** (Timer 2): `0x4009 000C`
- **T3PR** (Timer 3): `0x4009 400C`

## PC – Prescale Counter

The **Prescale Counter (PC)** is a 32-bit counter that works directly with the Prescale Register (PR) to control the rate at which the main Timer Counter (TC) increments.

Here is a breakdown of its operation and purpose:

- **Clock Division:** The PC controls the division of the peripheral clock (PCLK) by a constant value before that clock is applied to the TC. This allows you to effectively manage the trade-off between the resolution of your timer and the maximum duration it can count before the 32-bit TC overflows.
- **Counting Mechanism:** The PC increments on every single cycle of the PCLK. It continually counts up until its value equals the value stored in the Prescale Register (PR).
- **Match Event:** When the PC reaches the PR value, the very next PCLK cycle causes the **Timer Counter (TC) to increment** and the **Prescale Counter (PC) to be cleared** (reset to zero) so it can begin counting again. As an example, if the PR is set to `0`, the TC increments on every PCLK cycle; if the PR is set to `1`, the TC increments every 2 PCLK cycles.
- **Control and Reset:** The PC is fully observable and controllable through the bus interface. Additionally, both the PC and the TC can be synchronously reset to zero on the next positive edge of the PCLK by setting the Counter Reset bit (Bit 1) in the Timer Control Register (TCR).

Each of the four General Purpose Timers has its own dedicated Prescale Counter register:

- **T0PC** (Timer 0): `0x4000 4010`
- **T1PC** (Timer 1): `0x4000 8010`
- **T2PC** (Timer 2): `0x4009 0010`
- **T3PC** (Timer 3): `0x4009 4010`

## Match Registers (MR0–MR3)

The **Match Registers (MR0–MR3)** are 32-bit registers whose values are continuously compared against the current value of the Timer Counter (TC). When the TC reaches the exact value stored in one of the Match Registers, the microcontroller can automatically trigger a series of actions without requiring any software intervention.

These hardware actions are configured using two main control registers: the **Match Control Register (MCR)** and the **External Match Register (EMR)**.

1. Internal Actions: Interrupt, Reset, and Stop

The Match Control Register (MCR) is used to dictate how the core timer hardware behaves when a match occurs. Each Match Register (MR0–MR3) has three dedicated bits in the MCR to enable any combination of the following actions:

- **Interrupt:** Generates a timer interrupt. For example, setting the `MR0I` bit triggers an interrupt when the TC matches MR0.
- **Reset:** Synchronously resets the Timer Counter (TC) back to `0`. Setting the `MR0R` bit means the timer will automatically start counting from zero again immediately after matching MR0.
- **Stop:** Stops both the Timer Counter (TC) and the Prescale Counter (PC), halting the timer completely until it is re-enabled by software.

2. External Actions: Toggle Output

The External Match Register (EMR) manages what happens to the physical external match pins (MATn.0 through MATn.3) when a match event occurs.

By configuring the External Match Control bits (`EMC0` through `EMC3`), you can set the corresponding hardware output pin to perform one of four actions on a match:

- **Toggle:** The pin reverses its current state (switches from HIGH to LOW, or LOW to HIGH).
- **Clear:** The pin is forced LOW.
- **Set:** The pin is forced HIGH.
- **Do Nothing:** The pin state remains completely unchanged.

Practical Application: Pulse Width Modulation (PWM)

In addition to general-purpose timing, Match Registers are the foundational mechanism for the Pulse Width Modulator (PWM). In PWM mode, **Match Register 0 (MR0)** is typically configured to reset the timer on a match, which dictates the overall PWM cycle rate (the frequency). The remaining Match Registers (MR1, MR2, etc.) are used to trigger the actual HIGH/LOW toggles on the output pins, dictating the exact edge positions (the duty cycle) of the generated PWM signals.

## MCR – Match Control Register

The **Match Control Register (MCR)** is used to control what automated hardware operations are performed when the value in a Match Register exactly equals the value currently in the Timer Counter (TC).

For each Match Register available within a timer block, the MCR provides three dedicated bits to independently enable any combination of the following actions upon a match:

- **Interrupt (I):** When set to `1`, an interrupt is generated when the Match Register equals the Timer Counter.
- **Reset (R):** When set to `1`, the Timer Counter (TC) is automatically reset to `0` when a match occurs.
- **Stop (S):** When set to `1`, both the Timer Counter (TC) and the Prescale Counter (PC) are stopped, and the counter enable bit in the Timer Control Register (TCR) is cleared to `0` when a match occurs.

Within the microcontroller, there are two distinct variations of this register depending on the peripheral:

1. General Purpose Timers (T0MCR, T1MCR, T2MCR, T3MCR)

The four general-purpose timers each have an MCR located at addresses `0x4000 4014` (Timer 0), `0x4000 8014` (Timer 1), `0x4009 0014` (Timer 2), and `0x4009 4014` (Timer 3). These registers use bits 0 through 11 to control actions for the **four standard Match Registers (MR0–MR3)**:

- **Bits 2:0:** Control MR0 (Interrupt, Reset, Stop)
- **Bits 5:3:** Control MR1 (Interrupt, Reset, Stop)
- **Bits 8:6:** Control MR2 (Interrupt, Reset, Stop)
- **Bits 11:9:** Control MR3 (Interrupt, Reset, Stop)

2. Pulse Width Modulator (PWM1MCR)

Because the Pulse Width Modulator block is based on the standard timer but features expanded capabilities, its Match Control Register (located at `0x4001 8014`) is larger. It uses bits 0 through 20 to control actions for up to **seven Match Registers (PWMMR0–PWMMR6)**:

- **Bits 2:0:** Control PWMMR0 (Interrupt, Reset, Stop)
- **Bits 5:3:** Control PWMMR1 (Interrupt, Reset, Stop)
- **Bits 8:6:** Control PWMMR2 (Interrupt, Reset, Stop)
- **Bits 11:9:** Control PWMMR3 (Interrupt, Reset, Stop)
- **Bits 14:12:** Control PWMMR4 (Interrupt, Reset, Stop)
- **Bits 17:15:** Control PWMMR5 (Interrupt, Reset, Stop)
- **Bits 20:18:** Control PWMMR6 (Interrupt, Reset, Stop)

For both register variants, all other higher-order bits are reserved and should not be written to.

## UxLCR (Line Control Register)

The **Line Control Register (UnLCR)** is used to determine the format of the data characters being transmitted or received by the UART peripherals.

There is a dedicated LCR for each of the four UARTs: **U0LCR** (UART0), **U1LCR** (UART1), **U2LCR** (UART2), and **U3LCR** (UART3).

Here is the specific bit-level structure and functionality of the Line Control Register:

- **Bits 1:0 (Word Length Select):** Determines the number of data bits in each transmitted/received character.
    - `00`: 5-bit character length
    - `01`: 6-bit character length
    - `10`: 7-bit character length
    - `11`: 8-bit character length
- **Bit 2 (Stop Bit Select):** Determines the number of stop bits sent at the end of a character. `0` selects 1 stop bit. `1` selects 2 stop bits (or 1.5 stop bits if a 5-bit character length was selected above).
- **Bit 3 (Parity Enable):** When `0`, parity generation and checking are disabled. When `1`, parity is enabled.
- **Bits 5:4 (Parity Select):** If parity is enabled (Bit 3 = 1), these bits dictate the type of parity used:
    - `00`: Odd parity (the total number of 1s in the data + parity bit is odd).
    - `01`: Even parity (the total number of 1s in the data + parity bit is even).
    - `10`: Forced "1" stick parity.
    - `11`: Forced "0" stick parity.
- **Bit 6 (Break Control):** When set to `1`, break transmission is enabled, which forces the UART's TXD output pin to a constant logic `0` (spacing state). When `0`, break transmission is disabled.
- **Bit 7 (Divisor Latch Access Bit - DLAB):** A critical control bit used for configuring the UART baud rate.
    - `0`: Disables access to the Divisor Latches, allowing normal access to the Receiver Buffer Register (RBR), Transmit Holding Register (THR), and Interrupt Enable Register (IER).
    - `1`: Enables access to the Divisor Latches (DLL and DLM) so you can program the desired baud rate.

**Key Usage Note:** Because the Divisor Latches (DLL/DLM) share the same memory addresses as the Receiver Buffer (RBR), Transmit Holding Register (THR), and Interrupt Enable Register (IER), **you must flip the DLAB bit (Bit 7) to** **1** **before you can set your baud rate**, and then you **must flip it back to** **0** before you can actually send/receive data or enable UART interrupts.
## UxDLL / UxDLM

The **Divisor Latch LSB (UxDLL)** and **Divisor Latch MSB (UxDLM)** registers are used to configure the communication speed (baud rate) for the UART peripherals.

Together, these two 8-bit registers form a **single 16-bit divisor value**, where `UxDLL` holds the lower 8 bits and `UxDLM` holds the upper 8 bits. The UART's internal Baud Rate Generator uses this full divisor, in combination with an optional Fractional Divider, to scale down the peripheral clock (PCLK) and produce a baud rate clock that operates at exactly 16 times the desired UART baud rate.

Here are the key operational rules and requirements for using the UxDLL and UxDLM registers:

- **DLAB Must Be Set:** Because these registers share the same memory addresses as the normal read/write data registers (Receiver Buffer and Transmit Holding registers), you can only access the `UxDLL` and `UxDLM` registers when the **Divisor Latch Access Bit (DLAB)** in the Line Control Register (`UxLCR`) is set to `1`.
- **No Zero Division:** A programmed divisor value of `0x0000` is automatically treated by the hardware as `0x0001`, since division by zero is not allowed.
- **Fractional Divider Minimums:** If you are actively using the fractional baud-rate generator (meaning the `DIVADDVAL` value in the `UxFDR` register is greater than 0) and your `UxDLM` is set to `0`, you **must ensure the value in the** **UxDLL** **register is strictly greater than 2**.
- **Auto-Baud Update Sequence:** If your application uses the auto-baud feature to automatically detect and set the baud rate, any manual software writes to the `UxDLM` and `UxDLL` registers must be completed _before_ initiating the auto-baud process via the Auto-baud Control Register (`UxACR`).
## UxTHR Transmit register

The **Transmit Holding Register (UxTHR)** is a write-only register used to load data that you want to transmit via the UART peripherals.

Within the LPC17xx architecture, there is a dedicated THR for each UART: **U0THR** (`0x4000 C000`), **U1THR** (`0x4001 0000`), **U2THR** (`0x4009 8000`), and **U3THR** (`0x4009 C000`).

Here is the breakdown of its structure and operational rules:

**Bit-Level Structure**

- **Bits 7:0 (THR):** This 8-bit field holds the next character to be transmitted. Because it acts as the top byte of the UART's internal Transmit (TX) FIFO, writing to this register places your data directly into the TX FIFO. The byte will actually be sent out on the serial line once it reaches the bottom of the FIFO and the transmitter becomes available. The Least Significant Bit (LSB) of this data is always the first bit transmitted.
- **Bits 31:8:** These bits are reserved and should not be written to.

**Key Usage Notes**

- **DLAB Must Be Cleared:** Because the Transmit Holding Register shares the same memory address as the Divisor Latch LSB (UxDLL) and the Receiver Buffer Register (UxRBR), **you can only write to the UxTHR when the Divisor Latch Access Bit (DLAB) in the Line Control Register (UxLCR) is set to** **0**.
- **Write-Only:** The register is strictly write-only. If you need to read incoming serial data, you would read from the Receiver Buffer Register (UxRBR) at the same memory address.

## UxRBR

The **Receiver Buffer Register (UxRBR)** is a read-only register that represents the top byte of the UART's Receive (Rx) FIFO.

It contains the oldest received character ready to be read via the bus interface.

**Bit-Level Structure**

- **Bits 7:0 (RBR):** This field holds the oldest received byte in the Rx FIFO. The Least Significant Bit (LSB, bit 0) represents the very first (oldest) data bit received. If the received character length is configured to be less than 8 bits, the unused Most Significant Bits (MSBs) are automatically padded with zeroes.
- **Bits 31:8:** These bits are reserved and their read values are undefined.

**Key Usage Rules**

- **DLAB Must Be Cleared:** Because the UxRBR shares the exact same memory address as the Transmit Holding Register (UxTHR) and the Divisor Latch LSB (UxDLL), **the Divisor Latch Access Bit (DLAB) in the Line Control Register (UxLCR) must be exactly** **0** in order to access the Receive Buffer.
- **Reading Sequence for Errors:** Line error flags—specifically Parity Error (PE), Framing Error (FE), and Break Interrupt (BI)—correspond directly to the specific byte sitting at the top of the Rx FIFO. To fetch a valid pair of data and its matching status bits, **the correct approach is to always read the Line Status Register (UxLSR) first**, and then read the data byte from the UxRBR.

**Register Addresses (when DLAB = 0)**

- **U0RBR** (UART0): `0x4000 C000`
- **U1RBR** (UART1): `0x4001 0000`
- **U2RBR** (UART2): `0x4009 8000`
- **U3RBR** (UART3): `0x4009 C000`

## UxLSR (Line Status Register)

The **Line Status Register (UxLSR)** is a read-only register that provides essential status information regarding the UART's Transmit (TX) and Receive (RX) blocks, including the presence of unread data, the state of the transmitter, and any line errors that have occurred.

Each of the four UARTs has its own Line Status Register (U0LSR, U1LSR, U2LSR, and U3LSR) following the exact same bit-level structure:

- **Bit 0 (RDR - Receiver Data Ready):** Set to `1` when the Receiver Buffer Register (UxRBR) holds an unread character. It is cleared to `0` when the UART's Rx FIFO is completely empty.
- **Bit 1 (OE - Overrun Error):** Set to `1` if a new character has been fully assembled in the internal Shift Register but the Rx FIFO is already full. This indicates that the newly received character was lost because it could not be written to the FIFO. Cleared by reading the UxLSR.
- **Bit 2 (PE - Parity Error):** Set to `1` if the parity bit of the received character is in the wrong state. Cleared by reading the UxLSR.
- **Bit 3 (FE - Framing Error):** Set to `1` if the stop bit of a received character is a logic `0` instead of the expected `1`. Cleared by reading the UxLSR.
- **Bit 4 (BI - Break Interrupt):** Set to `1` if the RX input pin is held in the spacing state (all zeroes) for the duration of one full character transmission (including start, data, parity, and stop bits). Cleared by reading the UxLSR.
- **Bit 5 (THRE - Transmitter Holding Register Empty):** Set to `1` immediately when the Transmit Holding Register (UxTHR) becomes empty. It is cleared to `0` when software writes new data into the UxTHR.
- **Bit 6 (TEMT - Transmitter Empty):** Set to `1` only when _both_ the Transmit Holding Register (UxTHR) and the internal Transmit Shift Register (UxTSR) are completely empty. It is cleared to `0` if either register contains valid data.
- **Bit 7 (RXFE - Error in RX FIFO):** Set to `1` if any character currently sitting in the Rx FIFO contains an error (such as a Framing Error, Parity Error, or Break Interrupt). It is cleared when the UxLSR is read and there are no subsequent errors remaining in the FIFO.

**Key Usage Notes:**

- **Default State:** Upon chip reset, the register contains the default value `0x60` (meaning Bits 5 and 6 are `1`). This indicates that both the transmitter holding register and shift register are empty and ready to accept data.
- **Reading Sequence for Errors:** The error flags (Parity Error, Framing Error, and Break Interrupt) correspond specifically to the data byte currently sitting at the _top_ of the Rx FIFO. Because reading the UxRBR removes that byte from the FIFO, the correct procedure is to always read the UxLSR first to check for errors, and then read the UxRBR to retrieve the data byte.
- **Clearing Errors:** The action of reading the UxLSR register automatically clears the OE, PE, FE, and BI flags.

# Additional Essential System Modules
## Watchdog Registers

The **Watchdog Timer (WDT)** is designed to **reset the microcontroller within a reasonable amount of time if it enters an erroneous state**. It consists of a 32-bit down counter with a fixed divide-by-4 prescaler. Once enabled, the user program must periodically "feed" (reload) the timer to prevent it from underflowing and generating a system reset or interrupt. The WDT can be clocked from the Internal RC (IRC) oscillator, the RTC oscillator, or the APB peripheral clock.

**WDMOD (Watchdog Mode Register)** Located at address `0x4000 0000`, this register controls the basic operation mode and status of the Watchdog Timer.

- It contains the **WDEN** bit to enable the timer, the **WDRESET** bit to enable chip reset on timeout, the **WDTOF** time-out flag, and the **WDINT** interrupt flag.
- Changes made to the WDMOD register **do not take effect until a valid watchdog feed sequence is performed**.
- Once the WDEN or WDRESET bits are set, they **cannot be cleared by software**; they are only cleared by an external reset or a watchdog timer underflow.

**WDTC (Watchdog Timer Constant Register)** Located at address `0x4000 0004`, this 32-bit register determines the specific time-out value.

- Every time a valid feed sequence occurs, the contents of the WDTC are reloaded into the Watchdog timer.
- The hardware enforces a minimum value constraint: **writing any value below** **0xFF** **will automatically load** **0x0000 00FF** **into the WDTC**. This establishes the minimum possible watchdog interval as `(TWDCLK × 256 × 4)`.

**WDFEED (Watchdog Feed Register)** Located at address `0x4000 0008`, this write-only register is used to reload the timer and apply configuration changes.

- To properly feed the watchdog, you must **write** **0xAA** **followed directly by** **0x55** **to this register**.
- Performing this strict sequence also starts the Watchdog if it was enabled in the WDMOD register.
- If you write `0xAA` to the WDFEED register and then access any other Watchdog register before writing `0x55`, an **immediate reset or interrupt is generated** (provided the watchdog is already enabled).

**WDTV (Watchdog Timer Value Register)** Located at address `0x4000 000C`, this read-only register allows software to read the **current value of the Watchdog timer**.

- Because the watchdog timer uses a different clock domain (WDCLK) than the bus interface (PCLK), there is a synchronization process. Reading this 32-bit timer takes up to 6 WDCLK cycles plus 6 PCLK cycles, meaning **the value read by the CPU will be slightly older than the actual timer value** at the exact moment of the read.

## RTC (Battery-backed)

These registers belong to the **Real-Time Clock (RTC)** peripheral, which provides ultra-low power time-keeping and calendar functions that can remain active even when the main microcontroller power is removed.

Here is the detailed breakdown of the requested RTC registers:

1. CCR – Clock Control Register

Located at address `0x4002 4008`, the CCR controls the basic operation of the RTC's internal clock divider.

- **Bit 0 (CLKEN - Clock Enable):** When set to `1`, the time counters are enabled and running. When `0`, the time counters are disabled so they can be safely initialized by software.
- **Bit 1 (CTCRST - CTC Reset):** When set to `1`, the internal oscillator divider elements (which generate the 1 Hz clock from the 32 kHz crystal) are held in reset until this bit is cleared to `0`.
- **Bit 4 (CCALEN - Calibration Counter Enable):** When `0`, the calibration counter is enabled to adjust the RTC clock for better accuracy. When `1`, the calibration counter is disabled and reset to zero.

2. CTIME0, CTIME1, CTIME2 – Consolidated Time Registers

The Consolidated Time Registers allow software to efficiently read the complete current time and date in just three read operations. These registers are **read-only**; if you want to write a new time, you must use the individual Time Counter registers below.

- **CTIME0 (****0x4002 4014****):** Contains the rapidly changing time values. It packs together the **Seconds** (bits 5:0), **Minutes** (bits 13:8), **Hours** (bits 20:16), and **Day of Week** (bits 26:24).
- **CTIME1 (****0x4002 4018****):** Contains the calendar date values. It packs together the **Day of Month** (bits 4:0), **Month** (bits 11:8), and **Year** (bits 27:16).
- **CTIME2 (****0x4002 401C****):** Contains the **Day of Year** value (bits 11:0).

3. Time Counter Registers (SEC, MIN, HOUR, DOM, MONTH, YEAR)

These are the individual read/write counters that hold the actual clock and calendar values. They are incremented by the 1 Hz clock and automatically wrap around at their defined overflow points. Software must correctly initialize these registers to start the clock.

- **SEC (Seconds):** Range 0 to 59.
- **MIN (Minutes):** Range 0 to 59.
- **HOUR (Hours):** Range 0 to 23.
- **DOM (Day of Month):** Range 1 to 28, 29, 30, or 31. The RTC automatically adjusts the DOM limit depending on the current Month and whether the Year register indicates a leap year.
- **MONTH (Months):** Range 1 to 12.
- **YEAR (Years):** Range 0 to 4095. The RTC automatically calculates leap years by checking if the lowest two bits of this register are zero (meaning the year is evenly divisible by 4). _Note: This algorithm works from the year 1901 to 2099, but will fail for 2100_.

_(Note: The RTC also includes separate read/write counters for Day of Week (DOW) and Day of Year (DOY) to complete the calendar)_.

4. GPREG0–GPREG4 – General Purpose Registers

These are five 32-bit registers (located from `0x4002 4044` to `0x4002 4054`) used as **battery-backed storage**.

- They allow the user application to store important information that needs to be retained when the main CPU power supply (`VDD(REG)(3V3)`) is turned off.
- The values stored in GPREG0–4 are **not affected by a chip reset**. They will hold their data as long as power is maintained on the dedicated battery pin (`VBAT`).


