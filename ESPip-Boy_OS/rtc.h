#include "Wire.h"

#define RTC_ADDR 0x32

int readRTC(byte reg){
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(RTC_ADDR, 1);
  while (Wire.available() == 0);
  return Wire.read();
}


int writeRTC(byte reg){
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(RTC_ADDR, 1);
  while (Wire.available() == 0);
  return Wire.read();
}

void setup(){
  Serial.begin(115200);
  Serial.println("Hello from ESP32!");
  Wire.begin(4, 16);

  // Init RTC
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  Serial.println("RTC Initialized!");
}

void loop(){
  uint8_t second = readRTC(0x10);
  uint8_t seconds_one = second & 0x0F; // first four bits are binary coded decimal 0-9
  uint8_t seconds_ten = second >> 4; // next 3 bits are binary coded decimal 0-5
                                     //
  int hour = readRTC(0x02);
  uint8_t hours_one = hour & 0x0F; // first four bits are binary coded decimal 0-9
  uint8_t hours_ten = hour >> 4; // next 3 bits are binary coded decimal 0-5

  int minute = readRTC(0x01);
  uint8_t minutes_one = minute & 0x0F; // first four bits are binary coded decimal 0-9
  uint8_t minutes_ten = minute >> 4; // next 3 bits are binary coded decimal 0-5
                                       //
  float temp = readRTC(0x17);
  Serial.printf("Temp Raw: %d\n", temp);
  Serial.printf("Temp Calc: %02f\n", (temp * 2 - 187.19) / 3.218);
  //Serial.printf("Temp Calc: %d\n", (temp * 2 - 187) / 3);
  //

  Serial.printf("Vendor ID: %x\n", readRTC(0x20) >> 4);


  Serial.print(hours_ten);
  Serial.print(hours_one);
  Serial.print(":");
  Serial.print(minutes_ten);
  Serial.print(minutes_one);
  Serial.print(":");
  Serial.print(seconds_ten);
  Serial.print(seconds_one);
  Serial.println();

  delay(1000); // Wait for the next second
}
