#include <time.h>
#include <WiFi.h>
#include "../../ESPip-Boy_OS/rtc.h"
#include "../../ESPip-Boy_OS/wifi_config.h"

const long gmtOffset_sec = 5 * 60 * 60; // 5 hours idk why but configTime takes
                                        // positive offset

void wifi_set_system_time()
{
  // Connect to Wifi 
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
  }
  
  // Set domains to get time from
  configTime(gmtOffset_sec, 0, "time1.google.com", "pool.ntp.org");

  // Wait 5 seconds for NTP time sync connection
  time_t nowSecs = time(nullptr);
  while (nowSecs < 10)
  {
    delay(500);
    Serial.print(".");
    yield();
    // While waiting, we get current time since epoch in seconds
    nowSecs = time(nullptr);
  }

  // Set the time
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo); // Using time_t nowSecs (seconds since epoch), convert
                                 // to UTC calendar time (struct tm) timeinfo
  // This differs from localtime_r which converts to, you guessed it, local time (relies on tzset timezone)

  // Print 
  char buf[26];
  Serial.printf("WiFi:  localtime_r: %s\n", asctime_r(&timeinfo, buf));

  // Turn off Wifi
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void setup(){
  Serial.begin(115200);
  Wire.begin(4, 16);

  wifi_set_system_time();

  rtc.init();

  // get time in seconds since epoch
  time_t now;  //seconds
  time(&now);

  // convert time in seconds to calendar local time 
  struct tm tmLocal; // calendar struct
  localtime_r(&now, &tmLocal);

  // Print time with specific formatting
  char timeStr[100];
  strftime(timeStr, sizeof(timeStr), "%I:%M %p %a %e %b %Y", &tmLocal);

  Serial.printf("System: localtime_r: %s\n", timeStr);
  Serial.printf(
    "tm_sec=%d tm_min=%d tm_hour=%d tm_mday=%d tm_mon=%d tm_year=%d tm_wday=%d tm_yday=%d tm_isdst=%d\n",
    tmLocal.tm_sec, tmLocal.tm_min, tmLocal.tm_hour,
    tmLocal.tm_mday, tmLocal.tm_mon, tmLocal.tm_year,
    tmLocal.tm_wday, tmLocal.tm_yday, tmLocal.tm_isdst
  );

  // Set RTC with current time
  time(&now);
  localtime_r(&now, &tmLocal);
  rtc.setTime(tmLocal);
}

void loop(){
  struct tm tmLocal;
  rtc.getTime(&tmLocal);
  char timeStr[100];
  strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p %a %e %b %Y", &tmLocal);
  Serial.printf("System: localtime_r from RTC: %s\n", timeStr);
  delay(1000);
}
