#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define DHT_PIN 16 // D0 on NodeMCU

DHT weather_sensor(DHT_PIN, DHT11);

void setup() {
  Serial.begin(115200);
  weather_sensor.begin();
}

void loop() {
  Serial.println(weather_sensor.read(DHT_PIN));
  
  float humidity = weather_sensor.readHumidity();
  Serial.print("\nhumidity (%): ");
  Serial.println(humidity);

  float temp = weather_sensor.readTemperature();
  Serial.print("temperature (C): ");
  Serial.println(temp);

  delay(2000);
}
