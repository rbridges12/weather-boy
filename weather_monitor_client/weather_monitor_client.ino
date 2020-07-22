#include <dht11.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define DHT11_PIN 14 // D5 on NodeMCU

dht11 weather_sensor;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(weather_sensor.read(DHT11_PIN);
  Serial.print("\nhumidity %: ");
  Serial.println((float)weather_sensor.humidity, 2);
  Serial.print("temperature (C): ");
  Serial.println((float)weather_sensor.temperature, 2);

  delay(2000);
}
