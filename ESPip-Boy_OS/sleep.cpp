#include "sleep.h"
#include "imu.h"
#include "display.h"

#define WAKE_PIN GPIO_NUM_33

void set_power_saving_mode(){
  imu_set_low_power();
}

void go_sleep()
{
  // Wakeup from touch screen goes low
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);
  
  // Display during sleep?
  rtc_gpio_pullup_dis(GPIO_NUM_14);
  rtc_gpio_pulldown_en(GPIO_NUM_14);

  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

  set_power_saving_mode();

  display_screen_off();

  Serial.println("Going to deep sleep.");
  esp_deep_sleep_start();
}


#if 0
void isolate_unused_gpio(void)
{
    gpio_num_t pins[] = {
        GPIO_NUM_0, GPIO_NUM_2, GPIO_NUM_4,
        GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14,
        GPIO_NUM_15, GPIO_NUM_25, GPIO_NUM_26,
        GPIO_NUM_27, GPIO_NUM_32, GPIO_NUM_34,
        GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_39
    };

    for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
        rtc_gpio_isolate(pins[i]);
    }
}

/*
  // Setup wakeup pin
  rtc_gpio_init(WAKE_PIN);
  rtc_gpio_set_direction(WAKE_PIN, RTC_GPIO_MODE_INPUT_ONLY);

  rtc_gpio_pulldown_dis(WAKE_PIN);
  rtc_gpio_pullup_en(WAKE_PIN);

  /* TODO remove physical 100k pullup on BL gate that keeps it on */
  rtc_gpio_pulldown_en(GPIO_NUM_14);
  rtc_gpio_pullup_dis(GPIO_NUM_14);

  // Hold above config during deep sleep
  rtc_gpio_hold_en(WAKE_PIN);

  // Enable wakeup
  esp_sleep_enable_ext1_wakeup(1ULL << WAKE_PIN, ESP_EXT1_WAKEUP_ALL_LOW);

  // Do not use RTC memory during deep sleep
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

  // Need RTC peripherals for EXT1
  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_ON);

  //isolate_unused_gpio();

  power_down_periphs();

  Serial.println("Going to deep sleep.");
  delay(50); // Allow UART to flush
  esp_deep_sleep_start();
#endif

