#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// pins
#define RED_PIN 4
#define GREEN_PIN 5
#define BLUE_PIN 16
#define DHT11_PIN 2

// LED values
uint8_t red = 128;
uint8_t blue = 128;
uint8_t green = 128;
uint8_t brightness = 75;
bool led_on = false;

const int port = 8080;
const int json_capacity = 1000; // Json document capacity
const char *ssid = "";          // Wi-Fi SSID
const char *password = "";      // Wi-Fi Password
const char *server_domain_name = "http://iot-testing.herokuapp.com";

ESP8266WebServer server(port);
DHT weather_sensor = DHT(DHT11_PIN, DHT11);

void setup()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    Serial.begin(115200);
    weather_sensor.begin();
    WiFi.begin(ssid, password);

    // wait for connection
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(400);
        Serial.print(".");
    }

    // print local IP
    Serial.println();
    Serial.print("Connected, local IP address: ");
    Serial.println(WiFi.localIP());

    send_ip();

    //assign handlers
    server.on("/", handle_index);
    // server.on("/get_temp", handle_get_temp);
    // server.on("/get_humidity", handle_get_humidity);
    server.on("/get_weather_data", handle_get_weather_data);
    server.on("/update_brightness", handle_update_brightness);
    server.on("/toggle_led", handle_toggle_led);
    server.on("/update_rgb", handle_update_rgb);
    server.on("/sync_data", handle_sync_data);

    // start server
    server.begin();
    Serial.println("server started");
}

void loop()
{
    server.handleClient();
}

void handle_index()
{
    server.send(200, "text/plain", "LED Web Server");
}

// handler for GET request to send all LED settings to control server
void handle_sync_data()
{
    // add all LED settings to JSON
    StaticJsonDocument<json_capacity> message;
    message["toggle"] = (int)led_on;
    message["red"] = red;
    message["green"] = green;
    message["blue"] = blue;
    message["brightness"] = brightness;

    // convert JSON document to a string
    String serializedOutput = "";
    serializeJson(message, serializedOutput);
    Serial.println(serializedOutput);
    server.send(200, "text/plain", serializedOutput);
}

// handler for POST request to change the RGB color of the LED
void handle_update_rgb()
{
    // check for POST request body
    if (!server.hasArg("plain"))
    {
        Serial.println("Error: POST body not found");
        server.send(400, "text/plain", "body not found");
        return;
    }

    // deserialize JSON from POST body
    StaticJsonDocument<json_capacity> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));

    // ensure deserialization was successful
    if (err)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
        server.send(400, "text/plain", "An error occurred while updating color");
        return;
    }

    // verify the JSON contains the RGB values
    JsonObject obj = doc.as<JsonObject>();
    if (!obj.containsKey("r") ||
        !obj.containsKey("g") ||
        !obj.containsKey("b"))
    {
        Serial.println("Error: missing RGB value(s)");
        server.send(400, "text/plain", "An error occurred while updating color");
        return;
    }

    // get RGB values and update LED
    red = doc["r"];
    green = doc["g"];
    blue = doc["b"];
    update_led();
    server.send(200, "Color has been updated succesfully");
}

// handler for POST request to change the LED brightness value
void handle_update_brightness()
{
    // check for POST request body
    if (!server.hasArg("plain"))
    {
        Serial.println("Error: POST body not found");
        server.send(400, "text/plain", "body not found");
        return;
    }

    // deserialize JSON from POST body
    StaticJsonDocument<json_capacity> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));

    // ensure deserialization was successful
    if (err)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
        server.send(400, "text/plain", "An error occurred while updating brightness");
        return;
    }

    // verify the JSON contains the brightness value
    JsonObject obj = doc.as<JsonObject>();
    if (!obj.containsKey("brightness"))
    {
        Serial.println("Error: brightness data missing");
        server.send(400, "An error occurred while updating brightness");
        return;
    }

    // get the brightness value and update LED
    brightness = doc["brightness"];
    update_led();
    server.send(200, "Brightness has been updated");
}

// handler for POST request to toggle the LED on or off
void handle_toggle_led()
{
    // check for POST request body
    if (!server.hasArg("plain"))
    {
        Serial.println("Error: POST body not found");
        server.send(400, "text/plain", "body not found");
        return;
    }

    // deserialize JSON from POST body
    StaticJsonDocument<json_capacity> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));

    // ensure deserialization was successful
    if (err)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
        server.send(400, "text/plain", "An error occurred while turning on LED");
        return;
    }

    // verify the JSON contains the toggle value
    JsonObject obj = doc.as<JsonObject>();
    if (!obj.containsKey("toggle"))
    {
        Serial.println("error: toggle data missing");
        server.send(400, "An error occurred while turning on LED");
        return;
    }

    // get toggle value and update LED
    int toggle = doc["toggle"];
    led_on = toggle;
    update_led();
    server.send(200, "LED has been turned on");
}

