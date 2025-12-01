#include "CST816S.h"
#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Arduino_GFX_Library.h>
#include "display.h"

// ----- Pin Definitions -----
#define LCD_DC     27
#define LCD_RST    26
#define LCD_CS     15
#define LCD_SCK    18
#define LCD_BL     14
#define LCD_MOSI   23

#define LCD_WIDTH  240
#define LCD_HEIGHT 280

#define IIC_SDA    4
#define IIC_SCL    16

#define TP_RST     25
#define TP_INT     33

#define IMU_INT 2

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

// IMU Setup
BMI270 imu;
uint8_t i2cAddress = BMI2_I2C_PRIM_ADDR; // 0x68
volatile bool imuInterruptOccurred = false;
extern bool screenOn;
unsigned long lastWakeTime = 0;
unsigned long lastFacingTime = 0;
unsigned long lastIMUReading = 0;

static int16_t w, h, center;
static int16_t hHandLen, mHandLen, sHandLen, markLen;
static float sdeg, mdeg, hdeg;
static int16_t osx = 0, osy = 0, omx = 0, omy = 0, ohx = 0, ohy = 0; // Saved H, M, S x & y coords
static int16_t nsx, nsy, nmx, nmy, nhx, nhy;                         // H, M, S x & y coords
static int16_t xMin, yMin, xMax, yMax;                               // redraw range
static int16_t hh, mm, ss;
static unsigned long targetTime; // next action time

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

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BACKGROUND);

  // Backlight control
  display_bl_setup();
  display_screen_off();

  // init LCD constant
  w = gfx->width();
  h = gfx->height();
  if (w < h)
  {
    center = w / 2;
  }
  else
  {
    center = h / 2;
  }
  hHandLen = center * 3 / 8;
  mHandLen = center * 2 / 3;
  sHandLen = center * 5 / 6;
  markLen = sHandLen / 6;
  cached_points = (int16_t *)malloc((hHandLen + 1 + mHandLen + 1 + sHandLen + 1) * 2 * 2);

  // Draw 60 clock marks
  draw_round_clock_mark(
      // draw_square_clock_mark(
      center - markLen, center,
      center - (markLen * 2 / 3), center,
      center - (markLen / 2), center);

  hh = conv2d(__TIME__);
  mm = conv2d(__TIME__ + 3);
  ss = conv2d(__TIME__ + 6);

  targetTime = ((millis() / 1000) + 1) * 1000;

  Wire.begin(IIC_SDA, IIC_SCL);
  // Init IMU
    while(imu.beginI2C(i2cAddress) != BMI2_OK)
  {
      Serial.println("Error: BMI270 not connected, check wiring and I2C address!");
      delay(1000);
  }
  Serial.println("BMI270 connected!");
  imu.enableFeature(BMI2_WRIST_WEAR_WAKE_UP);
  //imu.enableFeature(BMI2_WRIST_GESTURE);
  imu.mapInterruptToPin(BMI2_WRIST_WEAR_WAKE_UP_INT, BMI2_INT1);
  //imu.mapInterruptToPin(BMI2_WRIST_GESTURE_INT, BMI2_INT1);

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
  }

  if(imuInterruptOccurred)
  {
      // Reset flag and get status
      imuInterruptOccurred = false;
      uint16_t interruptStatus = 0;
      imu.getInterruptStatus(&interruptStatus);

      if(interruptStatus & BMI270_WRIST_WAKE_UP_STATUS_MASK)
      {
          Serial.println("Wrist Focus Gesture Detected! Turning backlight on!");
          display_screen_on();
          lastWakeTime = millis();
          lastFacingTime = millis();
      }
  }

  // Screen off if watch is out of face up position
  if (screenOn) {

    if (isWatchFacing(x, y, z)) {
      lastFacingTime = millis();
      Serial.printf("Last facing time: %lu\n", lastFacingTime);
    }

    // Timeout display when out of focus
    currTime = millis();
    const int screen_focus_timeout_ms = 2000;
    if (currTime - lastFacingTime > screen_focus_timeout_ms) {
      Serial.printf("Screen has been out of focus for more than %d ms, turning off\n", screen_focus_timeout_ms);
      display_screen_off();
    }

    // General timeout
    const int screen_timeout_ms = 15000;
    if (millis() - lastWakeTime > screen_timeout_ms) {
      Serial.printf("Screen has been on for more than %d, turning off\n", screen_timeout_ms);
      display_screen_off();
    }
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
  }

  // Pre-compute hand degrees, x & y coords for a fast screen update
  sdeg = SIXTIETH_RADIAN * ((0.001 * (currTime % 1000)) + ss); // 0-59 (includes millis)
  nsx = cos(sdeg - RIGHT_ANGLE_RADIAN) * sHandLen + center;
  nsy = sin(sdeg - RIGHT_ANGLE_RADIAN) * sHandLen + center;
  if ((nsx != osx) || (nsy != osy))
  {
    mdeg = (SIXTIETH * sdeg) + (SIXTIETH_RADIAN * mm); // 0-59 (includes seconds)
    hdeg = (TWELFTH * mdeg) + (TWELFTH_RADIAN * hh);   // 0-11 (includes minutes)
    mdeg -= RIGHT_ANGLE_RADIAN;
    hdeg -= RIGHT_ANGLE_RADIAN;
    nmx = cos(mdeg) * mHandLen + center;
    nmy = sin(mdeg) * mHandLen + center;
    nhx = cos(hdeg) * hHandLen + center;
    nhy = sin(hdeg) * hHandLen + center;

    // redraw hands
    redraw_hands_cached_draw_and_erase();

    ohx = nhx;
    ohy = nhy;
    omx = nmx;
    omy = nmy;
    osx = nsx;
    osy = nsy;
  }
}

