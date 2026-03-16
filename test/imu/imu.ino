#include <Wire.h>
#include <SparkFun_BMI270_Arduino_Library.h>
#include "esp_sleep.h"

#define BMI_INT_PIN 2

BMI270 imu;

uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68

// Persists across deep sleep
RTC_DATA_ATTR uint32_t totalSteps = 0;

unsigned long notFacingStart = 0;

void setup()
{
  Serial.begin(115200);
  delay(100);

  Wire.begin(4, 16);
  while(imu.beginI2C(i2cAddress) != BMI2_OK)
  {
      // Not connected, inform user
      Serial.println("Error: BMI270 not connected, check wiring and I2C address!");

      // Wait a bit to see if connection is established
      delay(1000);
  }
  Serial.println("BMI270 connected!");

  imu.enableFeature(BMI2_STEP_COUNTER);

}

void loop(){
  uint32_t count;
  imu.getStepCount(&count);
  Serial.printf("Count: %d");

  delay(1000);
}

