#pragma once
#include <Wire.h>

#define BQ27421_ADDRESS 0x55


class BQ27421 {
public:
    void begin(int sda, int scl) {
        Wire.begin(sda, scl);
    }

    uint16_t readVoltage() {
        return readWord(0x04); // mV
    }

    uint16_t readSOC() {
        return readWord(0x1C); // %
    }

    int16_t readCurrent() {
        return (int16_t)readWord(0x10); // mA (signed)
    }

    int16_t readAveragePower() {
        return (int16_t)readWord(0x18); // mW
    }

    void writeControlWord(uint16_t control) {
      Wire.beginTransmission(BQ27421_ADDRESS);
      Wire.write(0x00);
      Wire.write(control & 0xFF);
      Wire.write((control >> 8) & 0xFF);
      Wire.endTransmission();
    }

    uint16_t readControlWord() {
        Wire.beginTransmission(BQ27421_ADDRESS);
        Wire.write(0x00);
        Wire.endTransmission(false);
        Wire.requestFrom(BQ27421_ADDRESS, (uint8_t)2);
        uint16_t lsb = Wire.read();
        uint16_t msb = Wire.read();
        return (msb << 8) | lsb;
    }

    // Writes 2-byte data to a Data Memory block
    void writeDM(uint8_t subclass, uint8_t offset, uint16_t value) {
        // Block select
        Wire.beginTransmission(BQ27421_ADDRESS);
        Wire.write(0x3E);
        Wire.write(subclass);
        Wire.write(0x00); // Block 0
        Wire.endTransmission();

        // Write value (LSB first)
        Wire.beginTransmission(BQ27421_ADDRESS);
        Wire.write(0x40 + offset);
        Wire.write(value & 0xFF);
        Wire.write((value >> 8) & 0xFF);
        Wire.endTransmission();
    }

  void configureBattery() {
      // Unseal (0x8000 key twice)
      writeControlWord(0x8000);
      writeControlWord(0x8000);
      delay(10);

      // Enter CONFIG UPDATE mode
      writeControlWord(0x0013);
      delay(50);

      // Set parameters (Design Capacity / Energy / Termination voltage)
      writeDM(0x52, 10, 200);    // Design Capacity (mAh)
      writeDM(0x52, 12, 740);    // Design Energy (mWh) = 3.7V * 200mAh ≈ 740
      writeDM(0x52, 16, 3000);   // Terminate Voltage (mV)

      delay(10);

      // Exit config mode
      writeControlWord(0x0042);
      delay(100);

      // Seal again
      writeControlWord(0x0020);
  }


private:
    uint16_t readWord(uint8_t command) {
        Wire.beginTransmission(BQ27421_ADDRESS);
        Wire.write(command);
        Wire.endTransmission(false);

        Wire.requestFrom(BQ27421_ADDRESS, (uint8_t)2);
        uint16_t lsb = Wire.read();
        uint16_t msb = Wire.read();

        return (msb << 8) | lsb;
    }
};

// ----- Fuel Gauge -----
BQ27421 fuelGauge;
