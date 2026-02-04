#include <Arduino.h>
#include "irrigation.h"
#include "sensors.h"
#include "actuators.h"
#include "config.h"

void controlIrrigation() {
  static bool irrigating = false;

  if (!irrigating && soilPercent < SOIL_ON_THRESHOLD) {
    setPump(true);
    irrigating = true;
  }

  if (irrigating && soilPercent > SOIL_OFF_THRESHOLD) {
    setPump(false);
    irrigating = false;
  }
}
