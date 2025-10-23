#include <Wire.h>
#include <MPU6050.h>

#define SDA_PIN 13
#define SCL_PIN 14
#define INT_PIN 33

MPU6050 mpu;

volatile bool motionDetected = false;

void IRAM_ATTR onMotion() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0){
    Serial.println("Woke up from MPU6050 motion interrupt!");
  }

  Wire.begin(SDA_PIN, SCL_PIN);
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }

  // Enable motion detection interrupt
  mpu.setSleepEnabled(false);
  mpu.setMotionDetectionThreshold(10);   // 2 = very sensitive, increase if too noisy
  mpu.setMotionDetectionDuration(1);    // 1 = short duration
  mpu.setIntMotionEnabled(true);
  mpu.setIntDataReadyEnabled(false);
  mpu.setDHPFMode(MPU6050_DHPF_5);    // Optional filtering
  mpu.setWakeCycleEnabled(false);

  // Clear any pending interrupts
  mpu.getIntStatus();

  // Attach interrupt
  pinMode(INT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), onMotion, RISING);

  Serial.println("MPU6050 ready.");
  Serial.println("Going to deep sleep now. Move MPU6050 to wake ESP32");

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); // Wake on HIGH? Isn't it active low?
  // Stabilize MPU 
  delay(100);
  esp_deep_sleep_start();
  

}

void loop() {
  /* Never reached in sleep 
  if (motionDetected) {
    motionDetected = false;
    Serial.println("Motion detected!");
    mpu.getIntStatus(); // clear interrupt
  }
  */
}
