#pragma once

#define RTC_ADDR 0x32

typedef struct rtc_t {
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  uint8_t week;
  uint8_t day;
  uint8_t month;
  uint8_t year;
};

uint8_t bcd_to_dec(uint8_t bcd){
  return (bcd & 0x0F) + (bcd >> 4) * 10;
}

uint8_t dec_to_bcd(uint8_t dec){
  return (dec / 10) << 4 | (dec & 0x0F);
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

  void init(){
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
    Serial.println("RTC Initialized!");
  }
  
  rtc_t get()
  {
    // first four bits are binary coded decimal 0-9
    // next 3 bits are binary coded decimal 0-5
    rtc_t rtc_time;

    uint8_t sec   = readRTC(0x00);
    uint8_t min   = readRTC(0x01);
    uint8_t hour  = readRTC(0x02);
    uint8_t week  = readRTC(0x03);
    uint8_t day   = readRTC(0x04);
    uint8_t month = readRTC(0x05);
    uint8_t year = readRTC(0x06);

    rtc_time = {
      bcd_to_dec(sec), 
      bcd_to_dec(min), 
      bcd_to_dec(hour),
      bin_to_weekday(week),
      bcd_to_dec(day),
      bcd_to_dec(month),
      bcd_to_dec(year)
    };

    return rtc_time;
  }

  void set(rtc_t rtc_time){
    writeRTC(0x00, dec_to_bcd(rtc_time.sec));
    writeRTC(0x01, dec_to_bcd(rtc_time.min));
    writeRTC(0x02, dec_to_bcd(rtc_time.hour));
    writeRTC(0x03, weekday_to_bin(rtc_time.week));
    writeRTC(0x05, dec_to_bcd(rtc_time.month));
    writeRTC(0x06, dec_to_bcd(rtc_time.year));
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
    //while (Wire.available() == 0);
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
