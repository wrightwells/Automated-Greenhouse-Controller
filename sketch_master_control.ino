#include <Wire.h>
#include <ESP32Ping.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
//#include <Math.h>
#include <secrets.h> //change to mysecrets as per repository
#include <OneWire.h>
#include <DallasTemperature.h>

//const variables

#define DHTTYPE DHT22               // DHT 22
long WindowFlutter = 60000;         //time break before windows reoperate after a state change
#define AirCalibrationValue 3231    //Soil moisture air calib number
#define WaterCalibrationValue 1390  //Soil moisture water calib number
#define soilDryLevel 79
#define soilWetLevel 83
#define CONNECTION_TIMEOUT 10
//define MQTT variables

#define mqttTemp1 "stat/PSG/temp1"           //Temperature
#define mqttHumid1 "stat/PSG/humid1"         //Humidity
#define mqttTemp2 "stat/PSG/temp2"           //Temperature
#define mqttHumid2 "stat/PSG/humid2"         //Humidity
#define mqttIntTemp "stat/PSG/mqttIntTemp"   //Humidity
#define mqttSoilMoisture1 "stat/PSG/soilM1"  //Soil Moisture 1
#define mqttSoilMoisture2 "stat/PSG/soilM2"  //Soil Moisture 2
#define mqttWindowOverride "cmds/PSG/windowOverride"
#define mqttWindow1Status "stat/PSG/window1Status"  //status of the window/open or closed
#define mqttWindow2Status "stat/PSG/window2Status"  //status of the window/open or closed
#define mqttFan1Status "stat/PSG/Fan1Status"
#define mqttFan2Status "stat/PSG/Fan2Status"
#define mqttFan1Override "cmds/PSG/fan1Override"
#define mqttFan2Override "cmds/PSG/fan2Override"
#define mqttPumpControl "cmds/PSG/pumpOverride"
#define mqttPumpStatus "stat/PSG/pumpStatus"
#define mqttFlowRate "stat/PSG/flowRate"
#define mqttDoorStatus "stat/PSG/doorStatus"
//mqtt Receive
#define mqttFanTemp "ctrl/PSG/VentilationControl"
//
//temperature Controls
#define Open_WindowsTemp1 21
#define Open_WindowsTemp2 23
#define InFan_Temp 25  //1 In Intake Fan
#define ExFan_Temp 27  //2 Ex Exhaust Fan
//

//define GPIO pins
#define relayOutputPin1 19  // Window 1
#define relayOutputPin2 15  // Window 2
#define relayOutputPin3 4  // Flap intake
#define relayOutputPin4 18  // Flap Exhaustt
#define relayOutputPin5 5   // Fan Exhaus
#define relayOutputPin6 16  // Fan Intake
#define relayOutputPin7 17  // Pump
#define relayOutputPin8 23  // TBA
#define doorSensorPin 32    //Door reed switch
#define skipOverWifiPin 33  //Run without having to connect ot wifi
#define SoilMoisturePin 35  //soil moisture
#define DHT1Pin 13          //DHT High Mounted
#define DHT2Pin 14          //DHT Low Mounted
#define IntAirTempPin 25    //Intake fan air temp
#define FlowSensorPin  27   //water flow rate
//text definitions
String statusOn = String("On ");
String statusOff = String("Off ");
String winO = String("Open"); 
String winC = String("Closed");
//flowsensor settings
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
//declare other non const variables
int soilMoisturePercent = 0;
int doorStatus;
int windowOverrideStatus = 0;
int fan1OverrideStatus = 0;
int fan2OverrideStatus = 0;
int pumpOverrideStatus = 0;
int window1Status = 0;  //0= closed 1= open
int window2Status = 0;  //0= closed 1= open
int fan1Status = 0;
int fan2Status = 0;
int pumpStatus = 0;
long loopDelay = 0;
long windowPause = 0;
byte wifiSkipState;
//Climate Variables
float temp1 = 0;
int newTemp = 0;
float humid1 = 0;
float temp2 = 0;
float humid2 = 0;
float intakeTemp = 0;
int cyclei = 0;
float IntakeCalib=0.7;
String fan1S;
String fan2S;
String pumpS;
String win1S;
String win2S;
String doorTStatus;
//

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht1(DHT1Pin, DHTTYPE);          // Initialize DHT High sensor.
DHT dht2(DHT2Pin, DHTTYPE);          // Initialize DHT Low sensor.
LiquidCrystal_I2C lcd(0x3F, 16, 2);  //initialise LCD
OneWire oneWire(IntAirTempPin);				// initialise DS18B20
DallasTemperature sensors(&oneWire);

