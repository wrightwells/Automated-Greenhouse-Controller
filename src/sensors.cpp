#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "sensors.h"
#include "pins.h"
#include "config.h"

// ---- Globals exposed via sensors.h ----
float airTemp = 0;
float airHumidity = 0;
float intakeTemp = 0;
int soilPercent = 0;
bool doorOpen = false;
volatile unsigned long flowPulses = 0;
float flowRate = 0;

// ---- Sensor objects ----
DHT dht(PIN_DHT, DHT_TYPE);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

void IRAM_ATTR flowISR() {
  flowPulses++;
}

void initSensors() {
  dht.begin();
  ds18b20.begin();

  pinMode(PIN_DOOR, INPUT_PULLUP);
  pinMode(PIN_FLOW, INPUT_PULLUP);
  attachInterrupt(PIN_FLOW, flowISR, RISING);
}

void readClimate() {
  airTemp = dht.readTemperature();
  airHumidity = dht.readHumidity();

  ds18b20.requestTemperatures();
  intakeTemp = ds18b20.getTempCByIndex(0);
}

void readSoil() {
  int raw = analogRead(PIN_SOIL);
  soilPercent = map(raw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
}

void readDoor() {
  doorOpen = digitalRead(PIN_DOOR) == LOW;
}

void updateFlow() {
  static unsigned long lastCalc = 0;
  unsigned long now = millis();

  if (now - lastCalc >= 1000) {
    noInterrupts();
    unsigned long pulses = flowPulses;
    flowPulses = 0;
    interrupts();

    flowRate = pulses * FLOW_CALIBRATION;
    lastCalc = now;
  }
}
