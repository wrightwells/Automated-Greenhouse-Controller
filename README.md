    # Automated Greenhouse Controller

============================================================
                 AUTOMATED GREENHOUSE CONTROLLER
============================================================

This project implements:
- Climate monitoring (temperature, humidity, intake air)
- Automated ventilation
- Automated window control
- Soil moisture–based irrigation
- Flow rate monitoring
- MQTT-based remote control
- Local LCD status display
- Manual override via MQTT


------------------------------------------------------------
CORE CONTROLLER
------------------------------------------------------------

+---------------------------+-----+---------------------------+------------------------------------------+
| Item                      | Qty | Specification             | Notes                                    |
+---------------------------+-----+---------------------------+------------------------------------------+
| ESP32 Development Board   |  1  | ESP32-WROOM / DevKitC     | WiFi, interrupts, ADC, I2C required      |
+---------------------------+-----+---------------------------+------------------------------------------+


------------------------------------------------------------
SENSORS
------------------------------------------------------------

+-----------------------------------+-----+---------------------------+------------------------------------------+
| Item                              | Qty | Specification             | Used For                                 |
+-----------------------------------+-----+---------------------------+------------------------------------------+
| DHT22 Temp/Humidity Sensor        |  2  | AM2302                    | High + low mounted air temp & RH         |
| DS18B20 Temperature Sensor        |  1  | Waterproof recommended    | Intake air temperature                  |
| Soil Moisture Sensor (Analog)     |  1  | Capacitive preferred      | Uses ADC pin                            |
| Water Flow Sensor                 |  1  | Hall-effect (YF-S201)     | Irrigation flow rate                    |
| Magnetic Reed Switch              |  1  | Normally closed           | Door open / closed detection            |
+-----------------------------------+-----+---------------------------+------------------------------------------+


------------------------------------------------------------
ACTUATORS (RELAYS / LOADS)
------------------------------------------------------------

+---------------------------+-----+---------------------------+------------------------------------------+
| Item                      | Qty | Type                      | Controlled Device                        |
+---------------------------+-----+---------------------------+------------------------------------------+
| Relay Channels            |  8  | 5V or 3.3V compatible     |                                          |
| -> Window Actuators       |  2  | Directional / Timed       | Window 1 & 2                             |
| -> Intake Flap Actuator   |  1  | Motor / Solenoid          | Fresh air flap                          |
| -> Exhaust Flap Actuator  |  1  | Motor / Solenoid          | Exhaust flap                            |
| -> Exhaust Fan            |  1  | AC or DC                  | High temperature ventilation            |
| -> Intake Fan             |  1  | AC or DC                  | Fresh air intake                        |
| -> Water Pump             |  1  | AC or DC                  | Irrigation                              |
| -> Spare Relay            |  1  | --                        | Expansion                               |
+---------------------------+-----+---------------------------+------------------------------------------+

NOTE:
- Relays are ACTIVE LOW (HIGH = OFF)


------------------------------------------------------------
DISPLAY & INTERFACE
------------------------------------------------------------

+---------------------------+-----+---------------------------+
| Item                      | Qty | Specification             |
+---------------------------+-----+---------------------------+
| LCD Display               |  1  | 16x2 I2C (0x3F)           |
| Push Button / Jumper      |  1  | Pull-up                   |
+---------------------------+-----+---------------------------+


------------------------------------------------------------
POWER & ELECTRICAL
------------------------------------------------------------

+---------------------------+-----+------------------------------------------+
| Item                      | Qty | Notes                                    |
+---------------------------+-----+------------------------------------------+
| 5V Power Supply           |  1  | >= 2A recommended                        |
| Relay Module Power        |  1  | Isolated preferred                       |
| Logic Level Converter     | Opt | Required if relays are 5V-only           |
| Flyback Diodes            | Req | For motors / solenoids                   |
| Terminal Blocks           | Var | Safe wiring                              |
+---------------------------+-----+------------------------------------------+


------------------------------------------------------------
NETWORK & CONTROL
------------------------------------------------------------

+---------------------------+-----+------------------------------------------+
| Item                      | Qty | Purpose                                  |
+---------------------------+-----+------------------------------------------+
| WiFi Network              |  1  | ESP32 connectivity                      |
| MQTT Broker               |  1  | Raspberry Pi / NAS / Server             |
| MQTT Credentials          |  1  | Defined in secrets.h                    |
+---------------------------+-----+------------------------------------------+


------------------------------------------------------------
WIRING & ACCESSORIES
------------------------------------------------------------

+---------------------------+-----+
| Item                      | Qty |
+---------------------------+-----+
| Jumper Wires (M/F/F/F)    | Var |
| Breadboard / PCB          |  1  |
| Waterproof Enclosures     | Var |
| Cable Glands              | Var |
| Heat Shrink / Ferrules    | Var |
+---------------------------+-----+


------------------------------------------------------------
OPTIONAL / RECOMMENDED IMPROVEMENTS
------------------------------------------------------------

+---------------------------+------------------------------------------+
| Item                      | Reason                                   |
+---------------------------+------------------------------------------+
| Capacitive Soil Sensor    | Longer lifespan                          |
| Opto-isolated Relays      | ESP32 protection                        |
| RTC Module                | Time-based irrigation                   |
| Fuse / Circuit Breaker    | Safety                                  |
| Current Sensor (ACS712)   | Pump diagnostics                        |
+---------------------------+------------------------------------------+


------------------------------------------------------------
APPLICATION FLOW
------------------------------------------------------------

BOOT
 |
 +-- setup()
     |
     +-- initialize hardware
     +-- initialize sensors
     +-- initialize display
     +-- optional WiFi / MQTT connection


LOOP
 |
 +-- maintain MQTT connection
 |
 +-- every 5 seconds:
 |     |
 |     +-- read sensors
 |     +-- make control decisions
 |     +-- actuate relays / hardware
 |     +-- publish MQTT telemetry
 |
 +-- every 1.5 seconds:
       |
       +-- update LCD display
