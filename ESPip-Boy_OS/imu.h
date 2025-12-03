#include "SparkFun_BMI270_Arduino_Library.h"
#include <Wire.h>

#define IMU_INT 2

void imuInterruptHandler()
{
    imuInterruptOccurred = true;
}

imu_init()
{
  Wire.begin(IIC_SDA, IIC_SCL);
  // Init IMU
    while(imu.beginI2C(i2cAddress) != BMI2_OK)
  {
      Serial.println("Error: BMI270 not connected, check wiring and I2C address!");
      delay(1000);
  }
  Serial.println("BMI270 connected!");
  
  // Wrist Wake Up -> poor performance
  //imu.enableFeature(BMI2_WRIST_WEAR_WAKE_UP);
  //imu.mapInterruptToPin(BMI2_WRIST_WEAR_WAKE_UP_INT, BMI2_INT1);

  // Any motion -> way easier to tune
  imu.enableFeature(BMI2_ANY_MOTION);
  bmi2_sens_config anyMotionConfig;
  anyMotionConfig.type = BMI2_ANY_MOTION;
  anyMotionConfig.cfg.any_motion.duration = 2;
  anyMotionConfig.cfg.any_motion.threshold = 1100;
  anyMotionConfig.cfg.any_motion.select_x = BMI2_ENABLE;
  anyMotionConfig.cfg.any_motion.select_y = BMI2_ENABLE;
  anyMotionConfig.cfg.any_motion.select_z = BMI2_ENABLE;
  imu.setConfig(anyMotionConfig);
  imu.mapInterruptToPin(BMI2_ANY_MOTION_INT, BMI2_INT1);

  // IMU Interrupt Setup
  bmi2_int_pin_config intPinConfig;
  intPinConfig.pin_type = BMI2_INT1;
  intPinConfig.int_latch = BMI2_INT_NON_LATCH;
  intPinConfig.pin_cfg[0].lvl = BMI2_INT_ACTIVE_HIGH;
  intPinConfig.pin_cfg[0].od = BMI2_INT_PUSH_PULL;
  intPinConfig.pin_cfg[0].output_en = BMI2_INT_OUTPUT_ENABLE;
  intPinConfig.pin_cfg[0].input_en = BMI2_INT_INPUT_DISABLE;
  imu.setInterruptPinConfig(intPinConfig);
  attachInterrupt(digitalPinToInterrupt(IMU_INT), imuInterruptHandler, RISING);
}
