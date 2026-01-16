#pragma once

int io0 = LOW;
bool flashlight_on = false;
bool toggleState = false;
int last_toggle_time = 0;

void turn_flashlight_on()
{
  // Stops repeated calls from hammering function (ideally we shouldn't call
  // it more than once but that's a TODO
  if (flashlight_on) return;
  ledcFade(LED_PIN, 0 /* start duty */, 255 /* end duty */, 250 /* ms */);
  flashlight_on = true;
}

void turn_flashlight_off()
{
  // Stops repeated calls from hammering function (ideally we shouldn't call
  // it more than once but that's a TODO
  if (!flashlight_on) return;
  ledcFade(LED_PIN, 255 /* start duty */, 0 /* end duty */, 250 /* ms */);
  flashlight_on = false;
}

bool flash_on(){
  return flashlight_on;
}

void init_flashlight()
{
  // LED PWM Channel init
  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);

  // Button pullup
  pinMode(0, INPUT_PULLUP);
}

void handle_flashlight()
{
  // Read button press from IO0
  io0 = digitalRead(0);
  // If active low and more than 250 since last toggle
  if (io0 == 0 && millis() - 250 > last_toggle_time){
    last_toggle_time = millis();

    Serial.println("Button pressed.");
    toggleState ^= HIGH; 

    // Cannot use with ledcAttach
    //digitalWrite(LED_PIN, flashlight_on);

    if(toggleState == HIGH){
      turn_flashlight_on();
    } else {
      turn_flashlight_off();
    }
  }
}