void IRAM_ATTR pulseCounter() //this flow rate function must appear here before the setup
{
  pulseCount++;
}
void setup() {
	Serial.begin(9600);
	lcd.init();
	lcd.clear();
	lcd.backlight();
	dht1.begin();
	dht2.begin();
	sensors.begin();  //onewire ds18b20
	//set window pin modes
	pinMode(relayOutputPin1, OUTPUT);  // Window 1
	pinMode(relayOutputPin2, OUTPUT);  // Window 2
	pinMode(relayOutputPin3, OUTPUT);  // Fan Intake
	pinMode(relayOutputPin4, OUTPUT);  // Fan Exhaust
	pinMode(relayOutputPin5, OUTPUT);  // Pump
	pinMode(relayOutputPin6, OUTPUT);  // TBA
	pinMode(relayOutputPin7, OUTPUT);  // Flap Exhaust
	pinMode(relayOutputPin8, OUTPUT);  // Flap intake
	pinMode(skipOverWifiPin, INPUT_PULLUP);
  pinMode(DHT1Pin,INPUT);
  pinMode(FlowSensorPin, INPUT_PULLUP);
  pinMode(doorSensorPin, INPUT_PULLUP); 
	//set relays as open (devices off) on startup
	digitalWrite(relayOutputPin1, HIGH);
	digitalWrite(relayOutputPin2, HIGH);
	digitalWrite(relayOutputPin3, HIGH);
	digitalWrite(relayOutputPin4, HIGH);
	digitalWrite(relayOutputPin5, HIGH);
	digitalWrite(relayOutputPin6, HIGH);
	digitalWrite(relayOutputPin7, HIGH);
	digitalWrite(relayOutputPin8, HIGH);
	wifiSkipState = digitalRead(skipOverWifiPin);
	if (wifiSkipState == HIGH) {
		wificonnect();
		client.setServer(mqtt_server, 1883);
		client.setCallback(callback);
		reconnectmqtt();
	}
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(FlowSensorPin), pulseCounter, FALLING);

}

void loop() {


	wifiSkipState = digitalRead(skipOverWifiPin);
	if (wifiSkipState == HIGH) {
		if (!client.connected()) {
			reconnectmqtt();
		}
	}
	sensors.requestTemperatures();
	long loopnow = millis();

	if (loopnow - loopDelay > 5000) {
		loopDelay = loopnow;
    getDoorStatus();
		getClimateValues();
		//openCloseFanFlaps();
		controlWindows();
		controlVentilation();
		getSoilMoistureValues();
		controlIrrigation();
		publishMqtt();
		publishLCDLoopStatusData(cyclei,pumpStatus,fan1Status,fan2Status,window1Status,window2Status);
		Serial.println();
		Serial.print("Celsius temperature: ");
		Serial.print(sensors.getTempCByIndex(0));  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    cyclei++;
    if(cyclei == 4)
      cyclei=0;
	}
  currentMillis = millis();
  if (currentMillis - previousMillis > interval)    // Only process flowratecounters once per second
    processFlowRate();
}
void getDoorStatus() {
  int digitalVal = digitalRead(doorSensorPin);
  if (digitalVal == LOW) {
    
    Serial.println();
		Serial.print("Door is Closed");
    Serial.println();
    doorStatus=1;
    doorTStatus = winC;
  }
  else {
        Serial.println();
		Serial.print("Door is Open");
    Serial.println();
    doorStatus=0;
    doorTStatus = winO;
  }
}
void processFlowRate() {

    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
}

