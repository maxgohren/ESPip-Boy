#include "wifi_mgr.h"

WifiMgr::Result WifiMgr::connect(int maxRetries, unsigned long retryDelayMs)
{
    if (WiFi.status() == WL_CONNECTED) {
        return Result::AlreadyConnected;
    }

    WiFi.begin(SSID_NAME, SSID_PASSWORD);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        if (retries >= maxRetries) {
            DEBUG_PRINTLN("WiFiMgr: Failed to connect to WiFi.");
            return Result::Failed;
        }
        retries++;
        DEBUG_PRINT('.');
        delay(retryDelayMs);
    }

    DEBUG_PRINTLN("WiFiMgr: Connected to WiFi.");
    return Result::Connected;
}

void WifiMgr::disconnect()
{
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
    }
    WiFi.mode(WIFI_OFF);
}

bool WifiMgr::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}
