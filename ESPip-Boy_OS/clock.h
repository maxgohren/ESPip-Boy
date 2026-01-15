#include <WiFi.h>
#include "wifi_config.h"
#include "rtc.h"
#include "sleep.h"

void wifi_set_system_time(Stream *Serial)
{
  // TODO make this an RTOS thread that runs once a day, specifically during
  // night time hours when the watch is not being used
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

void init_clock()
{
  rtc.init();

  // If first boot, set time from WiFi TODO do this every 24 hours?. Else get time from RTC
  if(get_boot_count() == 0){
    wifi_set_system_time(&Serial);
  } else {
    // TODO rtc.updateSystemTime(){
      // Get time from rtc into calendar struct
      struct tm tmLocal = {};
      rtc.getTime(&tmLocal);

      // Convert calendar struct to seconds since epoch
      time_t seconds_since_epoch = mktime(&tmLocal);

      // Convert time_t to timeval
      struct timeval t;
      t.tv_sec = seconds_since_epoch;
      t.tv_usec = 0;
      settimeofday(&t, NULL);
      
      // Verify time is set correctly
      time_t now;
      time(&now);
      Serial.println(ctime(&now));
  //}
  }
}
