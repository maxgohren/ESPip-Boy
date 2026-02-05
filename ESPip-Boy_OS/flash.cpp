#include "Arduino.h"
#include "flash.h"
#include "pinout.h"

int io0 = LOW;
bool flashlight_on = false;
bool oldToggleState = false;
bool toggleState = false;
int last_toggle_time = 0;

void toggle_flashlight()
{
  if (flash_on()){
    turn_flashlight_off();
  } else {
    turn_flashlight_on();
  }
}

bool flash_on(){
  return flashlight_on;
}

void turn_flashlight_on()
{
  // Stops repeated calls from hammering function (ideally we shouldn't call
  // it more than once but that's a TODO
  if (flashlight_on) return;
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(LED_PIN, duty);
    delay(1);
  }
  flashlight_on = true;
}

void turn_flashlight_off()
{
  // Stops repeated calls from hammering function (ideally we shouldn't call
  // it more than once but that's a TODO
  if (!flashlight_on) return;
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(LED_PIN, duty);
    delay(1);
  }
  flashlight_on = false;
}

void init_flashlight()
{
  // LED PWM Channel init
  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);

  // Button pullup
  pinMode(0, INPUT_PULLUP);
}

// Make sure flashlight on/off has nothing to do with button state, they are
// not related. Only the handling will decide what button presses mean with
// respects to flashlight on or off
void handle_flashlight()
{
  // Read button press from IO0
  io0 = digitalRead(0);
  // If active low and more than 250 since last toggle
  if (io0 == 0 && millis() - 250 > last_toggle_time){
    last_toggle_time = millis();

    Serial.println("Button pressed.");
    oldToggleState = toggleState;
    toggleState ^= HIGH; 

    // Only interact with flashlight when button was pressed
    if (toggleState != oldToggleState){
      if(toggleState == HIGH){
        turn_flashlight_on();
      } else {
        turn_flashlight_off();
      }
    }
  }
}
