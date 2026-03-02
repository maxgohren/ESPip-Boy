#include "log.h"
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pinout.h"
#include "clock.h"
#include "imu.h"
#include "display.h"
#include "sleep.h"
#include "fuel.h"
#include "rtc.h"
#include "termiwatch.h"
#include "flash.h"
#include "touch.h"
#include "weather.h"
#include "task_monitor.h"

void setup(void)
{
  DEBUG_BEGIN(115200);
  Wire.begin(IIC_SDA, IIC_SCL);

  init_clock();

  init_touch();

  init_weather_service();

  // Init Display ui
  init_termiwatch();

  // Display backlight control - turn on at bootup after ui in init
  init_display();

  imu_init();

  // Setup fuel gauge
  fuelGauge.begin(4, 16);
  //fuelGauge.configureBattery(); Only really needs to be done once

  init_flashlight();
  
  init_task_monitor();

  DEBUG_PRINTLN("ESPip-Boy init complete.");
}


void loop()
{
  handle_watch_orientation();
  handle_flashlight();
  handle_touch();
  
  vTaskDelay(pdMS_TO_TICKS(10)); // allow scheduler some breathing room
}
