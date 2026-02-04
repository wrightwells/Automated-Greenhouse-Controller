#include <Arduino.h>
#include "irrigation.h"
#include "sensors.h"
#include "actuators.h"
#include "config.h"

void controlIrrigation() {
  // Track whether the pump is currently running to implement hysteresis
  // and avoid rapidly toggling the relay when soilPercent hovers around
  // the threshold values.
  static bool irrigating = false;

  // Start irrigation when soil moisture falls below the ON threshold
  // and we are not already irrigating.
  if (!irrigating && soilPercent < SOIL_ON_THRESHOLD) {
    setPump(true);
    irrigating = true;
  }

  // Stop irrigation when soil moisture rises above the OFF threshold
  // while irrigating. Using separate ON/OFF thresholds provides hysteresis.
  if (irrigating && soilPercent > SOIL_OFF_THRESHOLD) {
    setPump(false);
    irrigating = false;
  }
}
