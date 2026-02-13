#pragma once
#include <Arduino.h>

class RTC {
public:
  RTC() = default;

  void getTime(struct tm *rtc_time);
  void setTime(struct tm rtc_time);

  float getTemp();
  int getVendorID();

private:
  uint8_t readRTC(byte reg);
  void writeRTC(byte reg, byte value);
};

extern RTC rtc;