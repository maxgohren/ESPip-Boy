#pragma once
#include "driver/rtc_io.h"
#include "pinout.h"
#include "display.h"

// ----- Deep Sleep -----
#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO) // 2 ^ GPIO in hex
#define USE_EXT0_WAKEUP           1
#define WAKEUP_GPIO               GPIO_NUM_2

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

// TODO bool user_active = false;
// set user active when turning things on like flashlight, touchscreen buttons
// etc. this will delay
//
int get_boot_count(){
  return bootCount;
}

void init_sleep_mode(Stream *S)
{
  ++bootCount;
  S->println("Boot count: " + String(bootCount));

  print_wakeup_reason();

  esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO, 1); // Trigger wakeup on HIGH from pin 2 (IMU_INT)
  
  // Configure gpio to be pulled down
  rtc_gpio_pullup_dis(WAKEUP_GPIO);
  rtc_gpio_pulldown_en(WAKEUP_GPIO);
}

void set_power_saving_mode(){

  // TODO
  //set_imu_low_power();
  //set_mag_low_power();
  //set_lux_low_power();
  // USB UART power modes?
  // fuel gauge power modes?
  // battery charger power modes?

  // shutdown speaker

}

void go_sleep()
{
  // TODO if(!user_active){
  // TODO
  // esp32 peripherals: 
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
  //set_power_saving_mode();
  //
  display_screen_off();
  Serial.println("Going to deep sleep.");
  esp_deep_sleep_start();
//}
  
}

