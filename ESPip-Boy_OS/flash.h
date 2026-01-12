
// LED settings
/*
const int pwmFreq = 5000;     // Hz
const int pwmResolution = 8;  // bits (0–255)
int io0 = LOW;
int flashlight = 0;
int last_toggle_time = 0;
// Fade in
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
*/

  // Flashlight thread
  io0 = digitalRead(0);
  if (io0 == 0 && millis() - 250 > last_toggle_time){
    last_toggle_time = millis();
    Serial.println("Button pressed. Toggling flashlight.");
    flashlight ^= HIGH; // toggle button
    digitalWrite(LED_PIN, flashlight);

    // TODO fade flashlight in/out
    /*
    if(flashlight)
      led_fade_out();
    else
      led_fade_in();
    */

  }