void publishMqtt() {

	// publish (send) the values to the RPi
	client.publish(mqttTemp1, String(temp1).c_str(), true);
	client.publish(mqttHumid1, String(humid1).c_str(), true);
	client.publish(mqttTemp2, String(temp2).c_str(), true);
	client.publish(mqttHumid2, String(humid2).c_str(), true);
	client.publish(mqttSoilMoisture1, String(soilMoisturePercent).c_str(), true);
	client.publish(mqttIntTemp, String(intakeTemp).c_str(), true);
	client.publish(mqttWindow1Status, String(window1Status).c_str(), true);
	client.publish(mqttWindow2Status, String(window2Status).c_str(), true);
	client.publish(mqttFan2Status, String(fan2Status).c_str(), true);
	client.publish(mqttFan1Status, String(fan1Status).c_str(), true);
	client.publish(mqttPumpStatus, String(pumpStatus).c_str(), true);
	client.publish(mqttFlowRate, String(flowRate).c_str(), true);
  client.publish(mqttDoorStatus, String(doorTStatus).c_str(), true);
	client.loop();
}
void topicsSubscribe() {
	client.subscribe(mqttFanTemp);
}
//declare lcd variables
void PublishLCD(String lcdLine1, String lcdLine2, int LcdDelay) {
	lcd.clear();
	lcd.setCursor(0, 0);  //Set cursor to character 2 on line 0
	lcd.print(lcdLine1);
	lcd.setCursor(0, 1);  //Set cursor to character 2 on line 0
	lcd.print(lcdLine2);
	delay(LcdDelay);
	//Serial.println(lcdLine1);
	//Serial.println(lcdLine2);
}

