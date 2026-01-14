#include "CST816S.h"
#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Arduino_GFX_Library.h>

#include "pinout.h"

#include "clock.h"
#include "display.h"
#include "sleep.h"
#include "fuel.h"
#include "loading_screen.h"
#include "rtc.h"

// IMU Setup
BMI270 imu;
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
volatile bool imuInterruptOccurred = false;
extern bool screenOn;
unsigned long lastWakeTime = 0;
unsigned long lastFacingTime = 0;
unsigned long lastIMUReading = 0;

// ----- Fuel Gauge -----
BQ27421 fuelGauge;

// ----- RTC ------
RTC rtc;

// ----- Display Setup -----
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(
  bus,
  LCD_RST,          // RST pin
  0,                // rotation (0–3)
  true,             // IPS
  LCD_WIDTH,
  LCD_HEIGHT,
  0, 20, 0, 0       // x, y offsets for 240x280 ST7789 panels
);

// ----- Touch Setup -----
// Touch screen currently doesn't work. Problem with chip I believe. Disabling
// to not interfere with imu setup.
//CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);

const long gmtOffset_sec = -5 * 60 * 60; // EST is -5

static int16_t w, h, center;
static int16_t hHandLen, mHandLen, sHandLen, markLen;
static float sdeg, mdeg, hdeg;
static int16_t osx = 0, osy = 0, omx = 0, omy = 0, ohx = 0, ohy = 0; // Saved H, M, S x & y coords
static int16_t nsx, nsy, nmx, nmy, nhx, nhy;                         // H, M, S x & y coords
static int16_t xMin, yMin, xMax, yMax;                               // redraw range
static int16_t hh, mm, ss;
static unsigned long targetTime; // analog target

static int16_t *cached_points;
static uint16_t cached_points_idx = 0;
static int16_t *last_cached_point;

void imuInterruptHandler()
{
    imuInterruptOccurred = true;
}

int readRTC(byte reg){
  Wire.beginTransmission(0x32);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(RTC_ADDR, 1);
  //while (Wire.available() == 0);
  return Wire.read();
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("ESPip-Boy init begin:");
  Wire.begin(IIC_SDA, IIC_SCL);

  // Backlight control
  display_bl_setup();
  display_screen_off();

  // Time init
  Serial.printf("ESPip-Boy %s begin.\n", "time");
  targetTime = ((millis() / 1000) + 1) * 1000;
  //struct tm tm;
  //time_t t = mktime(&tm);
  //struct timeval now = { .tv_sec = t };
  //settimeofday(&now, NULL);
  Serial.printf("ESPip-Boy %s complete.\n", "time");

  Serial.printf("ESPip-Boy %s begin.\n", "rtc");
  RTCDateTime rtc_time; 
  //rtc.init();
  // If first boot, set time  WiFi (TODO: compile flags). Else get time from RTC
  // TODO move to clock.h, pass in Wire handle
  if(get_boot_count() == 0){
     wifi_set_system_time(&Serial);
     //get_rtc_time_from_compile_time(__DATE__, __TIME__, &rtc_time);
    time_t now; 
    struct tm t;
    time(&now);
    localtime_r(&now, &t);

    rtc.setTime({ (uint8_t)t.tm_sec, (uint8_t)t.tm_min, (uint8_t)t.tm_hour });
    Serial.printf("Setting rtc time to %d:%d:%d\n", t.tm_hour, t.tm_min, t.tm_sec);
  } else {
    rtc_time = rtc.getTime();

    Serial.println("RTC Time: ");
    Serial.print(rtc_time.hours);
    Serial.print(":");
    Serial.print(rtc_time.minutes);
    Serial.print(":");
    Serial.println(rtc_time.seconds);
    Serial.printf("ESip-Boy %s complete.\n", "rtc");
  }

  // Init Display
  Serial.printf("ESPip-Boy %s begin:\n", "gfx->begin()"); 
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  Serial.printf("ESPip-Boy %s complete.", "gfx->begin()"); 


  Serial.println("Starting to init I2C.");
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

  // Setup fuel gauge
  fuelGauge.begin(4, 16);
  //fuelGauge.configureBattery(); Only really needs to be done once

  imu.enableFeature(BMI2_WRIST_WEAR_WAKE_UP);
  imu.mapInterruptToPin(BMI2_WRIST_WEAR_WAKE_UP_INT, BMI2_INT1);

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
  Serial.println("I2C init complete.");

  // Sleep
  init_sleep_mode(&Serial);

  // TODO fade lcd backlight in
  //ledcAttach(LCD_BL, pwmFreq, pwmResolution);
  //pinMode(0, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  //ledcAttach(LED_PIN, pwmFreq, pwmResolution);
  Serial.println("Flashlight init complete.");

  Serial.println("ESPip-Boy init complete.");
}

