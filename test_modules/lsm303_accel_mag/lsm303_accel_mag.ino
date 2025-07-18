#include <Wire.h>
#include <LSM303.h>

LSM303 compass;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  
  /*
  Calibration values; the default values of +/-32767 for each axis
  lead to an assumed magnetometer bias of 0. Use the Calibrate example
  program to determine appropriate values for your particular unit.
  */
  compass.m_min = (LSM303::vector<int16_t>){-603, -707, -548};
  compass.m_max = (LSM303::vector<int16_t>){+632,   +499,   +445};
  // Calibrate min/max readings  18:04:08.720 -> min: {  -603,   -707,   -548}    max: {  +632,   +499,   +445}
}

void loop() {
  compass.read();
  
  /*
  When given no arguments, the heading() function returns the angular
  difference in the horizontal plane between a default vector and
  north, in degrees.
  
  The default vector is chosen by the library to point along the
  surface of the PCB, in the direction of the top of the text on the
  silkscreen. This is the +X axis on the Pololu LSM303D carrier and
  the -Y axis on the Pololu LSM303DLHC, LSM303DLM, and LSM303DLH
  carriers.
  
  To use a different vector as a reference, use the version of heading()
  that takes a vector argument; for example, use
  
    compass.heading((LSM303::vector<int>){0, 0, 1});
  
  to use the +Z axis as a reference.
  */

  // I want to use Z as my chip is flat on the table rotating about Z axis
  // I am using -Z because I mounted my chip upside-down -> silkscreen on bottom haha
  static float headingArray[10] = {0}; // take average of 10 values
  static int i = 0;
  headingArray[i] = compass.heading((LSM303::vector<int>){0, 0, -1});
  i = (i + 1) % 10;

  // hello world test
  static float headingAvg = 0;
  for(int j = 0; j < 10; j++){
    headingAvg += headingArray[i];
  }
  headingAvg /= 10;
 
  Serial.println(headingAvg);
  Serial.println("WSL Vim Editor Test");
  delay(100);
}
