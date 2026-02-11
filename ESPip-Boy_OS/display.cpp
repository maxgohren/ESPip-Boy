#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include "pinout.h"

#define BACKLIGHT_PWM 1

#define FADE_UP 1
#define FADE_DOWN 2

TaskHandle_t displayBacklightFadeTaskHandle = NULL;

static bool screenOn = false;
static uint8_t currentDuty = 0;

bool screen_is_on()
{
  return screenOn;
}

void DisplayBacklightFadeTask(void *parameter)
{
  uint32_t command;

  for (;;)
  {
    // Wait forever for a backlight command
    if (xTaskNotifyWait(
          0, // don't clear bits on entry
          ULONG_MAX, // clear all bits on exit
          &command,
          portMAX_DELAY) == pdTRUE)
    {
      int targetDuty = (command == FADE_UP) ? 64 : 0;
      int step = (targetDuty > currentDuty) ? 1 : -1;

      while (currentDuty != targetDuty)
      {
        currentDuty += step;
        ledcWrite(BL_PIN, currentDuty);

        // Check if a new command arrived (interrupt current fade)
        if (xTaskNotifyWait(0, ULONG_MAX, &command, 0) == pdTRUE){
          targetDuty = (command == FADE_UP) ? 64 : 0;
          step = (targetDuty > currentDuty) ? 1 : -1;
        }

        vTaskDelay(pdMS_TO_TICKS(5));
      }

      screenOn = (currentDuty > 0);
    }
  }
}

void display_screen_off()
{
  if (displayBacklightFadeTaskHandle != NULL){
    xTaskNotify(displayBacklightFadeTaskHandle, FADE_DOWN, eSetValueWithOverwrite);
  }
}

void display_screen_on()
{
  if (displayBacklightFadeTaskHandle != NULL){
    xTaskNotify(displayBacklightFadeTaskHandle, FADE_UP, eSetValueWithOverwrite);
  }
}

void init_display()
{
#if BACKLIGHT_PWM
  ledcAttach(BL_PIN, 5000, 8);
#else
  pinMode(BL_PIN, OUTPUT);
#endif 

  // Kicks off 
  xTaskCreatePinnedToCore(
      DisplayBacklightFadeTask,
      "DisplayBacklightFade",
      4096,
      NULL,
      1, // lower priority than UI update
      &displayBacklightFadeTaskHandle,
      1); // same core

  display_screen_on(); // Start with fade up

  //Serial.printf("Display init with %s control.\n", BACKLIGHT_PWM ? "PWM" : "GPIO");
}
