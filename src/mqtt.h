#pragma once
#include <PubSubClient.h>

extern PubSubClient client;

// Publish topics
#define MQTT_TEMP1        "stat/PSG/temp1"
#define MQTT_HUMID1       "stat/PSG/humid1"
#define MQTT_TEMP2        "stat/PSG/temp2"
#define MQTT_HUMID2       "stat/PSG/humid2"
#define MQTT_INTAKE_TEMP  "stat/PSG/intakeFanTemp"
#define MQTT_SOIL         "stat/PSG/soilM1"
#define MQTT_FLOW         "stat/PSG/flowRate"
#define MQTT_DOOR         "stat/PSG/doorStatus"
#define MQTT_PUMP_STATUS  "stat/PSG/pumpStatus"
#define MQTT_FAN1_STATUS  "stat/PSG/fan1Status"
#define MQTT_FAN2_STATUS  "stat/PSG/fan2Status"
#define MQTT_WIN1_STATUS  "stat/PSG/window1Status"
#define MQTT_WIN2_STATUS  "stat/PSG/window2Status"

// Subscribe topics
#define MQTT_WIN_OVERRIDE  "cmds/PSG/windowOverride"
#define MQTT_FAN1_OVERRIDE "cmds/PSG/fan1Override"
#define MQTT_FAN2_OVERRIDE "cmds/PSG/fan2Override"
#define MQTT_PUMP_OVERRIDE "cmds/PSG/pumpOverride"

void mqttReconnect();
void mqttPublish();
void mqttCallback(char*, byte*, unsigned int);
