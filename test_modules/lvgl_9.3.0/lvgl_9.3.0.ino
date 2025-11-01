#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "CST816S.h"
#include "demos/lv_demos.h"

#define LCD_DC 2
#define LCD_RST 4
#define LCD_CS 15
#define LCD_SCK 18
#define LCD_BL 21
#define LCD_MOSI 23

#define LCD_WIDTH 240
#define LCD_HEIGHT 280

#define IIC_SDA 13
#define IIC_SCL 14

#define TP_RST 16
#define TP_INT 17


Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST /* RST */,
                                      0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);  // sda, scl, rst, irq

// Adjust these to match your display size
static const uint16_t SCREEN_WIDTH = 240;
static const uint16_t SCREEN_HEIGHT = 380;

// Simple display buffer
static lv_color_t buf1[SCREEN_WIDTH * 20];

// --- Display flush callback (called when LVGL wants to draw) ---
static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t*)&px_map, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)&px_map, w, h);
#endif

  lv_disp_flush_ready(disp);
}

// --- OPTIONAL: Touch read callback ---
static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touch.available()) {

    data->state = LV_INDEV_STATE_PR;

    /* Set the coordinates with some debounce */
    data->point.x = touch.data.x;
    data->point.y = touch.data.y;

    Serial.printf("Data x: %d, Data y: %d\n", touch.data.x, touch.data.y);

  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// --- Arduino setup ---
void setup() {
  Serial.begin(115200);
  Serial.println("LVGL v9.3.0 demo starting...");

  lv_init();

  // Create display object
  lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Optional input device (touch)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // Run one of the built-in demos
  //lv_demo_widgets();
  // lv_demo_benchmark();
  // lv_demo_keypad_encoder();
  // lv_demo_music();
  // lv_demo_stress();
}

// --- Arduino loop ---
void loop() {
  lv_timer_handler();  // Let LVGL process GUI updates
  delay(5);
}

