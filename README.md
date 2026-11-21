# Plant Intelligent

A bare-metal C (AVR) smart plant monitoring system built from scratch on a standalone **ATmega328P** breadboard circuit. Developed without Arduino frameworks or third-party sensor libraries, all peripheral drivers—including I2C (TWI), environmental compensation math, and an animated OLED graphics engine—are implemented directly at the register level.

![Platform](https://img.shields.io/badge/MCU-ATmega328P-blue)
![Language](https://img.shields.io/badge/Language-Bare--Metal%20C-00599C)
![Toolchain](https://img.shields.io/badge/Build-PlatformIO%20%7C%20AVR--GCC-orange)
![Status](https://img.shields.io/badge/Status-Phase%202%20Complete-brightgreen)

---

## Features

* **Zero External Libraries:** Custom bare-metal drivers written in pure C using `avr-libc` and direct hardware register manipulation (`TWCR`, `TWSR`, `TWBR`, `TWDR`).
* **Shared I2C (TWI) Bus at 100 kHz:** Four peripherals operating simultaneously on a dual-voltage (5V / 3.3V) bus bridged by a bi-directional logic level shifter.
* **Animated OLED Interface (SSD1306):** Custom 5×7 bitmap font renderer paired with a lightweight 32×32 pixel framebuffer (128 bytes RAM) that renders real-time, Xiaomi-style animated plant mood expressions.
* **Factory-Calibrated Environmental Sensing (BME280):** Reads internal Bosch calibration coefficients from sensor NVM and computes integer-compensated temperature, relative humidity, and atmospheric pressure.
* **Real-Time Clock (DS3231):** Hardware BCD timekeeping with battery backup and compile-time synchronization support (`__TIME__`).
* **Ambient Light Sensing (BH1750):** High-resolution continuous lux measurement (1 lx resolution) used both for telemetry and day/night mood detection.

---

## Hardware Architecture

The circuit is designed on a dual-rail breadboard to safely isolate the 5V microcontroller domain from the 3.3V I2C sensor domain:

* **5V Rail:** Powers the ATmega328P (`VCC` and `AVCC`), the USB-to-TTL programmer, and the High-Voltage (`HV`) side of the logic level shifter.
* **3.3V Rail:** Regulated via an **AMS1117-3.3**, powering the `LV` side of the level shifter and all four I2C peripherals.
* **Auto-Reset Circuit:** 10kΩ pull-up resistor on `RESET` (`PC6`) with a 100nF series capacitor connected to the TTL adapter's `DTR` pin.

### Bill of Materials & I2C Addressing

| Component | Function | Supply | Interface / Address |
| :--- | :--- | :---: | :--- |
| **ATmega328P-PU** | Main MCU (16 MHz crystal + 2× 22pF caps) | 5V | TWI Master / UART |
| **AMS1117-3.3** | Linear Voltage Regulator | 5V → 3.3V | Power |
| **4-Ch Logic Level Shifter** | Bi-directional 5V ↔ 3.3V I2C bridge | 5V / 3.3V | I2C (`SDA`/`SCL`) |
| **SSD1306 (128×64)** | Monochrome OLED Display | 3.3V | I2C (`0x3C`) |
| **BME280** | Temperature, Humidity & Pressure | 3.3V | I2C (`0x76` / `0x77`) |
| **DS3231** | Real-Time Clock (RTC) | 3.3V | I2C (`0x68`) |
| **BH1750** | Ambient Light Sensor (Lux) | 3.3V | I2C (`0x23` / `0x5C`) |

### ATmega328P Pin Mapping

| ATmega328P Pin | Port / Signal | Connection |
| :--- | :--- | :--- |
| Pin 1 | `PC6` (`RESET`) | 10kΩ pull-up to +5V; 100nF cap to TTL `DTR` |
| Pin 2 | `PD0` (`RXD`) | USB-TTL `TX` |
| Pin 3 | `PD1` (`TXD`) | USB-TTL `RX` |
| Pin 7, 20 | `VCC`, `AVCC` | +5V Rail |
| Pin 8, 22 | `GND` | Common GND |
| Pin 9, 10 | `PB6` / `PB7` (`XTAL1`/`2`) | 16 MHz Crystal + 22pF caps to GND |
| Pin 21 | `AREF` | 100nF decoupling capacitor to GND |
| Pin 27 | `PC4` (`SDA`) | Level Shifter `HV` → 3.3V `SDA` Bus |
| Pin 28 | `PC5` (`SCL`) | Level Shifter `HV` → 3.3V `SCL` Bus |
| Pin 23, 24 | `PC0`, `PC1` (`ADC0`/`1`) | *Reserved for Phase 3 (Soil Moisture & Battery ADC)* |

---

## Animated Mood Engine

To keep RAM usage minimal on the ATmega328P (2 KB SRAM), the right half of the OLED renders a procedural 32×32 pixel face directly into a 128-byte buffer and updates frame states every 400 ms:

| State | Trigger Condition | Animation |
| :--- | :--- | :--- |
| **Happy** | Normal light & `18 °C – 26 °C` | Smiling face with periodic `^ ^` eye blinks |
| **Night** | Light `< 20 lx` | Closed eyes, slow breathing mouth, floating `z` |
| **Hot** | Temperature `≥ 27 °C` | Worried eyebrows, frown, animated falling sweat drop |
| **Cold** | Temperature `≤ 17 °C` | Shivering head movement with zigzag mouth |

Thresholds (`LUX_NOITE`, `TEMP_CALOR`, `TEMP_FRIO`) can be configured at the top of `src/main.c`.

---

## Build & Flash

This project uses **PlatformIO** (VS Code or CLI) targeting an ATmega328P flashed with an Arduino-compatible bootloader at 16 MHz.

### 1. Clone and Build
```bash
git clone git@github.com:KitCat-png/plant-intelligent.git
cd plant-intelligent
pio run