// use 8 bit precision for LEDC timer
#define LEDC_TIMER_8_BIT 8

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 5000

// LED pins
#define LED_PIN_1 22

// LED channel that will be used instead of automatic selection.
#define LEDC_CHANNEL 0

int brightness = 0;  // how bright the LED is

void setup() {
  // Use single LEDC channel 0 for both pins
  ledcAttachChannel(LED_PIN_1, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT, LEDC_CHANNEL);
}

void loop() {
  // fade loop
  while(brightness < 255){
    ledcWriteChannel(LEDC_CHANNEL, brightness /* brightness 0-255 */);
    delay(30);
    brightness += 5;
  }

  while(brightness > 0){
    ledcWriteChannel(LEDC_CHANNEL, brightness /* brightness 0-255 */);
    delay(30);
    brightness -= 5;
  }
}
