#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "display.h"
#include "sensors.h"
#include "actuators.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void initDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void updateDisplay() {
  static uint8_t screen = 0;

  lcd.clear();

  switch (screen) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Air:");
      lcd.print(airTemp);
      lcd.print("C ");

      lcd.setCursor(0, 1);
      lcd.print("Hum:");
      lcd.print(airHumidity);
      lcd.print("%");
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Soil:");
      lcd.print(soilPercent);
      lcd.print("%");

      lcd.setCursor(0, 1);
      lcd.print("Pump:");
      lcd.print(pumpOn() ? "ON" : "OFF");
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Intake:");
      lcd.print(intakeTemp);
      lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Door:");
      lcd.print(doorOpen ? "OPEN" : "CLOSED");
      break;
  }

  screen = (screen + 1) % 3;
}
