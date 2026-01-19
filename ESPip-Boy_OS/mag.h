#include <Wire.h>

#define MAG_ADDR 0x1E
#define CFG_REG_A 0x60
#define CFG_REG_B 0x61


/* Lower power consumption of LIS2MDL is 25 uA:
 *  CFG_REG_A[LP] = 1 for low-power
 *  CFG_REG_B[OFF_CANC] = 0 
 */


void set_mag_low_power()
{
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x60);
  Wire.write(0x00);
  Wire.endTransmission();
}