void draw_round_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3)
{
  float x, y;
  int16_t x0, x1, y0, y1, innerR, outerR;
  uint16_t c;

  for (uint8_t i = 0; i < 60; i++)
  {
    if ((i % 15) == 0)
    {
      innerR = innerR1;
      outerR = outerR1;
      c = MARK_COLOR;
    }
    else if ((i % 5) == 0)
    {
      innerR = innerR2;
      outerR = outerR2;
      c = MARK_COLOR;
    }
    else
    {
      innerR = innerR3;
      outerR = outerR3;
      c = SUBMARK_COLOR;
    }

    mdeg = (SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN;
    x = cos(mdeg);
    y = sin(mdeg);
    x0 = x * outerR + center;
    y0 = y * outerR + center;
    x1 = x * innerR + center;
    y1 = y * innerR + center;

    gfx->drawLine(x0, y0, x1, y1, c);
  }
}

void draw_square_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3)
{
  float x, y;
  int16_t x0, x1, y0, y1, innerR, outerR;
  uint16_t c;

  for (uint8_t i = 0; i < 60; i++)
  {
    if ((i % 15) == 0)
    {
      innerR = innerR1;
      outerR = outerR1;
      c = MARK_COLOR;
    }
    else if ((i % 5) == 0)
    {
      innerR = innerR2;
      outerR = outerR2;
      c = MARK_COLOR;
    }
    else
    {
      innerR = innerR3;
      outerR = outerR3;
      c = SUBMARK_COLOR;
    }

    if ((i >= 53) || (i < 8))
    {
      x = tan(SIXTIETH_RADIAN * i);
      x0 = center + (x * outerR);
      y0 = center + (1 - outerR);
      x1 = center + (x * innerR);
      y1 = center + (1 - innerR);
    }
    else if (i < 23)
    {
      y = tan((SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN);
      x0 = center + (outerR);
      y0 = center + (y * outerR);
      x1 = center + (innerR);
      y1 = center + (y * innerR);
    }
    else if (i < 38)
    {
      x = tan(SIXTIETH_RADIAN * i);
      x0 = center - (x * outerR);
      y0 = center + (outerR);
      x1 = center - (x * innerR);
      y1 = center + (innerR);
    }
    else if (i < 53)
    {
      y = tan((SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN);
      x0 = center + (1 - outerR);
      y0 = center - (y * outerR);
      x1 = center + (1 - innerR);
      y1 = center - (y * innerR);
    }
    gfx->drawLine(x0, y0, x1, y1, c);
  }
}

void redraw_hands_cached_draw_and_erase()
{
  gfx->startWrite();
  draw_and_erase_cached_line(center, center, nsx, nsy, SECOND_COLOR, cached_points, sHandLen + 1, false, false);
  draw_and_erase_cached_line(center, center, nhx, nhy, HOUR_COLOR, cached_points + ((sHandLen + 1) * 2), hHandLen + 1, true, false);
  draw_and_erase_cached_line(center, center, nmx, nmy, MINUTE_COLOR, cached_points + ((sHandLen + 1 + hHandLen + 1) * 2), mHandLen + 1, true, true);
  gfx->endWrite();
}

void draw_and_erase_cached_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color, int16_t *cache, int16_t cache_len, bool cross_check_second, bool cross_check_hour)
{
#if defined(ESP8266)
  yield();
#endif
  bool steep = _diff(y1, y0) > _diff(x1, x0);
  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  int16_t dx, dy;
  dx = _diff(x1, x0);
  dy = _diff(y1, y0);

  int16_t err = dx / 2;
  int8_t xstep = (x0 < x1) ? 1 : -1;
  int8_t ystep = (y0 < y1) ? 1 : -1;
  x1 += xstep;
  int16_t x, y, ox, oy;
  for (uint16_t i = 0; i <= dx; i++)
  {
    if (steep)
    {
      x = y0;
      y = x0;
    }
    else
    {
      x = x0;
      y = y0;
    }
    ox = *(cache + (i * 2));
    oy = *(cache + (i * 2) + 1);
    if ((x == ox) && (y == oy))
    {
      if (cross_check_second || cross_check_hour)
      {
        write_cache_pixel(x, y, color, cross_check_second, cross_check_hour);
      }
    }
    else
    {
      write_cache_pixel(x, y, color, cross_check_second, cross_check_hour);
      if ((ox > 0) || (oy > 0))
      {
        write_cache_pixel(ox, oy, BACKGROUND, cross_check_second, cross_check_hour);
      }
      *(cache + (i * 2)) = x;
      *(cache + (i * 2) + 1) = y;
    }
    if (err < dy)
    {
      y0 += ystep;
      err += dx;
    }
    err -= dy;
    x0 += xstep;
  }
  for (uint16_t i = dx + 1; i < cache_len; i++)
  {
    ox = *(cache + (i * 2));
    oy = *(cache + (i * 2) + 1);
    if ((ox > 0) || (oy > 0))
    {
      write_cache_pixel(ox, oy, BACKGROUND, cross_check_second, cross_check_hour);
    }
    *(cache + (i * 2)) = 0;
    *(cache + (i * 2) + 1) = 0;
  }
}

void write_cache_pixel(int16_t x, int16_t y, int16_t color, bool cross_check_second, bool cross_check_hour)
{
  int16_t *cache = cached_points;
  if (cross_check_second)
  {
    for (uint16_t i = 0; i <= sHandLen; i++)
    {
      if ((x == *(cache++)) && (y == *(cache)))
      {
        return;
      }
      cache++;
    }
  }
  if (cross_check_hour)
  {
    cache = cached_points + ((sHandLen + 1) * 2);
    for (uint16_t i = 0; i <= hHandLen; i++)
    {
      if ((x == *(cache++)) && (y == *(cache)))
      {
        return;
      }
      cache++;
    }
  }
  gfx->writePixel(x, y, color);
}

