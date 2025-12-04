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

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Arduino_GFX Clock example");
  Serial.println(__TIME__);

  // Backlight control
  display_bl_setup();
  display_screen_on();

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BACKGROUND);
  gfx->draw16bitRGBBitmap(gfx->width() / 2 - IMG_WIDTH / 2 , 
                          gfx->height() / 2 - IMG_HEIGHT /2, 
                          (const uint16_t*)vault_boy_pip_boy_color, 
                          IMG_HEIGHT, 
                          IMG_WIDTH);
  Serial.printf("Finished drawing loading screen: w: %d, h: %d\n", gfx->width(), gfx->height());

  gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
  int textSize = gfx->width() / 94;
  gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);
  gfx->setCursor(20, 50); // Centered above pip
  gfx->print("Loading ESPip-Boy");


  // Set up time variables
  hh = conv2d(__TIME__);
  mm = conv2d(__TIME__ + 3);
  ss = conv2d(__TIME__ + 6);
  targetTime = ((millis() / 1000) + 1) * 1000;
  setClock(&Serial);

  Serial.println("ESPip-Boy init complete.");

  Wire.begin(IIC_SDA, IIC_SCL);
  // Init IMU
    while(imu.beginI2C(i2cAddress) != BMI2_OK)
  {
      Serial.println("Error: BMI270 not connected, check wiring and I2C address!");
      delay(1000);
  }
  Serial.println("BMI270 connected!");

  // Reconcile with BMI270 init
  fuelGauge.begin(4, 16);
  
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

  init_sleep_mode(&Serial);

  // Clear screen after loading
  gfx->fillScreen(BACKGROUND); 
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

    // Timeout display when out of focus
    /*
    currTime = millis();
    const int screen_focus_timeout_ms = 2000;
    if (currTime - lastFacingTime > screen_focus_timeout_ms) {
      Serial.printf("Screen has been out of focus for more than %d ms, turning off\n", screen_focus_timeout_ms);
      //display_screen_off();
      //go_sleep();
    }
    */

  } 

  if (!screenOn) {
    if (isWatchFacing(x, y, z)){
      display_screen_on();
    }
  }

  if (currTime >= targetTime)
  {
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
    gfx->setCursor(20, 20);
    gfx->print(dateStr);

    // Print Time
    char timeStr[15];
    strftime(timeStr, sizeof(timeStr), "%r", tmLocal);
    gfx->setCursor(20, 40);
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

    Serial.print("Voltage: ");
    Serial.print(v);
    Serial.println(" mV");
    char fuelVStr[100];
    snprintf(fuelVStr, sizeof(fuelVStr), "Voltage: %d mV", v);
    int textSize = gfx->width() / 120;
    gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);
    gfx->setCursor(20, 80);
    gfx->print(fuelVStr);

    Serial.print("SOC: ");
    Serial.print(soc);
    Serial.println(" %");
    char fuelSOCStr[100];
    snprintf(fuelSOCStr, sizeof(fuelSOCStr), "SOC: %d %", soc);
    gfx->setCursor(20, 100);
    gfx->print(fuelSOCStr);


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
  }

  // General timeout
  /*
  const int screen_timeout_ms = 10000;
  if (currTime - lastWakeTime > screen_timeout_ms) {
    Serial.printf("Watch has been on for more than %d s, turning off\n", screen_timeout_ms / 1000);
    lastWakeTime = currTime;
    //display_screen_off();
    //go_sleep();
  }
  */
}
