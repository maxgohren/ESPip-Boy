#include <WiFi.h>
#include "wifi_config.h"

void setClock(Stream *Serial)
{
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial->print('.');
    delay(500);
  }

  configTime(0, 0, "time1.google.com", "pool.ntp.org");


  Serial->print("Waiting for NTP time sync: ");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2)
  {
    delay(500);
    Serial->print(".");
    yield();
    nowSecs = time(nullptr);
  }

  Serial->println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial->print("Current time: ");
  char buf[26];
  Serial->println(asctime_r(&timeinfo, buf));

  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}
