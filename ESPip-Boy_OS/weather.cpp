#include "weather.h"
#include "secrets.h"
#include <time.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define FORECAST_ENTRIES 5

// ================= RTC PERSISTENT STORAGE =================
RTC_DATA_ATTR WeatherData rtcWeatherDataArray[FORECAST_ENTRIES];
RTC_DATA_ATTR bool rtcHasData = false;
// Timestamp of the last successful weather fetch (stored in RTC memory)
RTC_DATA_ATTR time_t rtcWeatherLastFetch = 0;

// ================= INTERNAL =================
static TaskHandle_t weatherTaskHandle = NULL;

static void weatherTask(void * parameter);
static bool fetchWeatherForecast();

// ==========================================================

bool fetchingWeather = false;

bool fetching_weather() {
    return fetchingWeather;
}

void init_weather_service()
{
    if (weatherTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            weatherTask,
            "WeatherTask",
            8192,
            NULL,
            1,
            &weatherTaskHandle,
            1
        );
    }
}

WeatherData WeatherService_getWeather()
{
    if (!rtcHasData) {
        Serial.println("No forecast data available!");
        return WeatherData{};
    }

    // Get current Unix timestamp in UTC
    time_t now = time(nullptr); // internal time

    long minDiff = LONG_MAX;
    int closestIndex = 0;
    for (int i = 0; i < FORECAST_ENTRIES; i++){
        // Compare UTC now to UTC forecast time
        unsigned long current_index_diff = abs(rtcWeatherDataArray[i].forecastTime - now);
        if (current_index_diff < minDiff){
            minDiff = current_index_diff;
            closestIndex = i;
        }
    }

    return rtcWeatherDataArray[closestIndex];
}

bool WeatherService_hasValidData()
{
    return rtcHasData;
}

// ==========================================================

// TODO use wifi with mutex so we own WiFi interface for duration of fetchWeatherForecast()
static void weatherTask(void * parameter)
{
    // Fetch weather once on poweron
    if(esp_reset_reason() == ESP_RST_POWERON){
        Serial.println("Power-on detected. Fetching initial weather forecast...");
        fetchingWeather = true;
        // Connect to Wifi 
        int retries = 0;
        const int max_retries = 20;
        WiFi.begin(SSID_NAME, SSID_PASSWORD);
        while (WiFi.status() != WL_CONNECTED)
        {
            if (retries < max_retries){
            retries++;
            Serial.print('.');
            delay(500);
            } else {
            Serial.println("Failed to connect to WiFi. Cannot set system time.");
            break;
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            if (fetchWeatherForecast()) {
                rtcWeatherLastFetch = time(nullptr);
            }
        }

        // Turn off Wifi
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        fetchingWeather = false;
    }

    // Then during sleep/reset cycles, only fetch when the last update is stale
    for (;;) {
        time_t now = time(nullptr);

        bool needFetch = false;
        const time_t fifteenHours = 15 * 3600;

        if (!rtcHasData) {
            needFetch = true;
        } else if (now == 0) {
            // No valid system time yet; treat as stale so attempt to refresh
            needFetch = true;
        } else if ((now - rtcWeatherLastFetch) >= fifteenHours) {
            needFetch = true;
        }

        if (!needFetch) {
            Serial.println("Weather data fresh; skipping fetch.");
            vTaskDelay(WEATHER_UPDATE_INTERVAL_MS / portTICK_PERIOD_MS);
            continue;
        }

        Serial.println("Weather stale or missing; updating forecast...");
        fetchingWeather = true;
        // Connect to Wifi 
        int retries = 0;
        const int max_retries = 20;
        WiFi.begin(SSID_NAME, SSID_PASSWORD);
        while (WiFi.status() != WL_CONNECTED)
        {
            if (retries < max_retries){
            retries++;
            Serial.print('.');
            delay(500);
            } else {
            Serial.println("Failed to connect to WiFi. Cannot set system time.");
            break;
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            if (fetchWeatherForecast()) {
                rtcWeatherLastFetch = time(nullptr);
            }
        }

        // Turn off Wifi
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        fetchingWeather = false;
    }
}

// ==========================================================
// HTTPS WEATHER FETCH USING WiFiClientSecure
// ==========================================================

static bool fetchWeatherForecast()
{
    const char* host = "api.openweathermap.org";
    const int httpsPort = 443;

    WiFiClientSecure client;
    // TODO install OpenWeatherMap root_ca into PROG_MEM[]
    client.setInsecure(); // skip cert validation

    Serial.println("Connecting to OpenWeatherMap...");
    if (!client.connect(host, httpsPort)) {
        Serial.println("Connection failed!");
        return false;
    }

    String url = "/data/2.5/forecast?q=";
    url += WEATHER_CITY;
    url += ",";
    url += WEATHER_COUNTRY_CODE;
    url += "&appid=";
    url += WEATHER_API_KEY;
    url += "&units=metric";

    // Send HTTPS GET request
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");


    while (client.connected() && !client.available()) { delay(10); }
    /* TODO NON-block weather */
    /* set a global flag indicating weather is being fetched, 
    pass to sleep to not allow watch to sleep until finished*/
    // Wait for response
    /*
    TickType_t startTick = xTaskGetTickCount();
    while (client.connected() && !client.available()) { 
        vTaskDelay(10);

        // Timeout after 5 seconds
        if ((xTaskGetTickCount() - startTick) > pdMS_TO_TICKS(5000)) {
            Serial.println("Timeout waiting for weather response");
            client.stop();
            return false;
        }
    }
    */

    // Skip HTTP headers
    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;
    }

    // Read JSON payload
    String payload;
    while (client.available()) { payload += client.readString(); }

    // TODO use wifi with mutex so we own WiFi interface
    // for duration of 
    client.stop();

    // --- Parse JSON ---
    StaticJsonDocument<1024> filter;
    for (int i = 0; i < FORECAST_ENTRIES; i++) {
        filter["list"][i]["dt"] = true;
        filter["list"][i]["main"]["temp"] = true;
        filter["list"][i]["main"]["humidity"] = true;
        filter["list"][i]["rain"]["3h"] = true;  // precipitation in last 3h (if any)
        filter["list"][i]["snow"]["3h"] = true;  // snow in last 3h (if any)
    }

    DynamicJsonDocument doc(FORECAST_ENTRIES * 512);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }

    JsonArray list = doc["list"];

    // Parse the next 5 entries (15 hours)
    for (int i = 0; i < FORECAST_ENTRIES && i < list.size(); i++) {
        // Save values to RTC memory
        rtcWeatherDataArray[i].temperature = list[i]["main"]["temp"].as<float>();
        rtcWeatherDataArray[i].humidity = list[i]["main"]["humidity"].as<float>();
        float rain = list[i]["rain"]["3h"] | 0.0;
        float snow = list[i]["snow"]["3h"] | 0.0;
        rtcWeatherDataArray[i].precipitation = rain + snow;
        rtcWeatherDataArray[i].forecastTime = list[i]["dt"].as<long>();

        // Print values
        Serial.print("Entry "); Serial.print(i);
        Serial.print(": Temp="); Serial.print(rtcWeatherDataArray[i].temperature);
        Serial.print("C, Humidity="); Serial.print(rtcWeatherDataArray[i].humidity);
        Serial.print("%, Precip="); Serial.print(rtcWeatherDataArray[i].precipitation);
        Serial.print("mm, Timestamp="); Serial.println(rtcWeatherDataArray[i].forecastTime);
    }

    rtcHasData = true;
    Serial.println("Weather forecast updated successfully.");
    return true;
}

