#include "pinout.h"

#define BACKGROUND RGB565_BLACK

extern bool screenOn = false;

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