bool isWatchFacing(float x, float y, float z) {
  return ( y > 0.6f && y < 0.9f 
             && z > 0.5f && 
             abs(x) < 0.5f );
}

void loop()
{
  unsigned long currTime = millis();

  double x = imu.data.accelX;
  double y = imu.data.accelY;
  double z = imu.data.accelZ;

  if (currTime - lastIMUReading >= 20) {
    imu.getSensorData();
    lastIMUReading = currTime;
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
  }


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
          display_screen_on();
          lastWakeTime = millis();
          lastFacingTime = millis();
      }
  }

  // Screen off if watch is out of face up position
  if (screenOn) {

    if (isWatchFacing(x, y, z)) {
      lastFacingTime = millis();
    }

    currTime = millis();
    const int screen_focus_timeout_ms = 10000;
    if (currTime - lastFacingTime > screen_focus_timeout_ms) {
      Serial.printf("Screen has been out of focus for more than %d sec, turning off\n\n", screen_focus_timeout_ms / 1000);
      display_screen_off();
      go_sleep();
    }

  } else {
    if (isWatchFacing(x, y, z)){
      display_screen_on();
    }
  }

  if (currTime >= targetTime){
  
    targetTime += 1000;
    ss++; // Advance second
    if (ss == 60)
    {
      ss = 0;
      mm++; // Advance minute
      if (mm > 59)
      {
        mm = 0;
        hh++; // Advance hour
        if (hh > 23)
        {
          hh = 0;
        }
      }
    }

    // Digital clock logic
    time_t now;
    time(&now);
    now += gmtOffset_sec;
    struct tm *tmLocal = localtime(&now);

    // Text Formatting
    int textSize = gfx->width() / 94;
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
    gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);

    // Print Date /* helpful strftime formatting site: https://strftime.net/ */
    char dateStr[50];
    strftime(dateStr, sizeof(dateStr), "%a %e %b %Y", tmLocal);
    gfx->setCursor(20, 160);
    gfx->print(dateStr);

    // Text Formatting
    textSize = gfx->width() / 52;
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
    gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);

    // Print Time
    char timeStr[15];
    strftime(timeStr, sizeof(timeStr), "%I:%M %p", tmLocal);
    gfx->setCursor(20, 80);
    gfx->print(timeStr);

  }

  // Print Gas Values
  static int lastGasUpdate = 0;
  if (currTime - lastGasUpdate >= 1000){
    lastGasUpdate = currTime;
    int v =   fuelGauge.readVoltage();
    int soc = fuelGauge.readSOC();
    int pwr = fuelGauge.readAveragePower();
    int ma =  fuelGauge.readCurrent();

    /*
    fuelGauge.writeControlWord(0x0001);
    uint16_t device_id = fuelGauge.readControlWord();
    Serial.printf("Device ID: %x\n", device_id);
    */

    int textSize = gfx->width() / 94;
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
    gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);

    //Serial.print("SOC: ");
    //Serial.print(soc);
    //Serial.println(" %");
    static char fuelSOCStr[100];
    snprintf(fuelSOCStr, sizeof(fuelSOCStr), "BAT: [%d %]", soc);
    gfx->setCursor(20, 180);
    gfx->print(fuelSOCStr);
#if 0


    Serial.print("Voltage: ");
    Serial.print(v);
    Serial.println(" mV");
    char fuelVStr[100];
    snprintf(fuelVStr, sizeof(fuelVStr), "Voltage: %d mV", v);
    int textSize = gfx->width() / 120;
    gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);
    gfx->setCursor(20, 80);
    gfx->print(fuelVStr);

    Serial.print("Current: ");
    Serial.print(ma);
    Serial.println(" mA");
    char fuelAStr[100];
    snprintf(fuelAStr, sizeof(fuelAStr), "Current: %d mA", ma);
    gfx->setCursor(20, 120);
    gfx->print(fuelAStr);

    Serial.print("Power: ");
    Serial.print(ma);
    Serial.println(" mW");
    char fuelPStr[100];
    snprintf(fuelPStr, sizeof(fuelPStr), "Power: %d mW", pwr);
    gfx->setCursor(20, 140);
    gfx->print(fuelPStr);
#endif 
  }
}
