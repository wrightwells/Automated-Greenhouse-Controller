#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

#include "secrets.h"
#include "config.h"
#include "pins.h"
#include "mqtt.h"
#include "sensors.h"
#include "actuators.h"
#include "display.h"
#include "irrigation.h"

// Globals
WiFiClient espClient;
PubSubClient client(espClient);

// Timing
unsigned long lastMainLoop = 0;
unsigned long lastLCD = 0;

void setup() {
  Serial.begin(9600);

  initRelays();
  initSensors();
  initDisplay();

  pinMode(PIN_WIFI_SKIP, INPUT_PULLUP);

  if (digitalRead(PIN_WIFI_SKIP)) {
    WiFi.begin(ssid, password);
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqttCallback);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !client.connected())
    mqttReconnect();

  client.loop();

  if (millis() - lastMainLoop > MAIN_LOOP_INTERVAL_MS) {
    lastMainLoop = millis();

    readClimate();
    readSoil();
    readDoor();
    updateFlow();
    controlIrrigation();
    mqttPublish();
  }

  if (millis() - lastLCD > LCD_INTERVAL_MS) {
    lastLCD = millis();
    updateDisplay();
  }
}
