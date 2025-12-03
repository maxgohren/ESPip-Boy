#include "driver/rtc_io.h"

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

void go_sleep()
{
  esp_deep_sleep_start();
}

