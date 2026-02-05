#include <WiFi.h>
#include "wifi_config.h"
#include "rtc.h"
#include "sleep.h"

const long gmtOffset_sec = -5 * 60 * 60; // -5 hours

void wifi_set_system_time()
{
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
  
  // Set domains to get time from
  configTime(gmtOffset_sec, 0, "time1.google.com", "pool.ntp.org");

  // Wait for NTP time sync asynchronous operation to actually be valid
  // If nowSecs < 10, it means we haven't jumped from epoch yet
  time_t nowSecs = time(nullptr);
  while (nowSecs < 10)
  {
    delay(500);
    Serial.print(".");
    nowSecs = time(nullptr); //update time to see if we jumped from epoch yet
  }

  // Convert nowSecs to calendar struct with date/month/year etc.
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo); 
                                 

  // Update RTC time with updated WiFi sync'd time
  rtc.setTime(timeinfo);

  // Print 
  char buf[26];
  Serial.printf("WiFi:  localtime_r: %s\n", asctime_r(&timeinfo, buf));

  // Turn off Wifi
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void init_clock()
{
  // Set timezone for localtime_r() call
  setenv("TZ", "EST5EDT", 1);
  tzset();

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
      Serial.println(ctime(&now));
  //}
  }
}
