#pragma once

extern float temp1, humid1, temp2, humid2;
extern float intakeTemp;
extern int soilMoisturePercent;
extern float flowRate;
extern bool doorClosed;

void initSensors();
void readClimate();
void readSoil();
void updateFlow();
void readDoor();
