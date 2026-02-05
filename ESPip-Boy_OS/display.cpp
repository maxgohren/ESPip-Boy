#include <Arduino.h>
#include "display.h"
#include "pinout.h"

#define BACKLIGHT_PWM 1

bool screenOn = false;

bool screen_is_on()
{
  return screenOn;
}

void display_screen_off()
{
  // Stops repeated calls from hammering function (ideally we shouldn't call
  // it more than once but that's a TODO
  if (!screenOn) return;

#if BACKLIGHT_PWM
  for (int duty = 64; duty >= 0; duty--) {
    ledcWrite(BL_PIN, duty);
    delay(5);
  }
#else
  digitalWrite(BL_PIN, LOW);
#endif 

  screenOn = false;
}

void display_screen_on()
{
  // Stops repeated calls from hammering function (ideally we shouldn't call
  // it more than once but that's a TODO
  if (screenOn) return;

#if BACKLIGHT_PWM
  // TODO make this asynchronous
  for (int duty = 0; duty <= 64; duty++) {
    ledcWrite(BL_PIN, duty);
    delay(5);
  }
#else
  digitalWrite(BL_PIN, HIGH);
#endif 

  screenOn = true;
}

void init_display()
{
#if BACKLIGHT_PWM
  ledcAttach(BL_PIN, 5000, 8);
#else
  pinMode(BL_PIN, OUTPUT);
#endif 

  display_screen_on();
  //Serial.printf("Display init with %s control.\n", BACKLIGHT_PWM ? "PWM" : "GPIO");
}
