#include <lvgl.h>
#include <ui.h>
//#include "/home/max/Arduino/libraries/ui/fonts/jetbrains_mono_italic_96.h"
// #define DIRECT_RENDER_MODE // Uncomment to enable full frame buffer
#include "CST816S.h"
#include <Arduino_GFX_Library.h>
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
#define BL_PIN 14
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(
    bus,
    LCD_RST,
    0,      // rotation
    true,   // IPS
    LCD_WIDTH,
    LCD_HEIGHT,
    0, 20, 0, 0
);

CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);  // Touch setup
uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_disp_draw_buf_t draw_buf;
lv_color_t *disp_draw_buf;
lv_disp_drv_t disp_drv;
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2
void example_increase_lvgl_tick(void *arg) {
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
#ifndef DIRECT_RENDER_MODE
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
#endif // #ifndef DIRECT_RENDER_MODE

  lv_disp_flush_ready(disp_drv);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
  if (touch.available())
  {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touch.data.x;
      data->point.y = touch.data.y;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void setup()
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX LVGL_Arduino_v8 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  touch.begin();

  // Turn on backlight
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();

#ifdef DIRECT_RENDER_MODE
  bufSize = screenWidth * screenHeight;
#else
  bufSize = screenWidth * 40;
#endif

#ifdef ESP32
#if defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
  disp_draw_buf = (lv_color_t *)gfx->getFramebuffer();
#else  // !(defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)))
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!disp_draw_buf)
  {
    // remove MALLOC_CAP_INTERNAL flag try again
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
  }
#endif // !(defined(DIRECT_RENDER_MODE) && (defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)))
#else  // !ESP32
  Serial.println("LVGL disp_draw_buf heap_caps_malloc failed! malloc again...");
  disp_draw_buf = (lv_color_t *)malloc(bufSize * 2);
#endif // !ESP32
  if (!disp_draw_buf)
  {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  }
  else
  {
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, bufSize);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 280;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
#ifdef DIRECT_RENDER_MODE
    disp_drv.direct_mode = true;
#endif
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    /* Setup hardware timer */
    const esp_timer_create_args_t lvgl_tick_timer_args = { .callback = &example_increase_lvgl_tick, .name = "lvgl_tick" };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

    //lv_obj_t *label = lv_label_create(lv_scr_act());
    //lv_label_set_text(label, "12345");
    //lv_obj_set_style_text_font(label, &jetbrains_mono_96, LV_PART_MAIN);
    //lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    // EEZ Studio UI
    ui_init();
  }

  Serial.println("Setup done");
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */

  ui_tick();
  delay(5);
}

