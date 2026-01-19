#pragma once
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Wire.h>

int getStepCount();
void imu_init();
void handle_watch_orientation();
void imu_set_low_power();

