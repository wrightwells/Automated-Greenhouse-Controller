#include <Arduino.h>
#include "actuators.h"
#include "pins.h"

// Internal state
static bool pumpState = false;
static bool fanState  = false;

void initRelays() {
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);

  digitalWrite(PIN_PUMP, LOW);
  digitalWrite(PIN_FAN, LOW);
}

void setPump(bool on) {
  pumpState = on;
  digitalWrite(PIN_PUMP, on ? HIGH : LOW);
}

void setFan(bool on) {
  fanState = on;
  digitalWrite(PIN_FAN, on ? HIGH : LOW);
}

bool pumpOn() {
  return pumpState;
}

bool fanOn() {
  return fanState;
}
