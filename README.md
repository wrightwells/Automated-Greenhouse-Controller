# 🌱 Automated Greenhouse Controller

An ESP32-based automated greenhouse control system featuring climate monitoring, irrigation, ventilation, and remote MQTT control.

---

## ✨ Features

- Climate monitoring (temperature, humidity, intake air)
- Automated ventilation
- Automated window control
- Soil moisture–based irrigation
- Flow rate monitoring
- MQTT-based remote control
- Local LCD status display
- Manual override via MQTT

---

## 🧠 Core Controller

| Item | Qty | Specification | Notes |
|-----|-----|---------------|-------|
| ESP32 Development Board | 1 | ESP32-WROOM / DevKitC | Must support WiFi, interrupts, ADC, I2C |

---

## 🌡️ Sensors

| Item | Qty | Specification | Used For |
|-----|-----|---------------|----------|
| DHT22 Temperature/Humidity Sensor | 2 | AM2302 | High & low mounted air temp and RH |
| DS18B20 Temperature Sensor | 1 | Waterproof recommended | Intake air temperature |
| Soil Moisture Sensor (Analog) | 1 | Capacitive preferred | Uses ADC pin |
| Water Flow Sensor | 1 | Hall-effect (e.g. YF-S201) | Irrigation flow rate |
| Magnetic Reed Switch | 1 | Normally closed | Door open/closed detection |

---

## 🪟 Actuators (Relays / Loads)

| Item | Qty | Type | Controlled Device |
|-----|-----|------|-------------------|
| Relay Channels | 8 | 5V or 3.3V compatible | — |
| Window Motor / Linear Actuator | 2 | Directional or timed | Window 1 & 2 |
| Intake Flap Actuator | 1 | Motor / Solenoid | Fresh air flap |
| Exhaust Flap Actuator | 1 | Motor / Solenoid | Exhaust flap |
| Exhaust Fan | 1 | AC or DC | High temperature ventilation |
| Intake Fan | 1 | AC or DC | Fresh air intake |
| Water Pump | 1 | AC or DC | Irrigation |
| Spare Relay | 1 | — | Expansion |

⚠️ **Relays are active LOW** (`HIGH = OFF`)

---

## 🖥️ Display & Interface

| Item | Qty | Specification |
|-----|-----|---------------|
| LCD Display | 1 | 16×2 I2C (0x3F) |
| Push Button / Jumper | 1 | Pull-up |

---

## 🔌 Power & Electrical

| Item | Qty | Notes |
|-----|-----|-------|
| 5V Power Supply | 1 | ≥ 2A recommended |
| Relay Module Power | 1 | Isolated preferred |
| Logic Level Converter | Optional | Required if relays are 5V-only |
| Flyback Diodes | Required | For motors and solenoids |
| Terminal Blocks | As needed | Safe wiring |

---

## 🌐 Network & Control

| Item | Qty | Purpose |
|-----|-----|---------|
| WiFi Network | 1 | ESP32 connectivity |
| MQTT Broker | 1 | Raspberry Pi / NAS / Server |
| MQTT Credentials | 1 | Defined in `secrets.h` |

---

## 🧵 Wiring & Accessories

| Item | Qty |
|-----|-----|
| Jumper Wires (M/F/F/F) | Assorted |
| Breadboard / PCB | 1 |
| Waterproof Enclosures | As needed |
| Cable Glands | As needed |
| Heat Shrink / Ferrules | As needed |

---

## 📦 Optional / Recommended Improvements

| Item | Reason |
|-----|--------|
| Capacitive Soil Sensor | Longer lifespan |
| Opto-isolated Relays | ESP32 protection |
| RTC Module | Time-based irrigation |
| Fuse / Circuit Breaker | Electrical safety |
| Current Sensor (ACS712) | Pump diagnostics |

---
ESP32 Pin Assignment

### Sensor Inputs

| Component            | ESP32 GPIO | Notes |
|----------------------|------------|-------|
| DHT22 #1             | GPIO14     | 4.7kΩ pull-up to 3.3V |
| DHT22 #2             | GPIO27     | 4.7kΩ pull-up to 3.3V |
| DS18B20 Data         | GPIO33     | 4.7kΩ pull-up to 3.3V |
| Soil Moisture (ADC)  | GPIO32     | Analog input |
| Water Flow Sensor    | GPIO25     | Interrupt capable |
| Reed Switch          | GPIO26     | Input with pull-up |
| Push Button          | GPIO34     | Input only pin |

---

### Relay Outputs

| Relay Channel | ESP32 GPIO | Function |
|--------------|------------|----------|
| IN1          | GPIO4      | Window Motor Open |
| IN2          | GPIO5      | Window Motor Close |
| IN3          | GPIO16     | Intake Flap |
| IN4          | GPIO17     | Exhaust Flap |
| IN5          | GPIO18     | Exhaust Fan |
| IN6          | GPIO19     | Intake Fan |
| IN7          | GPIO23     | Water Pump |
| IN8          | GPIO13     | Spare |

> ⚠️ Relay module inputs are **active LOW**

---

### I2C Display

| LCD Pin | ESP32 GPIO |
|--------|------------|
| SDA    | GPIO21 |
| SCL    | GPIO22 |
| VCC    | 5V (or 3.3V if supported) |
| GND    | GND |

---

## Power Connections

### ESP32
- Powered via **5V pin or USB**
- Internal regulator supplies **3.3V**

### Relay Module
- VCC → **5V**
- GND → **Common GND with ESP32**

### Sensors
- Powered from **3.3V** unless otherwise specified
- Grounds tied to **common ground**

---

## Pull-Up Resistors

| Sensor | Pull-Up |
|------|---------|
| DHT22 | 4.7kΩ between DATA and 3.3V |
| DS18B20 | 4.7kΩ between DATA and 3.3V |
| Reed Switch | Internal pull-up enabled | 

## 🔁 Application Flow

    BOOT --> SETUP[setup()]
    SETUP --> HW[Initialize hardware]
    SETUP --> SENS[Initialize sensors]
    SETUP --> DISP[Initialize display]
    SETUP --> NET[Optional WiFi / MQTT]

    LOOP --> MQTT[Maintain MQTT connection]

    MQTT --> S5[Every 5 seconds]
    S5 --> READ[Read sensors]
    READ --> DECIDE[Make control decisions]
    DECIDE --> ACT[Actuate relays / hardware]
    ACT --> PUB[Publish MQTT telemetry]

    MQTT --> S15[Every 1.5 seconds]
    S15 --> LCD[Update LCD display]
