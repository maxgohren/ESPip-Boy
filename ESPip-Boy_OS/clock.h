#include "log.h"
#include <WiFi.h>
#include "secrets.h"
#include "wifi_mgr.h"
#include "rtc.h"
#include "sleep.h"

void wifi_set_system_time()
{
  // Connect to Wifi
  WifiMgr::Result result = WifiMgr::connect();
  if (result != WifiMgr::Result::Connected && result != WifiMgr::Result::AlreadyConnected) {
    DEBUG_PRINTLN("WifiMgr: Unable to connect to WiFi; skipping time sync.");
    return;
  }

  // Set domains to get time from
  const char* time_zone = "EST5EDT,M3.2.0,M11.1.0";
  configTzTime(time_zone, "pool.ntp.org");

  // Wait for NTP time sync asynchronous operation to actually be valid
  // If nowSecs < 10, it means we haven't jumped from epoch yet
  time_t nowSecs = time(nullptr);
  while (nowSecs < 10)
  {
    delay(500);
    DEBUG_PRINT(".");
    nowSecs = time(nullptr); //update time to see if we jumped from epoch yet
  }

  // Convert nowSecs to calendar struct with date/month/year etc.
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo); 

  // Update RTC time with updated WiFi sync'd time
  rtc.setTime(timeinfo);

  // Print 
  char buf[26];
  DEBUG_PRINTF("WiFi:  localtime_r: %s\n", asctime_r(&timeinfo, buf));

  // Turn off Wifi
  WifiMgr::disconnect();
}

void init_clock()
{
  // If first boot, set time from WiFi TODO do this every 24 hours?. Else get time from RTC
  if(esp_reset_reason() == ESP_RST_POWERON){
    wifi_set_system_time();
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
      DEBUG_PRINTLN(ctime(&now));
  //}
  }
}
