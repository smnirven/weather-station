#include <Arduino.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <Wire.h>
#include <WiFi.h>
#include "secrets.h"
#include "SparkFunBME280.h"

BME280 bme280Sensor;
WiFiClient net;
MQTTClient client;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  Serial.println();
  Serial.println("Running Firmware.");

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if connecting failed.
    // This is due to incorrect credentials
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
      Serial.println();
    }
    delay(5000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Hello World, I'm connected to the internets!!");

  Serial.println("Connecting to mqtt broker...");
  client.begin(MQTT_BROKER, MQTT_PORT, net);
  while (!client.connect("arduino", "admin", "letmein")) {
    Serial.print(".");
    delay(5000);
  }
  
  // Setup BME280 atmospheric sensor
  Serial.println("Example showing alternate I2C addresses");
  Wire.begin();
  Wire.setClock(400000); //Increase to fast I2C speed!
  bme280Sensor.beginI2C();
  bme280Sensor.setReferencePressure(101200); //Adjust the sea level pressure used for altitude calculations

  Serial.println("Weather station online!");
}

void loop() {
  unsigned long loopStart = millis();
  DynamicJsonDocument doc(1024);
  float tempC = bme280Sensor.readTempC();
  float humidity = bme280Sensor.readFloatHumidity();
  float pressure = bme280Sensor.readFloatPressure();
  // Serial.print(mySensor.readTempF(), 2);
  doc[String("internal_temp_c")] = tempC;
  doc[String("relative_humidity")] = humidity;
  doc[String("pressure")] = pressure;
  doc[String("timestamp_ms")] = millis();
  String payload;
  serializeJson(doc, payload);
  client.publish("readings", payload);
  Serial.println(payload);
  unsigned long executionMillis = millis() - loopStart;
  // Serial.println(executionMillis);
  delay(5000);
}