; PlatformIO Project Configuration File
; -------------------------------------
; This config builds the Automated Greenhouse Controller
; for an ESP32 Dev Module (DevKitC / WROOM).

[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

; Serial monitor settings
monitor_speed = 115200

; Libraries (optional — add only those your project needs)
; Examples: PubSubClient for MQTT, DHT sensor library, OneWire, DallasTemperature, LiquidCrystal_I2C, etc.
lib_deps =
    PubSubClient
    adafruit/DHT sensor library@^1.4.0
    paulstoffregen/OneWire@^2.3.7
    milesburton/DallasTemperature@^3.9.1
    adafruit/Adafruit BusIO@^1.16.1
    adafruit/Adafruit GFX Library@^1.11.5
    adafruit/Adafruit SSD1306@^2.5.7

; Uncomment if you have a local “lib” or need additional include paths
; lib_extra_dirs = lib

; Build flags (define macros if needed)
build_flags =
    -D SERIAL_OUTPUT_ENABLED
    -D USE_WIFI

; Optional custom upload port (if needed):
; upload_port = /dev/ttyUSB0

; Optional: Override default flash frequency or size
; board_build.f_flash = 80m
; board_build.flash_size = 4MB
