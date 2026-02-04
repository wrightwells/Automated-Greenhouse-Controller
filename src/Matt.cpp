#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "mqtt.h"
#include "config.h"
#include "sensors.h"
#include "actuators.h"

// MQTT client is defined in main.cpp
extern PubSubClient client;

void mqttReconnect() {
  while (!client.connected()) {
    if (client.connect(MQTT_CLIENT_ID, mqtt_user, mqtt_password)) {
      client.subscribe(MQTT_CMD_TOPIC);
    } else {
      delay(2000); // short backoff, not ideal but acceptable here
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';

  // Example command handling
  if (strcmp(topic, MQTT_CMD_TOPIC) == 0) {
    if (strcmp((char*)payload, "pump_on") == 0) {
      setPump(true);
    }
    if (strcmp((char*)payload, "pump_off") == 0) {
      setPump(false);
    }
  }
}

void mqttPublish() {
  if (!client.connected()) return;

  client.publish(MQTT_TEMP_TOPIC, String(airTemp).c_str(), true);
  client.publish(MQTT_HUM_TOPIC,  String(airHumidity).c_str(), true);
  client.publish(MQTT_SOIL_TOPIC, String(soilPercent).c_str(), true);
  client.publish(MQTT_DOOR_TOPIC, doorOpen ? "OPEN" : "CLOSED", true);
}
