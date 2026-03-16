#include <Arduino.h>
#include "CST816S.h"
#include "esp_sleep.h"

#define IIC_SDA    4
#define IIC_SCL    16

#define TP_RST     25
#define TP_INT     33

CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);

void printWakeReason()
{
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  Serial.print("Wakeup reason: ");
  switch (reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Touch interrupt (EXT0)");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Timer");
      break;
    default:
      Serial.println("Other");
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  printWakeReason();

  // --- Touch init ---
  pinMode(TP_INT, INPUT);   // INT pin (external pull-up recommended)
  pinMode(TP_RST, OUTPUT);

  touch.begin();
  delay(50);

  // IMPORTANT:
  // Clear any pending touch interrupt before sleeping
  // (reading touch data does this on CST816S)
  touch.available();

  Serial.println("Going to deep sleep. Touch screen to wake...");

  // Wake when TP_INT goes LOW
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0);

  delay(100);  // allow serial to flush
  esp_deep_sleep_start();
}

void loop()
{
  // never reached
}

