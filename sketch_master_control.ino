/*****************************************************
 * Includes / Libraries
 *****************************************************/
#include <Wire.h>                 // I2C communication
#include <ESP32Ping.h>            // Ping support (used to test MQTT server reachability)
#include <PubSubClient.h>         // MQTT client
#include <SPI.h>
#include <DHT.h>                  // DHT temperature & humidity sensors
#include <LiquidCrystal_I2C.h>    // I2C LCD
#include <secrets.h>              // WiFi + MQTT credentials
#include <OneWire.h>              // OneWire bus (DS18B20)
#include <DallasTemperature.h>    // DS18B20 temperature sensor

/*****************************************************
 * Constants / Configuration
 *****************************************************/

// DHT sensor type
#define DHTTYPE DHT22

// Delay before windows can change state again (anti-flutter)
long WindowFlutter = 60000;

// Soil moisture calibration values
#define AirCalibrationValue 3231
#define WaterCalibrationValue 1390

// Soil moisture thresholds
#define soilDryLevel 79
#define soilWetLevel 83

// WiFi timeout
#define CONNECTION_TIMEOUT 10

/*****************************************************
 * MQTT Topics
 *****************************************************/

// Publish topics
#define mqttTemp1 "stat/PSG/temp1"
#define mqttHumid1 "stat/PSG/humid1"
#define mqttTemp2 "stat/PSG/temp2"
#define mqttHumid2 "stat/PSG/humid2"
#define mqttIntTemp "stat/PSG/intakeFanTemp"
#define mqttSoilMoisture1 "stat/PSG/soilM1"
#define mqttSoilMoisture2 "stat/PSG/soilM2"
#define mqttWindow1Status "stat/PSG/window1Status"
#define mqttWindow2Status "stat/PSG/window2Status"
#define mqttFan1Status "stat/PSG/fan1Status"
#define mqttFan2Status "stat/PSG/fan2Status"
#define mqttPumpStatus "stat/PSG/pumpStatus"
#define mqttFlowRate "stat/PSG/flowRate"
#define mqttDoorStatus "stat/PSG/doorStatus"

// Subscribe topics (control / overrides)
#define mqttWindowOverride "cmds/PSG/windowOverride"
#define mqttFan1Override "cmds/PSG/fan1Override"
#define mqttFan2Override "cmds/PSG/fan2Override"
#define mqttPumpControl "cmds/PSG/pumpOverride"
#define mqttFanTemp "ctrl/PSG/VentilationControl"

/*****************************************************
 * Temperature Control Thresholds
 *****************************************************/
#define Open_WindowsTemp1 21
#define Open_WindowsTemp2 23
#define InFan_Temp 29
#define ExFan_Temp 35

/*****************************************************
 * GPIO Pin Definitions
 *****************************************************/
#define relayOutputPin1 19   // Window 1
#define relayOutputPin2 15   // Window 2
#define relayOutputPin3 4    // Intake flap
#define relayOutputPin4 18   // Exhaust flap
#define relayOutputPin5 5    // Exhaust fan
#define relayOutputPin6 16   // Intake fan
#define relayOutputPin7 17   // Pump
#define relayOutputPin8 23   // Spare
#define doorSensorPin 32
#define skipOverWifiPin 33
#define SoilMoisturePin 35
#define DHT1Pin 13
#define DHT2Pin 14
#define IntAirTempPin 25     // DS18B20
#define FlowSensorPin 27

/*****************************************************
 * Text Constants
 *****************************************************/
String statusOn = "On ";
String statusOff = "Off ";
String winO = "Open";
String winC = "Closed";

/*****************************************************
 * Flow Sensor Variables
 *****************************************************/
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;             // 1 second sample window
float calibrationFactor = 4.5;
volatile byte pulseCount;        // Updated in interrupt
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

/*****************************************************
 * Runtime State Variables
 *****************************************************/
