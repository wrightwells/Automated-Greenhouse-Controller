/*****************************************************
 * Core Arduino + ESP32 system includes
 *****************************************************/
#include <Arduino.h>          // Core Arduino definitions (millis, pinMode, Serial, etc.)
#include <WiFi.h>             // ESP32 WiFi stack
#include <ESP32Ping.h>        // Optional network reachability checks
#include <DHT.h>              // DHT temperature/humidity sensors
#include <OneWire.h>          // OneWire bus (DS18B20)
#include <DallasTemperature.h>// DS18B20 temperature handling
#include <LiquidCrystal_I2C.h>// I2C LCD display

/*****************************************************
 * Project-specific headers
 *****************************************************/
#include "secrets.h"     // WiFi + MQTT credentials (kept out of repo)
#include "config.h"      // Timings, thresholds, calibration values
#include "pins.h"        // GPIO pin mappings
#include "mqtt.h"        // MQTT connect, publish, callbacks
#include "sensors.h"     // Sensor initialisation & reading
#include "actuators.h"   // Relay / output control
#include "display.h"     // LCD handling
#include "irrigation.h"  // Irrigation decision logic

/*****************************************************
 * Global network objects
 *****************************************************/

// WiFiClient handles the underlying TCP connection
WiFiClient espClient;

// PubSubClient wraps MQTT protocol on top of WiFiClient
// This object is shared across modules using `extern`
PubSubClient client(espClient);

/*****************************************************
 * Timing control variables
 *****************************************************/

// Timestamp of last full control cycle
unsigned long lastMainLoop = 0;

// Timestamp of last LCD update
unsigned long lastLCD = 0;


/*****************************************************
 * Runtime Debug Tracing
 *****************************************************/

// Master debug switch
#define DEBUG_ENABLED 1   // ← set to 0 to compile out all logs

#if DEBUG_ENABLED

  #define DBG_BEGIN(baud) Serial.begin(baud)

  #define DBG_PRINT(msg)            Serial.print(msg)
  #define DBG_PRINTLN(msg)          Serial.println(msg)

  // Timestamped log
  #define DBG_LOG(tag, msg)         \
    do {                            \
      Serial.print("[");            \
      Serial.print(millis());       \
      Serial.print("][");           \
      Serial.print(tag);            \
      Serial.print("] ");           \
      Serial.println(msg);          \
    } while (0)

#else

  // Compile-time removal (zero cost)
  #define DBG_BEGIN(baud)
  #define DBG_PRINT(msg)
  #define DBG_PRINTLN(msg)
  #define DBG_LOG(tag, msg)

#endif

/*****************************************************
 * setup()
 * Runs once at boot.
 * Responsible ONLY for initialisation.
 *****************************************************/
void setup() {

  // Initialise serial output for debugging
  DBG_BEGIN(9600);
  DBG_LOG("BOOT", "System starting");

  // Initialise all relay outputs and ensure they start OFF
  initRelays();
  DBG_LOG("ACT", "Relays initialised");

  // Initialise sensors (DHTs, DS18B20, ADCs, interrupts)
  initSensors();
  DBG_LOG("SENSOR", "Sensors initialised");

   // Initialise LCD (I2C, backlight, clear screen)
  initDisplay();
  DBG_LOG("LCD", "Display initialised");

  // Configure WiFi skip pin (hardware override)
  // HIGH = normal operation
  // LOW  = skip WiFi/MQTT entirely
  pinMode(PIN_WIFI_SKIP, INPUT_PULLUP);

  // Only attempt WiFi/MQTT if hardware allows it
  if (digitalRead(PIN_WIFI_SKIP)) {
    DBG_LOG("WIFI", "WiFi enabled by hardware");

    // Begin WiFi connection (non-blocking)
    WiFi.begin(ssid, password);
     // Configure MQTT broker and message callback
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);

  } else {
    DBG_LOG("WIFI", "WiFi skipped by hardware");
  }
}




/*****************************************************
 * loop()
 * Runs continuously after setup().
 * Designed to be:
 *  - non-blocking
 *  - watchdog-safe
 *  - MQTT-friendly
 *****************************************************/
void loop() {

  /***************************************************
   * 1. Maintain MQTT connectivity
   ***************************************************/
  // If WiFi is connected but MQTT is not, try to reconnect
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    mqttReconnect();
  }

  // REQUIRED for MQTT to function:
  // - processes incoming messages
  // - sends keep-alives
  // - triggers mqttCallback()
  client.loop();

  /***************************************************
   * 2. Main control loop (slow cycle)
   * Runs every MAIN_LOOP_INTERVAL_MS (e.g. 5 seconds)
   ***************************************************/
  if (millis() - lastMainLoop > MAIN_LOOP_INTERVAL_MS) {
    lastMainLoop = millis();

    DBG_LOG("LOOP", "Main cycle start");

    // Read temperature & humidity sensors
    readClimate();

    // Read soil moisture and convert to %
    readSoil();

    // Read door reed switch state
    readDoor();

    // Calculate water flow rate from pulse counter
    updateFlow();

    // Decide whether irrigation pump should run
    controlIrrigation();

    // Publish all sensor readings and states via MQTT
    mqttPublish();

  
    DBG_LOG("LOOP", "Main cycle end");
  }

  /***************************************************
   * 3. LCD update loop (UI only)
   * Runs more frequently than the main logic
   ***************************************************/
  if (millis() - lastLCD > LCD_INTERVAL_MS) {
    lastLCD = millis();

    // Cycle LCD screens:
    // climate → soil/intake → fans/pump → windows
    updateDisplay();
  }
}
