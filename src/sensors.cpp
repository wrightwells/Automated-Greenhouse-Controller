#include <Arduino.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "sensors.h"
#include "pins.h"
#include "config.h"

// ---- Globals exposed via sensors.h ----
// These are updated by the sensor read functions and used elsewhere
// in the program to make control decisions or publish telemetry.
float airTemp = 0;        // Ambient air temperature from DHT sensor (°C)
float airHumidity = 0;    // Relative humidity from DHT sensor (%RH)
float intakeTemp = 0;    // Intake temperature from DS18B20 (°C)
int soilPercent = 0;     // Soil moisture mapped to 0-100%
bool doorOpen = false;   // Door/micro switch state (true == open)
// Flow meter pulse counter (incremented from ISR) — volatile since ISR updates it.
volatile unsigned long flowPulses = 0;
float flowRate = 0;      // Calculated flow rate (units per second based on FLOW_CALIBRATION)

// ---- Sensor objects ----
// DHT for air temp/humidity, OneWire + Dallas for DS18B20 intake probe.
DHT dht(PIN_DHT, DHT_TYPE);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

// Interrupt Service Routine for flow meter pulses.
// Marked IRAM_ATTR for stable ISR placement on platforms that require it.
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

// Initialize sensors and configure input pins/interrupts.
// Uses INPUT_PULLUP for door and flow to provide defined idle states.

void readClimate() {
  // Read air temperature and humidity from DHT sensor.
  airTemp = dht.readTemperature();
  airHumidity = dht.readHumidity();

  // Trigger DS18B20 temperature conversion and read the first device.
  ds18b20.requestTemperatures();
  intakeTemp = ds18b20.getTempCByIndex(0);
}

void readSoil() {
  // Read raw analog value from soil moisture sensor and map to 0-100%.
  int raw = analogRead(PIN_SOIL);
  soilPercent = map(raw, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);
}

void readDoor() {
  // Using INPUT_PULLUP: LOW means the door switch is closed/open depending on wiring.
  // Here we treat LOW as 'open' (active low switch).
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
    // Calculate flow rate using the number of pulses counted in the last second
    // and a calibration constant defined in config.h.
    flowRate = pulses * FLOW_CALIBRATION;
    lastCalc = now;
  }
}
