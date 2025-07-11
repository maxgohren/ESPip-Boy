/**
 * Combined Example: Play MP3 from SPIFFS to Bluetooth A2DP Source
 * Uses AudioSourceSPIFFS + MP3DecoderHelix + BluetoothA2DPSource
 * by Phil Schatzmann
 */

#include "AudioTools.h"
#include "AudioTools/Disk/AudioSourceSPIFFS.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/AudioLibs/A2DPStream.h"
#include "SPIFFS.h"

using namespace audio_tools;

// ----------- CONFIG -----------
const char* mp3File = "/morgan.mp3";
BluetoothA2DPSource a2dp_source;
MP3DecoderHelix mp3_decoder;
AudioSourceSPIFFS source;
File mp3_file;
EncodedAudioStream decoder;

// ----------- CALLBACK for A2DP -----------
int32_t get_sound_data(uint8_t* data, int32_t size) {
  int32_t bytesRead = decoder.readBytes(data, size);
  delay(1); // Feed the dog
  return bytesRead;
}

// ----------- SETUP -----------
void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed!");
    while (1);
  }

  mp3_file = SPIFFS.open("/morgan.mp3");
  if(!mp3_file){
    Serial.println("Failed to open MP3!");
    while(1);
  }

  decoder.setStream(&mp3_file);
  decoder.setDecoder(&mp3_decoder);
  // Make sure we have enough space for PCM output
  decoder.transformationReader().resizeResultQueue(1024 * 8);

  if (!decoder.begin()) {
    Serial.println("Decoder failed!");
    while (1);
  } 

  // Start A2DP Bluetooth Source
  Serial.println("Starting A2DP Bluetooth...");
  a2dp_source.set_data_callback(get_sound_data);
  a2dp_source.start("UE BOOM 2");
  //a2dp_source.start("Max's AirPods");
}

// ----------- LOOP -----------
void loop() {
  delay(1000);
}

