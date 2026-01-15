#ifndef DISPLAY_H
#define DISPLAY_H
#include "pinout.h"

bool screenOn = false;

bool screen_is_on()
{
  return screenOn;
}

void display_bl_setup()
{
  pinMode(BL_PIN, OUTPUT);
}

void display_screen_off()
{
  digitalWrite(BL_PIN, LOW);
  screenOn = false;
}

void display_screen_on()
{
  digitalWrite(BL_PIN, HIGH);
  screenOn = true;
}

// TODO merge this with flash to reuse PWM code and fade in/out code for BL and FLSH?

#endif // DISPLAY_H