// // handler for GET request to send temp sensor value, sends as JSON
// void handle_get_temp()
// {
//     // read and print  sensor values
//     // TODO: move F/C conversion to the control server
//     float tempC = weather_sensor.readTemperature();
//     float tempF = weather_sensor.convertCtoF(tempC);
//     Serial.print("Temperature (C) = ");
//     Serial.println(tempC);
//     Serial.print("Temperature (F) = ");
//     Serial.println(tempF);

//     // put temp data in JSON
//     StaticJsonDocument<json_capacity> temperature_info;
//     temperature_info["tempC"] = tempC;
//     temperature_info["tempF"] = tempF;
//     String serialized_temp = "";
//     serializeJson(temperature_info, serialized_temp);

//     server.send(200, "text/plain", serialized_temp);
// }

// // handler for GET request to send humidity sensor value, sends as JSON
// void handle_get_humidity()
// {
//     // read and print sensor value
//     float humidity = weather_sensor.readHumidity();
//     Serial.print("Humidity = ");
//     Serial.println(humidity);

//     // put humidity data in JSON
//     StaticJsonDocument<json_capacity> humidity_info;
//     humidity_info["humidity"] = humidity;
//     String serialized_humidity = "";
//     serializeJson(humidity_info, serialized_humidity);

//     server.send(200, "text/plain", serialized_humidity);
// }

// handler for GET request to send temp and humidity data in a JSON string
void handle_get_weather_data()
{
    // read sensor data
    float temp = weather_sensor.readTemperature();
    float humidity = weather_sensor.readHumidity();

    // put it in JSON
    StaticJsonDocument<json_capacity> weather_data;
    weather_data["temp"] = temp;
    weather_data["humidity"] = humidity;
    String weather_data_json = "";
    serializeJson(weather_data, weather_data_json);

    server.send(200, "text/plain", weather_data_json);
}

// update the LED color, brightness, and toggle
void update_led()
{
    if (led_on)
    {
        float brightness_factor = brightness / 255.0;
        analogWrite(RED_PIN, red * brightness_factor);
        analogWrite(GREEN_PIN, green * brightness_factor);
        analogWrite(BLUE_PIN, blue * brightness_factor);
    }
    else
    {
        digitalWrite(RED_PIN, LOW);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, LOW);
    }
}

void send_ip()
{
    WiFiClient api_client;

    // connect to ipify API
    if (!api_client.connect("api.ipify.org", 80))
    {
        Serial.println("failed to connect to ipify");
        return;
    }

    // request plain text IP from API, timeout after 5 seconds
    api_client.print("GET / HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    unsigned long timeout = millis() + 5000;
    while (!api_client.available())
    {
        if (millis() > timeout)
        {
            Serial.println("ipify timed out");
            api_client.stop();
            return;
        }
    }

    // read the API response and parse the IP address string
    String raw_msg = api_client.readString();
    api_client.stop();
    int i = raw_msg.length() - 1;
    while (raw_msg.charAt(i) != '\n')
    {
        i--;
    }
    String ip_string = raw_msg.substring(i + 1);
    Serial.println(ip_string);

    // connect client to the control server with public IP in the address
    HTTPClient client;
    String url = server_domain_name + (String) "/api/led_control/client_ip/" + ip_string;
    client.begin(url);

    // send GET request and test connection
    int http_code = client.GET();
    if (!client.connected())
    {
        Serial.print("could not connect to ");
        Serial.println(url);
    }
    client.end();
    Serial.print("HTTP code from IP send request: ");
    Serial.println(http_code);
}

// send weather data to control server
void store_data()
{
    // read sensor values
    float tempC = weather_sensor.readTemperature();
    float tempF = weather_sensor.convertCtoF(tempC);
    float humidity = weather_sensor.readHumidity();
    Serial.println(tempC);
    Serial.println(tempF);
    Serial.println(humidity);
    Serial.println();

    // send data to server
    HTTPClient client;
    String url = server_domain_name + (String) "/api/temp_control/store_data/" + (String)tempF + "/" + (String)tempC + "/" + (String)humidity;
    client.begin(url);
    int http_code = client.GET();
    if (!client.connected())
    {
        Serial.print("could not connect to ");
        Serial.println(url);
    }
    client.end();
    Serial.print("HTTP code from weather data request: ");
    Serial.println(http_code);
}
