#pragma once
#include <PubSubClient.h>

extern PubSubClient client;

// Publish topics
#define MQTT_TEMP1        "stat/AGC/temp1"
#define MQTT_HUMID1       "stat/AGC/humid1"
#define MQTT_TEMP2        "stat/AGC/temp2"
#define MQTT_HUMID2       "stat/AGC/humid2"
#define MQTT_INTAKE_TEMP  "stat/AGC/intakeFanTemp"
#define MQTT_SOIL         "stat/AGC/soilM1"
#define MQTT_FLOW         "stat/AGC/flowRate"
#define MQTT_DOOR         "stat/AGC/doorStatus"
#define MQTT_PUMP_STATUS  "stat/AGC/pumpStatus"
#define MQTT_FAN1_STATUS  "stat/AGC/fan1Status"
#define MQTT_FAN2_STATUS  "stat/AGC/fan2Status"
#define MQTT_WIN1_STATUS  "stat/AGC/window1Status"
#define MQTT_WIN2_STATUS  "stat/AGC/window2Status"

// Subscribe topics
#define MQTT_WIN_OVERRIDE  "cmds/AGC/windowOverride"
#define MQTT_FAN1_OVERRIDE "cmds/AGC/fan1Override"
#define MQTT_FAN2_OVERRIDE "cmds/AGC/fan2Override"
#define MQTT_PUMP_OVERRIDE "cmds/AGC/pumpOverride"

void mqttReconnect();
void mqttPublish();
void mqttCallback(char*, byte*, unsigned int);
