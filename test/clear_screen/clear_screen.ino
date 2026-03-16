#include <Arduino.h>
#include <Arduino_GFX_Library.h>


#define LCD_DC     27
#define LCD_RST    26
#define LCD_CS     15
#define LCD_SCK    18
#define LCD_BL     14
#define LCD_MOSI   23

#define IIC_SDA    4
#define IIC_SCL    16

#define TP_RST     25
#define TP_INT     33

#define IMU_INT 2

#define BL_PIN 14

#define LED_PIN 13

#define LCD_WIDTH  240
#define LCD_HEIGHT 300

Arduino_DataBus *bus; 
Arduino_GFX *gfx; 

void setup()
{

  Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
  Arduino_GFX *gfx = new Arduino_ST7789(
    bus,
    LCD_RST,          // RST pin
    0,                // rotation (0–3)
    true,             // IPS
    LCD_WIDTH,
    LCD_HEIGHT,
    0, 0, 0, 0       // x, y offsets for 240x280 ST7789 panels
  );

   if (!gfx->begin())
    {
      Serial.println("gfx->begin() failed!");
    }

    Serial.printf("Init display w: %d, h: %d\n", LCD_WIDTH, LCD_HEIGHT);

    //gfx->fillScreen(BACKGROUND); // misses bottom edge of screen
    gfx->fillRect(0, 0, 300, 300, RGB565_BLACK);
    //gfx->fillRect(0, 0, 240, 300, RGB565_RED);
}

void loop()
{
}

