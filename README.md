# BMW E36 ESP32 Custom Cluster Gauge & Multi-Function Controller

A custom digital multi-gauge and body electronics controller for the BMW E36, running on an ESP32 and a 320x170 ST7789 IPS display. Designed with an OEM BMW amber/orange aesthetic to fit seamlessly into the factory instrument cluster or console.

---

## ✨ Features

- **OEM Cluster Styling:** Custom digital layout utilizing `DS-Digital` styling and BMW amber/orange color grading (`0x0800` / `0xfa20`).
- **Live Battery Voltage Monitor:** Real-time system voltage tracking with smooth averaging and dynamic decimal rendering.
- **Cranking Voltage Dip Capture:**
  - Automatically detects starter motor engagement via sudden voltage drop below baseline.
  - Records and holds the lowest voltage dip reached during engine crank.
  - Displays the minimum crank voltage for 5 seconds once the engine fires and alternator activates (>13.0V), then restores the normal view.
- **Oil Temperature & Warmup Progress Bar:**
  - High-precision NTC thermistor reading using a calibrated lookup table (up to 150°C).
  - Clean centered progress bar tracking engine warm-up (Cold → Amber → OEM Green @ 75°C+).
  - Automatically activates only when the alternator is charging (>13.0V) and cleanly hides when the engine is off.
- **Comfort Lane-Change Turn Signals:**
  - Tap-to-blink comfort indicator controller (3 blinks on momentary touch).
  - Dedicated relay outputs with 200ms debounce protection.
- **Cluster Illumination / Backlight Dimming:** PWM backlight control triggered by headlight status.
- **Cruise Control & Low Beam Icons:** Integrated dashboard indicator icons with standby timeout handling.
- **Custom Boot Splash Screen:** BMW/custom splash sequence at startup.

---

## 🛠 Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| **TFT SCL (SCK)** | `GPIO 23` | SPI Clock (VSPI) |
| **TFT SDA (MOSI)**| `GPIO 21` | SPI MOSI (VSPI) |
| **TFT RES (RST)** | `GPIO 16` | Display Reset |
| **TFT DC**        | `GPIO 4`  | Data / Command |
| **TFT CS**        | `GPIO 0`  | Chip Select |
| **TFT Backlight** | `GPIO 2`  | PWM Dimming Control |
| **Battery Voltage**| `GPIO 32` | Analog In (Resistor Divider) |
| **Oil Temp (NTC)**| `GPIO 33` | Analog In (NTC Voltage Divider) |
| **Cruise In**     | `GPIO 5`  | Digital In (Active LOW) |
| **Blinker Left In** | `GPIO 19` | Turn Signal Stalk Input |
| **Blinker Right In**| `GPIO 18` | Turn Signal Stalk Input |
| **Blinker Left Relay** | `GPIO 27` | Output to Left Relay |
| **Blinker Right Relay**| `GPIO 14` | Output to Right Relay |

---

## 📦 Required Libraries

- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit ST7735 and ST7789 Library](https://github.com/adafruit/Adafruit-ST7735-Library)
- `SPI` & `Wire` (Standard ESP32 Arduino Core)

---

## ⚡ Flashing Instructions

- **Target Board:** ESP32 WROOM-DA / ESP32 Dev Module
- **Flash Mode:** `DOUT` *(Required for ESP32-D0WD rev 3.1 with Fudan Micro flash chips)*
- **Upload Speed:** 115200 or 921600 baud
