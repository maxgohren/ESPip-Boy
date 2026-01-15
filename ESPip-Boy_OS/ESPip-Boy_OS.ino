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

  init_clock();

  // Init Display ui
  init_termiwatch();

  // Backlight control
  //ledcAttach(LCD_BL, pwmFreq, pwmResolution);
  display_bl_setup();
  display_screen_off();

  imu_init();

  // Setup fuel gauge
  fuelGauge.begin(4, 16);
  //fuelGauge.configureBattery(); Only really needs to be done once

  // Sleep
  init_sleep_mode(&Serial);

  // Flashlight
  init_flashlight();

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
