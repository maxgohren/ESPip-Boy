#pragma once

#include <Wire.h>

#define RTC_ADDR 0x32

#define SEC  0x00
#define MIN  0x01
#define HOUR 0x02
#define WDAY 0x03
#define MDAY 0x04
#define MON  0x05
#define YEAR 0x06
#define YDAY 0x07

uint8_t bcd_to_dec(uint8_t bcd){
  return (bcd & 0x0F) + (bcd >> 4) * 10;
}

uint8_t dec_to_bcd(uint8_t dec){
  return (dec / 10) << 4 | (dec % 10);
}

uint8_t weekday_to_bin(uint8_t weekday)
{
  return 1 << weekday;
}

uint8_t bin_to_weekday(uint8_t bin_weekday)
{
  if      (bin_weekday & 0x01) return 0;
  else if (bin_weekday & 0x02) return 1;
  else if (bin_weekday & 0x04) return 2;
  else if (bin_weekday & 0x08) return 3;
  else if (bin_weekday & 0x10) return 4;
  else if (bin_weekday & 0x20) return 5;
  else if (bin_weekday & 0x40) return 6;

  return 0;
}

class RTC {
public:
  void getTime(struct tm *rtc_time)
  {
    rtc_time->tm_sec = bcd_to_dec(readRTC(SEC));
    rtc_time->tm_min = bcd_to_dec(readRTC(MIN)); 
    rtc_time->tm_hour = bcd_to_dec(readRTC(HOUR)); 
    rtc_time->tm_mday = bcd_to_dec(readRTC(MDAY));  
    rtc_time->tm_mon = bcd_to_dec(readRTC(MON)); 
    rtc_time->tm_year = bcd_to_dec(readRTC(YEAR)); 
    rtc_time->tm_wday = bin_to_weekday(readRTC(WDAY)); 
    rtc_time->tm_yday = bcd_to_dec(readRTC(YDAY));
    rtc_time->tm_isdst = -1;
  }

  void setTime(struct tm rtc_time)
  {
    Serial.printf(
      "RTC W: tm_sec=%d tm_min=%d tm_hour=%d tm_mday=%d tm_mon=%d tm_year=%d tm_wday=%d tm_yday=%d tm_isdst=%d\n",
      rtc_time.tm_sec,
      rtc_time.tm_min,
      rtc_time.tm_hour,
      rtc_time.tm_mday,
      rtc_time.tm_mon,
      rtc_time.tm_year,
      rtc_time.tm_wday,
      rtc_time.tm_yday,
      rtc_time.tm_isdst
    );

    writeRTC(SEC,  dec_to_bcd(rtc_time.tm_sec));
    writeRTC(MIN,  dec_to_bcd(rtc_time.tm_min));
    writeRTC(HOUR, dec_to_bcd(rtc_time.tm_hour));
    writeRTC(WDAY, weekday_to_bin(rtc_time.tm_wday));
    writeRTC(MDAY, dec_to_bcd(rtc_time.tm_mday));
    writeRTC(MON,  dec_to_bcd(rtc_time.tm_mon));
    writeRTC(YEAR, dec_to_bcd(rtc_time.tm_year));
    writeRTC(YDAY, dec_to_bcd(rtc_time.tm_yday));

    Serial.printf(
        "RTC R: tm_sec=%d tm_min=%d tm_hour=%d tm_mday=%d tm_mon=%d tm_year=%d tm_wday=%d tm_yday=%d tm_isdst=%d\n",
      bcd_to_dec(readRTC(SEC)),
      bcd_to_dec(readRTC(MIN)), 
      bcd_to_dec(readRTC(HOUR)), 
      bcd_to_dec(readRTC(MDAY)),  
      bcd_to_dec(readRTC(MON)), 
      bcd_to_dec(readRTC(YEAR)), 
      bin_to_weekday(readRTC(WDAY)), 
      bcd_to_dec(readRTC(YDAY)),
      0);
      Serial.println();
  }

  float getTemp(){
    float temp = readRTC(0x17);
    return ((temp * 2 - 187.19) / 3.218);
  }

  int getVendorID() {
    return readRTC(0x20) >> 4;
  }

private:
  uint8_t readRTC(byte reg){
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(reg);
    Wire.endTransmission();

    Wire.requestFrom(RTC_ADDR, 1);
    while (Wire.available() == 0);
    return Wire.read();
  }

  void writeRTC(byte reg, byte value){
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
  }
};


// ----- RTC ------
RTC rtc;
