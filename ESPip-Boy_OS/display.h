#include "pinout.h"

#define BACKGROUND RGB565_BLACK
#define MARK_COLOR RGB565_WHITE
#define SUBMARK_COLOR RGB565_DARKGREY // RGB565_LIGHTGREY
#define HOUR_COLOR RGB565_WHITE
#define MINUTE_COLOR RGB565_BLUE // RGB565_LIGHTGREY
#define SECOND_COLOR RGB565_RED

#define SIXTIETH 0.016666667
#define TWELFTH 0.08333333
#define SIXTIETH_RADIAN 0.10471976
#define TWELFTH_RADIAN 0.52359878
#define RIGHT_ANGLE_RADIAN 1.5707963


static uint8_t conv2d(const char *p)
{
  uint8_t v = 0;
  return (10 * (*p - '0')) + (*++p - '0');
}

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
