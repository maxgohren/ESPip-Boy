/* Inspired by u/quarl0w and
 * https://github.com/kuglee/TermiWatch?tab=readme-ov-file 
 */

#include <Arduino_GFX_Library.h>
#include "flash.h"

#define BACKGROUND RGB565_BLACK

int  textSize;
char textLine[100];
char subStr[50];

// ----- Display Setup -----
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

// ----- Touch Setup -----
//CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);

//void init_termiwatch(Arduino_GFX *gfx){
void init_termiwatch(){
    if (!gfx->begin())
    {
      Serial.println("gfx->begin() failed!");
    }

    gfx->fillScreen(BACKGROUND);

    textSize = 2.5; // pixel multiplier
    gfx->setTextSize(textSize, textSize, 2 /* pixel_margin */);
}

void draw_termiwatch()
{
    int vpos = 55;
    int hpos = 15;
    int gap = 25;

    // Digital clock logic
    // TODO abstract into clock.h
    time_t now;
    time(&now);
    struct tm *tmLocal = localtime(&now); // must have called rtc.init() to set TZ

    /* user@watch:~ $now */
    snprintf(textLine, sizeof(textLine), "max@watch:~ $ now");
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    /* Time */
    strftime(subStr, sizeof(subStr), "%I:%M %p", tmLocal);
    snprintf(textLine, sizeof(textLine), "[TIME] %s", subStr);
    gfx->setTextColor(RGB565_CYAN, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    /* Date */
    strftime(subStr, sizeof(subStr), "%a %e %b", tmLocal);
    snprintf(textLine, sizeof(textLine), "[DATE] %s", subStr);
    gfx->setTextColor(RGB565_RED, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    /* TEMP */
    snprintf(textLine, sizeof(textLine), "[TEMP] %.1f C", rtc.getTemp());
    gfx->setTextColor(RGB565_THISTLE, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    /* BATV */
    // rough clear logic
    // static old_v = 0;
    /* int v = fuelGauge.readVoltage();
     * if (v != old_v)
     *  gfx->drawRect() (or even just an empty line)
     */
    snprintf(textLine, sizeof(textLine), "[BATV] %d mV", fuelGauge.readVoltage());
    gfx->setCursor(hpos, vpos);
    gfx->setTextColor(RGB565_ORANGE, RGB565_BLACK);
    gfx->print(textLine);
    vpos += gap;

    /* BATA */
    snprintf(textLine, sizeof(textLine), "[BATA] %d mA", fuelGauge.readCurrent());
    gfx->setTextColor(RGB565_SLATEGRAY, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    /* STEP */
    snprintf(textLine, sizeof(textLine), "[STEP] %d", getStepCount());
    gfx->setTextColor(RGB565_CHARTREUSE, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    /* FLSH */
    snprintf(textLine, sizeof(textLine), "[FLSH] %s", flash_on() ? "ON" : "OFF");
    gfx->setTextColor(RGB565_TURQUOISE, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;

    snprintf(textLine, sizeof(textLine), "max@watch:~ $");
    gfx->setTextColor(RGB565_WHITE, RGB565_BLACK);
    gfx->setCursor(hpos, vpos);
    gfx->print(textLine);
    vpos += gap;
}
