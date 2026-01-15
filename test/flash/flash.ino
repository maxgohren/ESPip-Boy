#include "../ESPip-Boy_OS/pinout.h"

int io0 = LOW;
int flashlight = 0;
int last_toggle_time = 0;

void setup() {
  Serial.begin(9600);

  // Test interrupt
  // attachInterrupt(0, 
  
  // Test button
  pinMode(0, INPUT_PULLUP);

  // Flashlight LED Pin
  pinMode(LED_PIN, OUTPUT);

  // Turn off backlight
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
}


void loop(){
  // Toggle button logic
  io0 = digitalRead(0);
  if (io0 == 0 && millis() - 250 > last_toggle_time){
    last_toggle_time = millis();
    Serial.println("Button pressed. Toggling flashlight.");
    flashlight ^= HIGH; // toggle button
    digitalWrite(LED_PIN, flashlight);
  }
}

