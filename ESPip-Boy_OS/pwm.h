// ESP32 PWM can generate 1 timer to use with different channels (BL/FLASH)
#define PWM_FREQ 5000
#define PWM_RES  8

void init_pwm()
{
  // Setup frequency and resolution for seperate channels
  // Attach a pin to a channel
  ledcAttach(BL_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);

  // Start with both off
  ledcWrite(BL_PIN, 0);
  ledcWrite(LED_PIN, 0);
}
