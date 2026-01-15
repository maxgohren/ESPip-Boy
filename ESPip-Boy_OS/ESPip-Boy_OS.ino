#include "CST816S.h"
#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Arduino_GFX_Library.h>

#include "pinout.h"

#include "clock.h"
#include "imu.h"
#include "display.h"
#include "sleep.h"
#include "fuel.h"
#include "loading_screen.h"
#include "rtc.h"
#include "termiwatch.h"
#include "flash.h"

void setup(void)
{
  Serial.begin(115200);
  Wire.begin(IIC_SDA, IIC_SCL);

  // Set timezone
  //setenv("TZ", "EST-5", 1);
  //tzset();

  time_t now; 
  struct tm t;
  time(&now);
  localtime_r(&now, &t);

  // If first boot, set time from WiFi TODO do this every 24 hours?. Else get time from RTC
  if(get_boot_count() == 0){
    wifi_set_system_time(&Serial);
    rtc.set({ (uint8_t)t.tm_sec, (uint8_t)t.tm_min, (uint8_t)t.tm_hour, (uint8_t)t.tm_wday, (uint8_t)t.tm_mday, (uint8_t)t.tm_mon, (uint8_t)t.tm_year});
  } else {
    /*
    rtc_t rtc_time; 
    rtc_time = rtc.get();

    struct tm tm_time;
    tm_time.tm_year = rtc_time.year - 1900; 
    tm_time.tm_mon  = rtc_time.month - 1;
    tm_time.tm_mday = rtc_time.day;
    tm_time.tm_hour = rtc_time.hour;
    tm_time.tm_min  = rtc_time.min;
    tm_time.tm_sec  = rtc_time.sec;

    tm_time.tm_isdst = -1; // let system determine DST

    time_t epoch = mktime(&tm_time);

    struct timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;

    // Above code and RTC code is not correct, do not do this yet
    // I'll live with the internal RTC drift for a few more days ahah
    //settimeofday(&tv, NULL);
    */
  }

  // Init Display and clear
  init_termiwatch();

  // Backlight control
  //ledcAttach(LCD_BL, pwmFreq, pwmResolution);
  display_bl_setup();
  display_screen_on();

  Serial.println("Starting to init I2C.");
  imu_init();

  // Setup fuel gauge
  fuelGauge.begin(4, 16);
  //fuelGauge.configureBattery(); Only really needs to be done once

  // Sleep
  init_sleep_mode(&Serial);

  // Flashlight
  init_flashlight();
  Serial.println("Flashlight init complete.");

  Serial.println("ESPip-Boy init complete.");
}


void loop()
{
  // imu processing
  imu_handle_interrupt();
  handle_watch_orientation();

  // flashlight
  handle_flashlight();
  
  // time processing
  static unsigned long lastTimeUpdate = 0;
  if (millis() - lastTimeUpdate >= 1000) {
    lastTimeUpdate = millis();

    // Update UI once a second
    draw_termiwatch();
  }

}
