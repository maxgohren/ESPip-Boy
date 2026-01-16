#ifndef DISPLAY_H
#define DISPLAY_H
#include "pinout.h"

// TODO change to boolean set by pwm.h to confirm if ledcAttach was successful
//#define FADE 1

bool screenOn = false;

bool screen_is_on()
{
  return screenOn;
}

void display_screen_off()
{
#ifdef FADE
  ledcFade(BL_PIN, 255 /* start duty */, 0 /* end duty */, 500 /* ms */);
#else
  digitalWrite(BL_PIN, LOW);
#endif 
  screenOn = false;
}

void display_screen_on()
{
#ifdef FADE
  ledcFade(BL_PIN, 0 /* start duty */, 255 /* end duty */, 500 /* ms */);
#else
  digitalWrite(BL_PIN, HIGH);
#endif 
  screenOn = true;
}

void init_display()
{
#ifndef FADE
  pinMode(BL_PIN, OUTPUT);
#endif 
  display_screen_off();
}

#endif // DISPLAY_H
