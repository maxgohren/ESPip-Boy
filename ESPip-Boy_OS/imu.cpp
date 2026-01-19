#include "SparkFun_BMI270_Arduino_Library.h"
#include <Wire.h>
#include "imu.h"
#include "display.h"
#include "sleep.h"

#define IMU_INT 2

// IMU Setup
BMI270 imu;
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68

volatile bool imuInterruptOccurred = false;

uint32_t step_count = 0;

// Time variables
unsigned long lastActiveTime = 0;
unsigned long lastFacingTime = 0;
unsigned long lastIMUReading = 0;

// Watch State
bool oldScreenOn = false;
bool oldWatchFacing = false;
bool imuScreenOn = false;
bool watchFacing = false;
double x = 0;
double y = 0;
double z = 0;

const int screen_focus_timeout_ms = 500;
const int watch_inactive_timeout = 1000;

static void imuInterruptHandler()
{
    imuInterruptOccurred = true;
}

static bool isWatchFacing() 
{
  if ( y > 0.5f && y < 0.9f && z > 0.5f && abs(x) < 0.5f ) {
    lastFacingTime = millis();
    return true;
  } else {
    return false;
  }
}

static void imu_handle_interrupt()
{
  if(imuInterruptOccurred)
  {
      // Reset flag and get status
      imuInterruptOccurred = false;
      uint16_t interruptStatus = 0;
      imu.getInterruptStatus(&interruptStatus);

      if(interruptStatus & BMI270_WRIST_WAKE_UP_STATUS_MASK){
          Serial.println("Wrist Focus Gesture Detected!");
      }

      if (interruptStatus & BMI270_ANY_MOT_STATUS_MASK){
          Serial.println("Motion Detected!");
      }

      // Check if this is the correct interrupt condition
      if(interruptStatus & BMI270_STEP_CNT_STATUS_MASK){
          imu.getStepCount(&step_count);
      }
  }
}

void handle_watch_orientation()
{
  imu_handle_interrupt();

  if (millis() - lastIMUReading >= 20) {
    // Update Sensor readings
    lastIMUReading = millis();
    imu.getSensorData(); // TODO make this a one-shot read on device, no need to poll higher in
                         // hardware than software processing time
    x = imu.data.accelX;
    y = imu.data.accelY;
    z = imu.data.accelZ;

    // Record old state to monitor change
    bool oldScreenOn = imuScreenOn;
    bool oldWatchFacing = watchFacing;

    // Calculate current state based on sensor data
    imuScreenOn = screen_is_on();
    watchFacing = isWatchFacing();

    // Only write when state changed
    if (imuScreenOn != oldScreenOn || watchFacing != oldWatchFacing){
      Serial.printf("Watch display: screen %s, facing? %s\n",
          imuScreenOn ? "On" : "Off",
          watchFacing ? "Yes" : "No");
    }

    // Handle screen and esp32 sleep logic with updated position logic
    if (imuScreenOn && watchFacing) {
      // update last active time
      lastActiveTime = millis();
    } else if (!imuScreenOn && watchFacing){
      display_screen_on();
    } else if (imuScreenOn && !watchFacing){
      // Timeout display if screen is on and not facing user for more than timeout
      long unsigned screen_not_focused_time = millis() - lastFacingTime;
      if (screen_not_focused_time > screen_focus_timeout_ms) {
        Serial.printf("Turning screen off, out of focus for %d ms.\n", screen_focus_timeout_ms);
        display_screen_off();
      }
    } else if (!imuScreenOn && !watchFacing){
      // Sleep watch if screen is off and not facing for more than timeout
      long unsigned watch_inactive_time = millis() - lastActiveTime;
      if (watch_inactive_time > watch_inactive_timeout) {
        Serial.printf("Watch inactive for %d ms, going to deep sleep\n", watch_inactive_timeout);
        go_sleep();
      }
    } //screen logic
  } //imu reading
}


int getStepCount()
{
  // Should be only doing this in interrupt routine, but lazily poll it for now
  imu.getStepCount(&step_count);
  return step_count;
}


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
  

void imu_set_low_power()
{
  // We can lower the power consumption by setting the power mode of each
  // sensor to the power optimized mode
  imu.setAccelPowerMode(BMI2_POWER_OPT_MODE);
  imu.setGyroPowerMode(BMI2_POWER_OPT_MODE, BMI2_POWER_OPT_MODE);

  // Lower ODR values result in less power consumption. To ensure the sensors
  // are syncronized, we'll choose the minimum ODR that both sensors support
  imu.setAccelODR(BMI2_ACC_ODR_25HZ);
  imu.setGyroODR(BMI2_GYR_ODR_25HZ);
  
  // The BMI270 has an advanced power save mode that can be used to minimize
  // current consumption. Note that this comes with some restrictions, see
  // datasheet for details
  imu.enableAdvancedPowerSave();

  // After the sensor has been configured, we can enter suspend mode by
  // disabling both the accelerometer and gyroscope. This can lower the
  // BMI270's current consumption down to 3.5uA
  imu.disableFeature(BMI2_ACCEL);
  imu.disableFeature(BMI2_GYRO);
}
