#include <WiFi.h>
#include "secrets.h"
#include "weather.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Connecting to WiFi...");
    WiFi.begin(SSID_NAME, SSID_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected!");

    WeatherService_begin();
}

void loop()
{
    if (WeatherService_hasValidData()) {

        WeatherData data = WeatherService_getWeather();

        Serial.println("------ WEATHER DATA ------");
        Serial.print("Temperature: ");
        Serial.println(data.temperature);

        Serial.print("Humidity: ");
        Serial.println(data.humidity);

        Serial.print("Precipitation: ");
        Serial.println(data.precipitation);

        Serial.println("--------------------------");
    }
    else {
        Serial.println("No weather data yet...");
    }

    delay(10000);
}