void wificonnect() {

	// begin Wifi connect
	Serial.print("Connecting to ");

	PublishLCD("Trying to", "connect to wifi.", 0);
	Serial.println(ssid);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(2000);
	WiFi.begin(ssid, password);
	int timeout_counter = 0;
	lcd.clear();
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");

		lcd.print(".");
		timeout_counter++;
		if (timeout_counter >= CONNECTION_TIMEOUT * 5) {
			PublishLCD("Giving up :(", "Resetting ESP32", 3000);
			//lcd.print("Skipping Wifi");
			//delay(3000);
			ESP.restart();
		}
	}

	Serial.println("");
	Serial.println("WiFi connected");
	String myip = WiFi.localIP().toString();
	PublishLCD("Wifi Connected", myip, 0);
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	//end Wifi connect

	Serial.print("Pinging ip ");
	Serial.println(mqtt_server);
	//are we connected to the Rpi (mqtt server)
	if (Ping.ping(mqtt_server)) {
		Serial.println("Success!!");
		PublishLCD("Conex to MQTT", "", 1000);
	} else {
		Serial.println("Error :(");
		PublishLCD("Sorry MQTT #FAIL", "", 1000);
	}
}
void reconnectmqtt() {
	// Loop until we're reconnected
	int counter = 0;
	//
	while (!client.connected()) {
		if (counter == 5) {
			ESP.restart();
		}
		counter += 1;
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect

		if (client.connect("esp32server2", mqtt_user, mqtt_pass)) {
			Serial.println("connected");
			topicsSubscribe();
			client.subscribe(mqttWindowOverride);
			client.subscribe(mqttFan1Override);
			client.subscribe(mqttFan2Override);
			client.subscribe(mqttPumpControl);
			client.subscribe(mqttFanTemp);
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}
void callback(char* topic, byte* message, unsigned int length) {
	Serial.print("Message arrived on topic: ");
	Serial.print(topic);
	Serial.print(". Message: ");
	String messageTemp;

	for (int i = 0; i < length; i++) {
		Serial.print((char)message[i]);
		messageTemp += (char)message[i];
	}
	Serial.println();

	//Let's Override the status of the windows
	if (String(topic) == mqttWindowOverride) {
		Serial.print("Changing output to ");
		if (messageTemp == "on") {
			Serial.println("Override on");
			if (window1Status == 0) {
				changeWindowState(1, relayOutputPin1);
				changeWindowState(1, relayOutputPin2);
				windowOverrideStatus = 1;
			} else {
				changeWindowState(0, relayOutputPin1);
				changeWindowState(0, relayOutputPin2);
				windowOverrideStatus = 1;
			}
		} else if (messageTemp == "off") {
			Serial.println("Override off");

			//closeWindows();
			windowOverrideStatus = 0;
		}
	}
	//Let's Override the status of the fan
	if (String(topic) == mqttFan1Override) {
		Serial.print("Changing output to ");
		if (messageTemp == "on") {
			Serial.println("Fan Override on");
			if (fan1Status == 0) {
				changeFanState(1, relayOutputPin5);
				changeFanState(1, relayOutputPin6);
				fan1OverrideStatus = 1;
			} else {
				changeFanState(0, relayOutputPin5);
				changeFanState(0, relayOutputPin6);
				fan1OverrideStatus = 1;
			}
		} else if (messageTemp == "off") {
			Serial.println("Fan Override off");
				changeFanState(0, relayOutputPin5);
				changeFanState(0, relayOutputPin6);			
			fan1OverrideStatus = 0;
		}
	}
	if (String(topic) == mqttPumpControl) {
		Serial.print("Changing output to ");
		if (messageTemp == "on") {
			Serial.println("Override Pump on");
			controlPump(1);
			pumpOverrideStatus = 1;
		} else if (messageTemp == "off") {
			Serial.println("Override Pump off");
			controlPump(0);
			pumpOverrideStatus = 0;
		}
	}
}
void controlPump(int control) {
	if (control == 1) {
		digitalWrite(relayOutputPin7, HIGH);
		pumpStatus = 1;
		//PublishLCD("Pump is on...", "", 1000);

	} else if (control == 0) {
		digitalWrite(relayOutputPin7, LOW);
		pumpStatus = 0;
		//PublishLCD("Pump is off...", "", 1000);
	}
}
void getClimateValues() {

	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	humid1 = dht1.readHumidity();
	temp1 = dht1.readTemperature();  // Read temperature as Celsius (the default)
	humid2 = dht2.readHumidity();
	temp2 = dht2.readTemperature();
	// Check if any reads failed and exit early (to try again).
	if (isnan(temp1) || isnan(humid1)) {

		Serial.println(F("Failed to read from DHT High sensor!"));
		PublishLCD("No DHT High data :(", "", 1000);
	}
	if (isnan(temp2) || isnan(humid2)) {

		Serial.println(F("Failed to read from DHT Low sensor!"));
		//PublishLCD("No DHT Low data :(", "", 1000);
	
	}

	intakeTemp = sensors.getTempCByIndex(0);  //get air temp for intake fan
	intakeTemp = intakeTemp-IntakeCalib;			//calibrate air temp for this sensor
	Serial.println(intakeTemp);
}
void changeWindowState(int state, int window) {
	switch (state) {
	case 1:
		digitalWrite(window, LOW);
		break;
	case 0:
		digitalWrite(window, HIGH);
		break;
	}
	switch (window) {
	case 1:
		window1Status = state;
		break;
	case 2:
		window2Status = state;
		break;
	}
}

void controlWindows() {

	long windowNow = millis();
	if (windowNow - windowPause > WindowFlutter) {

		if (temp1 > Open_WindowsTemp1 && windowOverrideStatus == 0) {
      if(window1Status==0)
			  PublishLCD("Gosh it's warm!", "Opening Win 1", 1000);
			changeWindowState(1, relayOutputPin1);
			windowPause = windowNow;

		} else if (temp1 <= Open_WindowsTemp1 && windowOverrideStatus == 0) {
      if(window1Status==1)
			  PublishLCD("Cooling down", "Closing Win 1", 1000);
			changeWindowState(0, relayOutputPin1);
			windowPause = windowNow;
		}
		if (temp1 > Open_WindowsTemp2 && windowOverrideStatus == 0) {
      if(window2Status==0)
			  PublishLCD("Gosh it's hot!", "Opening Win 2", 1000);
			changeWindowState(1, relayOutputPin2);
			windowPause = windowNow;
		} else if (temp1 <= Open_WindowsTemp2 && windowOverrideStatus == 0) {
      if(window1Status==1)
			  PublishLCD("Cooling down", "Closing Win 2", 1000);
			changeWindowState(0, relayOutputPin2);
			windowPause = windowNow;
		}
	}
}

void changeFanState(int state, int fan) {  // 1 on 0 off fan 1=in 2=ex
	switch (state) {
	case 1:
		digitalWrite(fan, LOW);
		break;
	case 0:
		digitalWrite(fan, HIGH);
	}
	switch (fan) {
	case 1:
		fan1Status = state;
		break;
	case 2:
		fan2Status = state;
		break;
	}
}
void controlVentilation() {  // 1 on 0 off fan 1=in 2=ex
	if (temp1 >= ExFan_Temp && fan2Status == 0 && fan2OverrideStatus == 0) {
		changeFanState(1, relayOutputPin6);
		PublishLCD("It's damn hot!", "Exhaust fan on", 1000);
		Serial.println("Threshold Vent Fan Temp 1 Reached - Fan is on");
	} else if (temp1 < ExFan_Temp && fan2Status == 1 && fan2OverrideStatus == 0) {
		PublishLCD("It's cooler", "Exhaust fan off", 1000);
		Serial.println("Fan Ventilation not required - Fan is off");
		changeFanState(0, relayOutputPin6);
	}
	if (temp1 >= InFan_Temp && fan1Status == 0 && fan1OverrideStatus == 0) {
		changeFanState(1, relayOutputPin5);
		PublishLCD("It's warm!", "Intake fan on", 1000);
		Serial.println("Threshold Vent Fan Temp 1 Reached - Fan is on");
	} else if (temp1 < InFan_Temp && fan1Status == 1 && fan1OverrideStatus == 0) {
		PublishLCD("It's cooler", "Intake fan off", 1000);
		Serial.println("Fan Ventilation not required - Fan is off");
		changeFanState(0, relayOutputPin5);
	}
}
void getSoilMoistureValues() {
	float value = analogRead(SoilMoisturePin);
	Serial.print("Soil moisture: ");
	//Serial.println(value);

	soilMoisturePercent = map(value, AirCalibrationValue, WaterCalibrationValue, 0, 100);

	Serial.println(value);
	if (soilMoisturePercent > 100) {
		soilMoisturePercent = 100;
		Serial.print("100");
	} else if (soilMoisturePercent < 0) {
		soilMoisturePercent = 0;
		Serial.print("0");
	} else {
		Serial.print(soilMoisturePercent);
	}
	Serial.println("%");
}
void publishLCDLoopStatusData(int cycle, int pump, int fan1, int fan2, int win1, int win2) {
	//newTemp = round(temp1*10)/10.0;
  if(pump==0)
    pumpS = statusOff;
  else
    pumpS = statusOn;
  if(fan1==0)
    fan1S = statusOff;
  else
    fan1S = statusOn;
  if(fan2==0)
    fan2S = statusOff;
  else
    fan2S = statusOn;
  if(win1==0)
    win1S = winC;
  else
    win1S = winO;
  if(win2==0)
    win2S = winC;
  else
    win2S = winO;

	switch(cycle)
	{
	  case 0://climate data
		lcd.clear();
		lcd.setCursor(0, 0);  //Set cursor to character 0 on line 0
		lcd.print("T1:");
		lcd.print(temp1, 1);
		lcd.print("c ");
		lcd.print("H1:");
		lcd.print(humid1, 1);
		lcd.print("%");

		lcd.setCursor(0, 1);
		lcd.print("T2:");
		lcd.print(temp2, 1);
		lcd.print("c ");
		lcd.print("H2:");
		lcd.print(humid2, 1);
		break;

	  case 1:// soil and intake air temp
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Soil M:");
		lcd.print(soilMoisturePercent, 1);
		lcd.print("% ");
    lcd.setCursor(0, 1);
		lcd.print("In Air Temp:");
		lcd.print(intakeTemp, 1);
		lcd.print("c");
		break;

    case 2://Fan statuses
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("FIn:");
		lcd.print(fan1S);
		lcd.print("FEx:");
		lcd.print(fan2S);
    lcd.setCursor(0, 1);
		lcd.print("Pump:");
    lcd.print(pumpS);
    lcd.print("Lm:");
    lcd.print(flowRate);
		break;

    case 3://window statuses
    lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Window 1:");
		lcd.print(win1S);
    lcd.setCursor(0, 1);
		lcd.print("Window 2:");
    lcd.print(win2S);  
    break;
	}
}
void controlIrrigation() {
  if(soilMoisturePercent != 0)
  {
	if (soilMoisturePercent <= soilDryLevel && soilMoisturePercent != 0 && pumpOverrideStatus == 0)
		controlPump(1);
	if (soilMoisturePercent >= soilWetLevel && pumpOverrideStatus == 0)
		controlPump(0);
  }
}