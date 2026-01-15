#pragma once
// LED settings
const int pwmFreq = 5000;     // Hz
const int pwmResolution = 8;  // bits (0–255)

int io0 = LOW;
bool flashlight_on = false;
int last_toggle_time = 0;

bool flash_on()
{
  return flashlight_on;
}
void led_fade_in() {
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(LED_PIN, duty);
    delay(10);
  }
}
// Fade out
void led_fade_out() {
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(LED_PIN, duty);
    delay(10);
  }
}

void init_flashlight()
{
  pinMode(0, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  // digitalWrite no longer works if ledcAttach is called, must use ledcWrite()
  //ledcAttach(LED_PIN, pwmFreq, pwmResolution);
}

void handle_flashlight()
{
  io0 = digitalRead(0);
  if (io0 == 0 && millis() - 250 > last_toggle_time){
    last_toggle_time = millis();
    //Serial.println("Button pressed. Toggling flashlight.");
    flashlight_on ^= HIGH; // toggle button
    digitalWrite(LED_PIN, flashlight_on);

    // TODO fade flashlight in/out
    /*
    if(flashlight)
      led_fade_out();
    else
      led_fade_in();
    */

  }
}
