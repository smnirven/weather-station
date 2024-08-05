#include <Arduino.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include <Wire.h>
#include <WiFi.h>
#include "secrets.h"
#include "SparkFunBME280.h"
#include "SparkFun_Weather_Meter_Kit_Arduino_Library.h"

// Pins for Weather Carrier with ESP32 Processor Board
int windDirectionPin = 35;
int windSpeedPin = 14;
int rainfallPin = 27;

// Create an instance of the weather meter kit
SFEWeatherMeterKit weatherMeterKit(windDirectionPin, windSpeedPin, rainfallPin);
// Here we create a struct to hold all the calibration parameters
SFEWeatherMeterKitCalibrationParams calibrationParams;

BME280 bme280Sensor;
WiFiClient net;
MQTTClient client;

void clearUserInput()
{
    // Ensure all previous characters have come through
    delay(100);

    // Throw away all previous characters
    while (Serial.available() != 0)
    {
        Serial.read();
    }
}

void waitForUserInput()
{
    // Remove previous user input
    clearUserInput();

    // Wait for user to input something
    while (Serial.available() == 0)
    {
        // Nothing to do, keep waiting
    }
}

void setupWifi() {
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
  Serial.println("DNS Server:");
  Serial.println(WiFi.dnsIP());

  Serial.println("Hello World, I'm connected to the internets!!");
}

void setupMqttBroker() {
  Serial.println("Connecting to mqtt broker...");
  client.begin(MQTT_BROKER, MQTT_PORT, net);
  while (!client.connect("arduino", "admin", "letmein")) {
    Serial.print(".");
    delay(5000);
  }
}

void runRainfallCalibrationHelper()
{
    Serial.println();
    Serial.println(F("Rainfall calibration!"));

    // Rainfall calibration
    Serial.println();
    Serial.println(F("The rainfall detector contains a small cup that collects rain"));
    Serial.println(F("water. When the cup fills, the water gets dumped out and a"));
    Serial.println(F("counter is incremented. The exact volume of this cup needs to"));
    Serial.println(F("be known to get an accurate measurement of the total rainfall."));
    Serial.println(F("To calibrate this value, you'll need to pour a known volume"));
    Serial.println(F("of water into the rainfall detector, and the cup volume will"));
    Serial.println(F("be calculated. The rate at which the water is poured can"));
    Serial.println(F("affect the measurement, so go very slowly to simulate actual"));
    Serial.println(F("rain rather than dumping it all at once!"));
    Serial.println(F("Enter any key once you're ready to begin"));

    // Wait for user to begin
    waitForUserInput();

    // User is ready, reset the rainfall counter
    weatherMeterKit.resetTotalRainfall();

    Serial.println();
    Serial.println(F("Begin pouring!"));
    Serial.println();

    // Wait for user to finish
    clearUserInput();
    while (Serial.available() == 0)
    {
        Serial.print(F("Enter any key once finished pouring."));
        Serial.print(F("    Number of counts: "));
        Serial.print(weatherMeterKit.getRainfallCounts());
        Serial.print(F("    Measured rainfall (mm): "));
        Serial.println(weatherMeterKit.getTotalRainfall(), 1);

        // Print slowly
        delay(1000);
    }

    Serial.println();
    Serial.println(F("Now enter the volume of water poured in mL"));
    waitForUserInput();
    int totalWaterML = Serial.parseInt();

    // Convert ml to mm^3
    int totalWaterMM3 = totalWaterML * 1000;

    // Divide by collection area of rainfall detector. It's about 50mm x 110mm,
    // resulting in a collection area of about 5500mm^2
    float totalRainfallMM = totalWaterMM3 / 5500.0;

    // Divide by number of counts
    float mmPerCount = totalRainfallMM / weatherMeterKit.getRainfallCounts();

    // Set this as the new mm per count
    calibrationParams.mmPerRainfallCount = mmPerCount;
    weatherMeterKit.setCalibrationParams(calibrationParams);

    // Print value for user to see
    Serial.println();
    Serial.print(F("Setting mm per count to "));
    Serial.println(mmPerCount, 4);

    Serial.println();
    Serial.println(F("Rainfall calibration complete!"));
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  Serial.println(F("SparkFun Weather Meter Kit Example 3 - Calibration Helper"));
  Serial.println();
  Serial.println(F("This example will help you determine the best calibration"));
  Serial.println(F("parameters to use for your project. Once each section is done,"));
  Serial.println(F("the values will be printed for you to copy into your sketch."));

  // We'll be changing the calibration parameters one at a time, so we'll get
  // all the default values now
  calibrationParams = weatherMeterKit.getCalibrationParams();

  // Begin weather meter kit
  weatherMeterKit.begin();

  calibrationParams.mmPerRainfallCount = 0.3636;
  weatherMeterKit.setCalibrationParams(calibrationParams);

  // runRainfallCalibrationHelper();

  setupWifi();

  setupMqttBroker();
  
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
  doc[String("total_rainfall_mm")] = weatherMeterKit.getTotalRainfall();
  doc[String("wind_speed_kph")] = weatherMeterKit.getWindSpeed();
  doc[String("wind_direction_deg")] = weatherMeterKit.getWindDirection();
  doc[String("timestamp_ms")] = millis();
  String payload;
  serializeJson(doc, payload);
  client.publish("readings", payload);
  Serial.println(payload);
  unsigned long executionMillis = millis() - loopStart;
  delay(5000 - executionMillis);
}