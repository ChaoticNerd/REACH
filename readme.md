# REACH — STM32F407G-DISC1 Pin Map

**Updated:** November 2, 2026  
**Microcontroller:** STM32F407VGT6 (STM32F407G-DISC1)  
**Scope:** All GPIO pins assigned in the current REACH camera, FSR, and test firmware; proposed connections are explicitly labeled.

## 1. Master GPIO pin allocation

| STM32 pin | Peripheral / signal | Component or function | Status / notes |
|---|---|---|---|
| **PA1** | ADC1_IN1 | Adafruit #1075 FSR voltage-divider output | Assigned; analog input |
| **PA4** | DCMI_HSYNC / HREF | OV7670 HREF | Assigned; AF13 |
| **PA6** | DCMI_PIXCLK | OV7670 PCLK | Assigned; AF13 |
| **PA8** | MCO1 / XCLK | OV7670 external clock | Assigned; AF0, nominal 16 MHz HSI output |
| **PB6** | DCMI_D5 | OV7670 D5 | Assigned; AF13; **use PB6, not PD3** |
| **PB7** | DCMI_VSYNC | OV7670 VSYNC | Assigned; AF13 |
| **PB8** | I2C1_SCL | Shared I²C/SCCB clock: OV7670 (direct test), MPU-6050; later TCA9548A and IR sensor | Assigned; AF4, open drain |
| **PB9** | I2C1_SDA | Shared I²C/SCCB data: OV7670 (direct test), MPU-6050; later TCA9548A and IR sensor | Assigned; AF4, open drain |
| **PC6** | DCMI_D0 | OV7670 D0 | Assigned; AF13 |
| **PC7** | DCMI_D1 | OV7670 D1 | Assigned; AF13 |
| **PC8** | DCMI_D2 | OV7670 D2 | Assigned; AF13 |
| **PC9** | DCMI_D3 | OV7670 D3 | Assigned; AF13 |
| **PC11** | DCMI_D4 | OV7670 D4 | Assigned; AF13 |
| **PD12** | GPIO output | Onboard **green** LED: test pass/contact | Used by test firmware; no external wire |
| **PD13** | GPIO output | Onboard **orange** LED: test activity | Used by test firmware; no external wire |
| **PD14** | GPIO output | Onboard **red** LED: test failure | Used by test firmware; no external wire |
| **PE5** | DCMI_D6 | OV7670 D6 | Assigned; AF13 |
| **PE6** | DCMI_D7 | OV7670 D7 | Assigned; AF13 |

**Currently assigned:** 18 distinct GPIO pins, including three onboard LEDs

## 2. OV7670 camera — direct wiring for the working single-camera test

| OV7670 label | STM32 pin / connection | Direction relative to STM32 |
|---|---|---|
| XCLK | PA8 | Output to camera |
| PCLK | PA6 | Input from camera |
| HREF | PA4 | Input from camera |
| VSYNC | PB7 | Input from camera |
| D0 | PC6 | Input from camera |
| D1 | PC7 | Input from camera |
| D2 | PC8 | Input from camera |
| D3 | PC9 | Input from camera |
| D4 | PC11 | Input from camera |
| D5 | **PB6** | Input from camera |
| D6 | PE5 | Input from camera |
| D7 | PE6 | Input from camera |
| SIOC / SCL | PB8 | I²C1 clock |
| SIOD / SDA | PB9 | I²C1 data |
| GND | STM32 GND | Common ground |
| VCC | Supply compatible with OV7670 breakout| Check breakout specifications before connecting |
| PWDN | Logic LOW during normal operation | Fixed level if correctly supported by breakout; no STM32 GPIO required |
| RESET | Logic HIGH for your common active-low-reset OV7670 revision | Verify breakout/revision; no STM32 GPIO required |

**Camera peripherals:** DCMI + DMA2 Stream1 Channel 1, capturing 160 × 120 RGB565 (38,400 bytes). DMA memory-to-memory tests use DMA2 Stream0 internally; they do not consume GPIO pins.

## 3. Adafruit MPU-6050 — new I²C sensor

Applies to **Adafruit MPU-6050 6-DoF breakout, product #3886**, not necessarily generic GY-521 breakouts.

