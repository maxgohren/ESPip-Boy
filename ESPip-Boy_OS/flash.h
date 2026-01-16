#pragma once

int io0 = LOW;
bool flashlight_on = false;
int last_toggle_time = 0;

bool flash_on(){
  return flashlight_on;
}

void init_flashlight()
{
  // Can't use with ledcAttach from pwm.h
  //pinMode(0, INPUT_PULLUP);
  //pinMode(LED_PIN, OUTPUT);
}

void handle_flashlight()
{
  return;
  // Read button press from IO0
  io0 = digitalRead(0);

  // If active low and more than 250 since last toggle
  if (io0 == 0 && millis() - 250 > last_toggle_time){
    last_toggle_time = millis();

    Serial.println("Button pressed. Toggling flashlight.");
    flashlight_on ^= HIGH; 

    // Cannot use with ledcAttach
    //digitalWrite(LED_PIN, flashlight_on);

    if(flashlight_on)
      ledcFade(LED_PIN, 0 /* start duty */, 255 /* end duty */, 250 /* ms */);
    else
      ledcFade(LED_PIN, 255 /* start duty */, 0 /* end duty */, 250 /* ms */);

  }
}
