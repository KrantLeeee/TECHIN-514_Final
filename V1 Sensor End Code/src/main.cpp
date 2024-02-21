#include <Arduino.h>

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "3310-5G";
const char* password = "12345678";

// Server settings
const char* serverUrl = "http://display-device.local/data";
const int httpPort = 80;

Adafruit_BME280 bme; // I2C

void setup() {
  Serial.begin(115200);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  // Read data from the BME280 sensor
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  // Create a JSON object
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["pressure"] = pressure;
  String output;
  serializeJson(doc, output);

  // Send data to the Display Device
  WiFiClient client;
  if (client.connect(serverUrl, httpPort)) {
    client.println("POST /data HTTP/1.1");
    client.println("Host: display-device.local");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(output.length());
    client.println();
    client.print(output);
  }

  delay(10000); // Send data every 10 seconds
}
