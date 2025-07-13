#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define RX_PIN 16  // GPS TX → ESP32 RX
#define TX_PIN 17  // ESP32 TX → GPS RX (optional)

TinyGPSPlus gps;

// Use UART1
HardwareSerial gpsSerial(1);

void setup() {
  Serial.begin(38400);
  gpsSerial.begin(38400, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println(F("ESP32 - GPS module (HardwareSerial + Raw NMEA)"));
}

void loop() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();

    // Debug: print raw NMEA to Serial Monitor
    Serial.write(c);

    // Feed it to TinyGPS++
    gps.encode(c);
  }

  if (gps.location.isUpdated()) {
    Serial.println();
    Serial.println(F("==== Decoded GPS Data ===="));
    Serial.print(F("- latitude: "));
    Serial.println(gps.location.lat(), 6);

    Serial.print(F("- longitude: "));
    Serial.println(gps.location.lng(), 6);

    Serial.print(F("- altitude: "));
    if (gps.altitude.isValid())
      Serial.println(gps.altitude.meters());
    else
      Serial.println(F("INVALID"));

    Serial.print(F("- speed: "));
    if (gps.speed.isValid()) {
      Serial.print(gps.speed.kmph());
      Serial.println(F(" km/h"));
    } else {
      Serial.println(F("INVALID"));
    }

    Serial.print(F("- GPS date&time: "));
    if (gps.date.isValid() && gps.time.isValid()) {
      Serial.print(gps.date.year());
      Serial.print(F("-"));
      Serial.print(gps.date.month());
      Serial.print(F("-"));
      Serial.print(gps.date.day());
      Serial.print(F(" "));
      Serial.print(gps.time.hour());
      Serial.print(F(":"));
      Serial.print(gps.time.minute());
      Serial.print(F(":"));
      Serial.println(gps.time.second());
    } else {
      Serial.println(F("INVALID"));
    }

    Serial.println(F("=========================="));
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring and baud rate"));
  }
}