| MPU-6050 breakout pin | STM32F407G-DISC1 connection | Notes |
|---|---|---|
| **VIN** | **3.3 V** supply pin | Adafruit breakout has an onboard regulator; VIN supports 3–5 V. Power at 3.3 V for this STM32 design. |
| **GND** | **GND** | Must share ground with STM32. |
| **SCL** | **PB8** (I2C1_SCL) | Shared I²C clock. |
| **SDA** | **PB9** (I2C1_SDA) | Shared I²C data. |
| **AD0** | Leave at default LOW (or explicitly tie to GND if required by the exact board) | Default 7-bit address **0x68**; HIGH selects **0x69**. |
| **INT** | **Not connected initially** | Optional data-ready/motion interrupt; start with I²C polling to save a GPIO. Assign an unused interrupt-capable pin only if needed later. |
| **3Vo** | **Leave unconnected** | Regulated 3.3 V **output** on Adafruit breakout; do not confuse with VIN. |
| **FS, SCE, SDE, CLKIN** | Leave unconnected | Advanced auxiliary/external interface pins, not required for basic gyro/accelerometer readings. |

The Adafruit breakout has onboard I²C level shifting and approximately **10 kΩ pull-ups on SCL/SDA**. Check other connected breakout pull-ups before adding external resistors. Its standard solderable header holes provide direct jumper-wire access; the STEMMA QT connector is optional.

**Minimum new wiring:** four leads: `VIN → 3.3V`, `GND → GND`, `SCL → PB8`, `SDA → PB9`.

## 4. Other REACH sensors and I²C bus plan

| Device | STM32 connection | 7-bit I²C address | Status |
|---|---|---|---|
| OV7670 #1 SCCB | PB8/PB9 directly for current test | `0x21` | Current one-camera setup |
| **Adafruit MPU-6050** | PB8/PB9, directly on shared I²C1 bus | **`0x68`** (default) | **New assignment** |
| TCA9548A I²C multiplexer | PB8/PB9 upstream SCL/SDA | `0x70` (default) | Planned reconnection for two cameras; currently bypassed |
| OV7670 #1 / #2 via mux | TCA9548A channels 0 / 1, respectively | `0x21` on each separate channel | Later two-camera configuration |
| MLX90614-DCI infrared thermometer | PB8/PB9, either shared upstream or on a TCA9548A downstream channel | `0x5A` (default) | Planned; not confirmed connected |
| Adafruit #1075 FSR | PA1 (ADC1_IN1) | Not an I²C device | Assigned; 10 kΩ divider resistor to GND |

Because the devices use distinct I²C addresses, the **MPU-6050 can share PB8/PB9** with the camera SCCB and the TCA9548A. With two OV7670 cameras, use the mux to isolate their duplicate `0x21` addresses.

## 5. Built-in ST-LINK debugging connections

| STM32 pin | Function | Notes |
|---|---|---|
| **PA13** | SWDIO | Reserved for onboard ST-LINK debugging/programming |
| **PA14** | SWCLK | Reserved for onboard ST-LINK debugging/programming |

PA13/PA14 are dedicated to the on-board debugger in our workflow and **not counted as application GPIO assignments** above. No extra jumper wires are needed when the Discovery board's normal debug jumpers are installed.

## 6. Planned but NOT assigned yet

| Component | STM32 pin assignment | Notes |
|---|---|---|
| Three TMUX1574 video multiplexers | **TBD** | PC0 was a proposed shared SELECT signal in earlier firmware, but is **not confirmed wired or enabled**. The mux also requires correct enable/control wiring. |
| SPI LCD (ST7735, ST7789, or ILI9341) | **TBD** | Select SPI bus, display CS, DC, RESET, and backlight pins after LCD choice. Avoid all assigned DCMI pins. |
| Gripper / arm motors | **PA0** | Select timer/PWM/control pins once motor drivers are finalized. Number of motors determines how many pins are used |
| Optional MPU-6050 interrupt | **TBD** | Only required if polling isn't sufficient. |

## 7. Implementation cautions

1. **Do not reconnect camera D5 to PD3.** The corrected STM32 DCMI input for D5 is PB6.
2. Never connect a 5 V I²C pull-up directly to the shared 3.3 V STM32 signals. Use verified 3.3 V logic levels throughout the shared bus.
3. Use a common ground for the STM32, camera, FSR divider, MPU-6050, and other sensor boards.
4. The OV7670 breakout's power and control pin requirements depend on its exact version; confirm them from its labeling/documentation.
5. The MPU-6050 can be read via your existing bare-metal `I2C.c/.h`; it needs a device-specific register driver, but **not** a separate I²C peripheral or additional STM32 GPIOs.

## References

- [Adafruit MPU-6050 product #3886](https://www.adafruit.com/product/3886)
- [Adafruit MPU-6050 pinouts and 3.3 V / I²C guidance](https://learn.adafruit.com/mpu6050-6-dof-accelerometer-and-gyro/pinouts)
- [STM32F407 reference manual RM0090](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

