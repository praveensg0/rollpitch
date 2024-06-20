#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

// Wokwi's virtual WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";  // No password for Wokwi-GUEST

//ThingSpeak API endpoint API Key
const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "1KZZZ1XV3F6EQNHO";

#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

void i2cWrite(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void i2cRead(uint8_t reg, uint8_t *data, size_t length) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, length);
  for (size_t i = 0; i < length; i++) {
    data[i] = Wire.read();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  i2cWrite(MPU6050_PWR_MGMT_1, 0x00); 
}

void loop() {
  uint8_t accelData[6];
  i2cRead(MPU6050_ACCEL_XOUT_H, accelData, 6);

  int16_t accelX = (accelData[0] << 8) | accelData[1];
  int16_t accelY = (accelData[2] << 8) | accelData[3];
  int16_t accelZ = (accelData[4] << 8) | accelData[5];

  // Calculate Roll and Pitch
  float roll = atan2(accelY, accelZ) * 180.0 / PI;
  float pitch = atan(-accelX / sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;

  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print("\tPitch: ");
  Serial.println(pitch);

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(roll) + "&field2=" + String(pitch);
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  // Wait 10 seconds
  delay(10000);
}