int soilMoisturePercent = 0;
int doorStatus;
int windowOverrideStatus = 0;
int fan1OverrideStatus = 0;
int fan2OverrideStatus = 0;
int pumpOverrideStatus = 0;
int window1Status = 0;
int window2Status = 0;
int fan1Status = 0;
int fan2Status = 0;
int pumpStatus = 0;
long loopDelay = 0;
long windowPause = 0;
byte wifiSkipState;

/*****************************************************
 * Climate Variables
 *****************************************************/
float temp1 = 0;
float humid1 = 0;
float temp2 = 0;
float humid2 = 0;
float intakeTemp = 0;
float IntakeCalib = 0.7;
int cyclei = 0;

/*****************************************************
 * Display Status Strings
 *****************************************************/
String fan1S, fan2S, pumpS, win1S, win2S, doorTStatus;

/*****************************************************
 * Objects
 *****************************************************/
WiFiClient espClient;
PubSubClient client(espClient);

DHT dht1(DHT1Pin, DHTTYPE);
DHT dht2(DHT2Pin, DHTTYPE);

LiquidCrystal_I2C lcd(0x3F, 16, 2);

OneWire oneWire(IntAirTempPin);
DallasTemperature sensors(&oneWire);

/*****************************************************
 * Flow Sensor Interrupt
 *****************************************************/
void IRAM_ATTR pulseCounter() {
  pulseCount++;  // Increment on every pulse
}

/*****************************************************
 * Setup
 *****************************************************/
void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  dht1.begin();
  dht2.begin();
  sensors.begin();

  // Configure relay outputs
  pinMode(relayOutputPin1, OUTPUT);
  pinMode(relayOutputPin2, OUTPUT);
  pinMode(relayOutputPin3, OUTPUT);
  pinMode(relayOutputPin4, OUTPUT);
  pinMode(relayOutputPin5, OUTPUT);
  pinMode(relayOutputPin6, OUTPUT);
  pinMode(relayOutputPin7, OUTPUT);
  pinMode(relayOutputPin8, OUTPUT);

  pinMode(skipOverWifiPin, INPUT_PULLUP);
  pinMode(FlowSensorPin, INPUT_PULLUP);
  pinMode(doorSensorPin, INPUT_PULLUP);

  // Default all relays OFF (active LOW)
  digitalWrite(relayOutputPin1, HIGH);
  digitalWrite(relayOutputPin2, HIGH);
  digitalWrite(relayOutputPin3, HIGH);
  digitalWrite(relayOutputPin4, HIGH);
  digitalWrite(relayOutputPin5, HIGH);
  digitalWrite(relayOutputPin6, HIGH);
  digitalWrite(relayOutputPin7, HIGH);
  digitalWrite(relayOutputPin8, HIGH);

  // Optional WiFi skip
  wifiSkipState = digitalRead(skipOverWifiPin);
  if (wifiSkipState == HIGH) {
    wificonnect();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    reconnectmqtt();
  }

  // Flow sensor setup
  attachInterrupt(digitalPinToInterrupt(FlowSensorPin), pulseCounter, FALLING);
}

/*****************************************************
 * Main Loop
 *****************************************************/
void loop() {

  // Maintain MQTT connection
  if (digitalRead(skipOverWifiPin) == HIGH && !client.connected()) {
    reconnectmqtt();
  }

  sensors.requestTemperatures();

  long now = millis();
  if (now - loopDelay > 5000) {
    loopDelay = now;

    getDoorStatus();
    getClimateValues();
    controlWindows();
    controlVentilation();
    getSoilMoistureValues();
    controlIrrigation();
    publishMqtt();
    publishLCDLoopStatusData(cyclei, pumpStatus, fan1Status, fan2Status, window1Status, window2Status);

    cyclei = (cyclei + 1) % 4;
  }

  // Process flow sensor once per second
  if (millis() - previousMillis > interval) {
    processFlowRate();
  }
}
