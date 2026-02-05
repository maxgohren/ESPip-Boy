#include <Wire.h>

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
#include "touch.h"

void setup(void)
{
  Serial.begin(115200);
  Wire.begin(IIC_SDA, IIC_SCL);

  init_clock();

  init_touch();

  // Init Display ui
  init_termiwatch();

  // Display backlight control - turn on at bootup after ui in init
  init_display();

  imu_init();

  // Setup fuel gauge
  fuelGauge.begin(4, 16);
  //fuelGauge.configureBattery(); Only really needs to be done once

  // Flashlight
  init_flashlight();

  Serial.println("ESPip-Boy init complete.");
}


void loop()
{
  // imu processing
  handle_watch_orientation();

  // flashlight
  handle_flashlight();

  // handle touch
  handle_touch();
  
  // time processing
  static unsigned long lastTimeUpdate = 0;
  if (millis() - lastTimeUpdate >= 1000) {
    lastTimeUpdate = millis();

    // Update UI once a second
    draw_termiwatch();
  }

}
