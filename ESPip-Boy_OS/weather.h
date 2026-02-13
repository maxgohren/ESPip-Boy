#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <Arduino.h>

// ================= USER CONFIG =================
#define WEATHER_CITY           "Markham"
#define WEATHER_COUNTRY_CODE   "CA"

// Default: 24 hours
#define WEATHER_UPDATE_INTERVAL_MS (15UL * 60UL * 60UL * 1000UL)
// ===============================================

struct WeatherData {
    float temperature;
    float humidity;
    float precipitation;
    long forecastTime;  // Unix timestamp
};

// Public API
void init_weather_service();
WeatherData WeatherService_getWeather();
bool WeatherService_hasValidData();
bool fetching_weather();

#endif

