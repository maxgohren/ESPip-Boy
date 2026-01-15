#include "SparkFun_BMI270_Arduino_Library.h"
#include <Wire.h>
#include "display.h"
#include "sleep.h"

#define IMU_INT 2

// IMU Setup
BMI270 imu;

uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
volatile bool imuInterruptOccurred = false;
unsigned long lastWakeTime = 0;
unsigned long lastFacingTime = 0;
uint32_t step_count = 0;

unsigned long lastIMUReading = 0;
double x = 0;
double y = 0;
double z = 0;


int getStepCount()
{
  return step_count;
}

void imuInterruptHandler()
{
    imuInterruptOccurred = true;
}

    // Print acceleration data
    /*
    Serial.print("Acceleration in g's");
    Serial.print("\t");
    Serial.print("X: ");
    Serial.print(x, 3);
    Serial.print("\t");
    Serial.print("Y: ");
    Serial.print(y, 3);
    Serial.print("\t");
    Serial.print("Z: ");
    Serial.print(z, 3);
    Serial.println();
    */

void imu_init()
{
  // Init IMU
  while(imu.beginI2C(i2cAddress) != BMI2_OK)
  {
      static int retries = 0;
      Serial.println("Error: BMI270 not connected, check wiring and I2C address!");
      delay(1000);
      retries++;
      if (retries == 5) {
        break;
      }
      Serial.println("BMI270 not initialized, must press button to turn screen on");
  }

  imu.enableFeature(BMI2_WRIST_WEAR_WAKE_UP);
  imu.mapInterruptToPin(BMI2_WRIST_WEAR_WAKE_UP_INT, BMI2_INT1);

  /* Step Counter */
  imu.enableFeature(BMI2_STEP_DETECTOR);
  imu.enableFeature(BMI2_STEP_COUNTER);
  imu.enableFeature(BMI2_STEP_ACTIVITY);

  imu.setStepCountWatermark(1);
  // Will wake up device from sleep every time there is a step
  // I'm fine with polling this for now, or, we disable this before sleeping!
  // That way when display is on, we enable it for live updates
  //imu.mapInterruptToPin(BMI2_STEP_COUNTER_INT, BMI2_INT1);

  // Wrist Wakeup Feature Enable & Config
  bmi2_sens_config wristWakeUpConfig;
  wristWakeUpConfig.type = BMI2_WRIST_WEAR_WAKE_UP;
  wristWakeUpConfig.cfg.wrist_wear_wake_up.min_angle_focus = 1024;
  wristWakeUpConfig.cfg.wrist_wear_wake_up.min_angle_nonfocus = 1448;
  wristWakeUpConfig.cfg.wrist_wear_wake_up.max_tilt_lr = 1024;
  wristWakeUpConfig.cfg.wrist_wear_wake_up.max_tilt_ll = 700;
  wristWakeUpConfig.cfg.wrist_wear_wake_up.max_tilt_pd = 179;
  wristWakeUpConfig.cfg.wrist_wear_wake_up.max_tilt_pu = 1774;
  imu.setConfig(wristWakeUpConfig);

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
  Serial.println("IMU init complete.");
}
  
bool isWatchFacing() 
{
  const int screen_focus_timeout_ms = 2000;

  if ( y > 0.6f && y < 0.9f && z > 0.5f && abs(x) < 0.5f ) {
    lastFacingTime = millis();
    //Serial.println("Watch is focused...");
    return true;
  } else {
    //Serial.println("Watch is unfocused...");
    if (millis() - lastFacingTime > screen_focus_timeout_ms) {
      Serial.printf("Screen out of focus for more than %d sec, turning off\n\n", screen_focus_timeout_ms / 1000);
      go_sleep();
    }
    return false;
  }
}

void handle_watch_orientation()
{
  if (millis() - lastIMUReading >= 20) {
    lastIMUReading = millis();
    imu.getSensorData();
    x = imu.data.accelX;
    y = imu.data.accelY;
    z = imu.data.accelZ;

    // Screen off if watch is out of face up position
    if (screen_is_on()) {
      isWatchFacing(); // update lastFacing Time
    } else {
      if (isWatchFacing()){
        display_screen_on();
      }
    }
  }
}

void imu_handle_interrupt()
{
  if(imuInterruptOccurred)
  {
      // Reset flag and get status
      imuInterruptOccurred = false;
      uint16_t interruptStatus = 0;
      imu.getInterruptStatus(&interruptStatus);

      if(interruptStatus & BMI270_WRIST_WAKE_UP_STATUS_MASK)
      {
          Serial.println("Wrist Focus Gesture Detected!");
          display_screen_on();
          lastWakeTime = millis();
          lastFacingTime = millis();
      }

      if (interruptStatus & BMI270_ANY_MOT_STATUS_MASK)
      {
          Serial.println("Motion Detected!");
          lastWakeTime = millis();
          //TODO
          //if (isWatchFacing())
          //    display_screen_on();
      }

      // Check if this is the correct interrupt condition
      if(interruptStatus & BMI270_STEP_CNT_STATUS_MASK)
      {
          imu.getStepCount(&step_count);
      }
  }
}
