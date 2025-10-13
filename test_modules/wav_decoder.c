#include <stdio.h>
#include <stdlib.h>

const char* filename = "test.wav";

int main(){
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("fopen");
    exit(1);
  }

  char RIFF[4];
  size_t readBytes = fread(&RIFF, sizeof(char), sizeof(RIFF), file);
  if (readBytes != sizeof(RIFF)) { perror("fread"); exit(1); }
  for (int i = 0; i < sizeof(RIFF); i++){ printf("%c", RIFF[i]); }
  printf("\n");

  // UNSIGNED LONG IN 1991 IS 4 BYTES, IN 2025 ITS 8 BYTES!!!
  // just use regular int haha
  // Why is it 36? 36 what? bytes? makes no sense
  // ITS 36 BECAUSE OF THE REST OF THE HEADER!!!!
  // see header here: https://en.wikipedia.org/wiki/WAV
  int chunkSize;
  readBytes = fread(&chunkSize, sizeof(int), 1, file);
  printf("RIFF chunk size: %u (minus 4 bytes for RIFF & 4 for chunk size)\n", chunkSize);

  char WAVE[4];
  readBytes = fread(&WAVE, sizeof(char), sizeof(WAVE), file);
  for (int i = 0; i < sizeof(WAVE); i++){ printf("%c", WAVE[i]); }
  printf("\n");

  char fmt[4];
  readBytes = fread(&fmt, sizeof(char), sizeof(fmt), file);
  for (int i = 0; i < sizeof(fmt); i++){ printf("%c", fmt[i]); }
  printf("\n");

  readBytes = fread(&chunkSize, sizeof(int), 1, file);
  printf("WAV Chunk size %u\n", chunkSize);

  short AudioFormat;
  readBytes = fread(&AudioFormat, sizeof(short), 1, file);
  printf("Audio Format %d\n", AudioFormat);

  short NumChannels;
  readBytes = fread(&NumChannels, sizeof(short), 1, file);
  printf("NumChannels %d\n", NumChannels);

  int Frequency;
  readBytes = fread(&Frequency, sizeof(Frequency), 1, file);
  printf("Frequency %d\n", Frequency);

  int BytePerSec;
  readBytes = fread(&BytePerSec, sizeof(BytePerSec), 1, file);
  printf("BytePerSec %d (Freq * BytePerBlock)\n", BytePerSec);

  short BytePerBloc;
  readBytes = fread(&BytePerBloc, sizeof(short), 1, file);
  printf("BytePerBloc %d (NumChannels * BitsPerSample / 8)\n", BytePerBloc);

  short BitsPerSample;
  readBytes = fread(&BitsPerSample, sizeof(short), 1, file);
  printf("BitsPerSample %d (NumChannels * BitsPerSample / 8)\n", BitsPerSample);

  char data[4];
  readBytes = fread(&data, sizeof(char), 4, file);
  for (int i = 0; i < sizeof(data); i++){ printf("%c", data[i]); }
  printf("\n");

  unsigned int DataSize;
  readBytes = fread(&DataSize, sizeof(DataSize), 1, file);
  printf("DataSize %u %lu\n", DataSize, readBytes);

  return 0;
}
