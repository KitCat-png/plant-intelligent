# 🌱 Plant Intelligent

A bare-metal **AVR C** embedded plant monitoring system built from scratch on a standalone **ATmega328P-PU** microcontroller — **no Arduino framework or third-party libraries**.

The system monitors ambient temperature, air humidity, atmospheric pressure, ambient light, real-time clock data, and soil moisture, displaying live readings alongside an animated retro **Tamagotchi-style** plant companion on an SSD1306 OLED screen.

---

## ✨ Key Features

* **100% Bare-Metal C (`avr-libc`):** Custom register-level drivers for TWI (I2C at 100 kHz), 10-bit ADC (16-sample averaging), SSD1306 OLED, BME280, DS3231 RTC, and BH1750.
* **Integer-Only BME280 Compensation:** Implements Bosch's official factory calibration formulas using fixed-point integer math to avoid floating-point overhead on an 8-bit MCU.
* **Procedural 48×48 Tamagotchi UI:** A 24×24 pixel-art sprite engine scaled 2× (288 bytes of RAM) featuring **6 animated moods** driven by sensor thresholds:
  * **Happy** — optimal environment (bouncing animation + blink)
  * **Thirsty** (`≤ 30%` soil moisture) — droopy sprout, panting, falling water drop
  * **Overwatered** (`≥ 85%` soil moisture) — submerged in animated waves with rising bubbles
  * **Hot** (`≥ 27°C`) — droopy sprout, sweat drop, heat wave
  * **Cold** (`≤ 17°C`) — shivering animation
  * **Night / Sleep** (`< 20 lx`) — curled up sleeping with floating `Z`
* **Minimalist OLED Layout + Calibration Mode:** Displays date/time on Page 0 and 8×8 pixel-art icons for temperature, air humidity, and soil moisture on the left (intentionally keeping Page 5 blank to work around a hardware dead-pixel spot on the prototype display). Toggle `#define DEBUG_SCREEN 1` to view raw ADC values, lux, and pressure during calibration.

---

## 🔌 Hardware & Wiring

### Core Setup
* **MCU:** Standalone ATmega328P-PU @ 16 MHz (`16 MHz` crystal + `2× 22 pF` capacitors)
* **Programming:** USB-TTL serial adapter with auto-reset (`10 kΩ` pull-up on Pin 1 `RESET` + `100 nF` cap on `DTR`)
* **ADC Reference:** `AVCC` (5V) with `100 nF` decoupling capacitor on Pin 21 (`AREF`) to `GND`
* **Dual Power Rails:**
  * **5V Rail:** ATmega `VCC` (Pin 7), `AVCC` (Pin 20), USB-TTL, Logic Level Converter (`HV`), Capacitive Soil Moisture Sensor
  * **3.3V Rail (via AMS1117-3.3):** Logic Level Converter (`LV`) and all I2C peripherals

### Pinout & Peripherals

| Module / Interface | Protocol / Address | ATmega328P Pin | Notes |
| :--- | :--- | :--- | :--- |
| **I2C Bus (`SCL` / `SDA`)** | TWI @ 100 kHz | Pin 28 (`PC5`) / Pin 27 (`PC4`) | Routed through 5V ↔ 3.3V bidirectional level shifter |
| **OLED SSD1306 (128×64)** | I2C (`0x3C`) | I2C Bus (3.3V) | Yellow top band / blue main area |
| **BME280** | I2C (`0x76` / `0x77`) | I2C Bus (3.3V) | Temperature, air humidity, pressure |
| **DS3231 RTC** | I2C (`0x68`) | I2C Bus (3.3V) | Battery-backed real-time clock |
| **BH1750** | I2C (`0x23`) | I2C Bus (3.3V) | Ambient light sensor (`ADDR` → `GND`) |
| **Capacitive Soil Sensor v1.2** | Analog (ADC) | Pin 23 (`PC0` / `ADC0`) | Powered at 5V; corrosion-resistant capacitive probe |

---

## 📁 Project Structure

```text
.
├── include/
│   ├── config.h      # Pin/address macros, sensor calibration, mood thresholds
│   ├── i2c.h         # Hardware TWI (I2C) driver interface
│   ├── adc.h         # 10-bit ADC driver & soil moisture scaling
│   ├── ds3231.h      # RTC driver
│   ├── bh1750.h      # Ambient light sensor driver
│   ├── bme280.h      # Environmental sensor driver
│   ├── oled.h        # SSD1306 display & 5x7 bitmap font driver
│   └── ui.h          # 8x8 icons & 48x48 Tamagotchi sprite engine
├── src/
│   ├── i2c.c
│   ├── adc.c
│   ├── ds3231.c
│   ├── bh1750.c
│   ├── bme280.c
│   ├── oled.c
│   ├── ui.c
│   └── main.c        # System initialization & main sensor/UI loop
└── platformio.ini