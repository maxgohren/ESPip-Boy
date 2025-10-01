#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <driver/i2s.h>

// I2S microphone pins
#define I2S_WS 33
#define I2S_SCK 32
#define I2S_SD 35

// SD card SPI pins
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_CLK 18

#define I2S_SAMPLE_RATE 16000
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define I2S_NUM_CHANNELS 1
#define RECORD_TIME_SEC 30  // recording length in seconds
#define WAV_FILENAME "/test.wav"

// I2S config
i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = I2S_SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = I2S_COMM_FORMAT_I2S,
  .intr_alloc_flags = 0,
  .dma_buf_count = 4,
  .dma_buf_len = 1024,
  .use_apll = false
};

i2s_pin_config_t pin_config = {
  .bck_io_num = I2S_SCK,
  .ws_io_num = I2S_WS,
  .data_out_num = -1, // not used
  .data_in_num = I2S_SD
};

void writeWavHeader(File &file, uint32_t sampleRate, uint32_t bitsPerSample, uint32_t channels, uint32_t dataLength) {
  uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
  uint16_t blockAlign = channels * bitsPerSample / 8;

  file.seek(0);
  file.write((const uint8_t*)"RIFF", 4);
  uint32_t chunkSize = 36 + dataLength;
  file.write((const uint8_t*)&chunkSize, 4);
  file.write((const uint8_t*)"WAVE", 4);

  // fmt subchunk
  file.write((const uint8_t*)"fmt ", 4);
  uint32_t subChunk1Size = 16;
  file.write((const uint8_t*)&subChunk1Size, 4);
  uint16_t audioFormat = 1; // PCM
  file.write((const uint8_t*)&audioFormat, 2);
  file.write((const uint8_t*)&channels, 2);
  file.write((const uint8_t*)&sampleRate, 4);
  file.write((const uint8_t*)&byteRate, 4);
  file.write((const uint8_t*)&blockAlign, 2);
  file.write((const uint8_t*)&bitsPerSample, 2);

  // data subchunk
  file.write((const uint8_t*)"data", 4);
  file.write((const uint8_t*)&dataLength, 4);
}

void setup() {
  Serial.begin(115200);

  // Init SD card
  SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  if(!SD.begin(SD_CS)){
    Serial.println("SD Card mount failed!");
    while(1);
  }

  // Create WAV file
  File wavFile = SD.open(WAV_FILENAME, FILE_WRITE);
  if(!wavFile){
    Serial.println("Failed to open file for writing");
    while(1);
  }

  // Placeholder WAV header (we will rewrite later)
  writeWavHeader(wavFile, I2S_SAMPLE_RATE, 16, 1, 0);

  // Init I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  // Record
  uint32_t sampleCount = I2S_SAMPLE_RATE * RECORD_TIME_SEC;
  int16_t buffer[512];
  uint32_t bytesWritten = 0;

  Serial.println("Recording...");

  while(sampleCount > 0){
    size_t bytesRead;
    uint32_t toRead = min(sampleCount, (uint32_t)(sizeof(buffer)/sizeof(buffer[0])));
    i2s_read(I2S_NUM_0, buffer, toRead * 2, &bytesRead, portMAX_DELAY);
    wavFile.write((uint8_t*)buffer, bytesRead);
    bytesWritten += bytesRead;
    sampleCount -= bytesRead / 2;
  }

  Serial.println("Recording finished");

  // Rewrite WAV header with actual data length
  writeWavHeader(wavFile, I2S_SAMPLE_RATE, 16, 1, bytesWritten);

  wavFile.close();
  i2s_driver_uninstall(I2S_NUM_0);
}

void loop() {
  // Nothing here
}
