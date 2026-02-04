#include <Arduino.h>
#include "actuators.h"
#include "pins.h"

// Keep track of the last-known actuator states locally so callers
// can query without reading hardware pins.
static bool pumpState = false;
static bool fanState  = false;

// Initialize relay output pins and ensure actuators start OFF.
void initRelays() {
  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);

  // Drive relays LOW to disable pump and fan at startup (active HIGH assumed).
  digitalWrite(PIN_PUMP, LOW);
  digitalWrite(PIN_FAN, LOW);
}

// Turn pump ON/OFF and update cached state.
// Parameter: on - true to enable pump, false to disable.
void setPump(bool on) {
  pumpState = on;
  // Write the appropriate level to the relay pin (active HIGH).
  digitalWrite(PIN_PUMP, on ? HIGH : LOW);
}

// Turn fan ON/OFF and update cached state.
// Parameter: on - true to enable fan, false to disable.
void setFan(bool on) {
  fanState = on;
  // Write the appropriate level to the relay pin (active HIGH).
  digitalWrite(PIN_FAN, on ? HIGH : LOW);
}

// Return whether pump was last set ON via `setPump()`.
bool pumpOn() {
  return pumpState;
}

// Return whether fan was last set ON via `setFan()`.
bool fanOn() {
  return fanState;
}
