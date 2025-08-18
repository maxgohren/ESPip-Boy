/*
  ESP32 SD Card — Thorough Mount/Diagnostics/Benchmark
  - Uses SPI mode (SD library)
  - Verbose logs for each step
  - Falls back across several SPI clock speeds
  - File create/read/append/rename/delete + recursive list
  - Binary R/W benchmark with integrity check

  ===== Quick Setup =====
  1) Wire SD module (SPI mode):
     CS=GPIO5, SCK=18, MISO=19, MOSI=23, VCC=3.3V, GND=GND
  2) Format card as FAT32 (not exFAT).
  3) Set SD_CS below if you used a different CS pin.
  4) Upload and open Serial Monitor @ 115200.

  Notes:
  - If mount fails, sketch will tell you what to try next.
  - You can switch to HSPI by setting USE_HSPI=1 and wiring SCK/MISO/MOSI accordingly
    (typical HSPI defaults: SCK=14, MISO=12, MOSI=13 — beware GPIO12 is a strapping pin).
*/

#include <FS.h>
#include <SD.h>
#include <SPI.h>

// ===================== USER CONFIG =====================
#define SD_CS          5      // Your SD chip-select pin
#define USE_HSPI       0      // 0 = VSPI, 1 = HSPI (requires different wiring)
#define FORCE_PINS     1      // 1 = explicitly set SCK/MISO/MOSI pins below
// VSPI defaults (common on dev boards)
#if USE_HSPI
  #define SD_SCK  14
  #define SD_MISO 12  // ⚠ strapping pin; avoid if possible
  #define SD_MOSI 13
#else
  #define SD_SCK  18
  #define SD_MISO 19
  #define SD_MOSI 23
#endif

// Try these SPI clock speeds (Hz) in order until mount succeeds
uint32_t TRY_SPEEDS[] = { 20000000UL, 10000000UL, 4000000UL, 1000000UL };

// Benchmark size (in MiB) and chunk size (bytes)
#define BENCH_MIB      8      // total size written/read for benchmark (8 MiB)
#define CHUNK_BYTES    4096   // buffer size per write/read
// =======================================================

SPIClass *sdSPI = nullptr;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
bool writeText(fs::FS &fs, const char * path, const String &content);
bool readText(fs::FS &fs, const char * path, String &out);
bool appendText(fs::FS &fs, const char * path, const String &content);
bool renamePath(fs::FS &fs, const char * from, const char * to);
bool removePath(fs::FS &fs, const char * path);
bool benchRW(fs::FS &fs, const char * path, size_t mib, size_t chunk);

void printCardInfo();
const char* cardTypeName(uint8_t t);

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("==== ESP32 SD Thorough Test ===="));

#if USE_HSPI
  Serial.println(F("[BUS] Using HSPI"));
  sdSPI = new SPIClass(HSPI);
#else
  Serial.println(F("[BUS] Using VSPI"));
  sdSPI = new SPIClass(VSPI);
#endif

#if FORCE_PINS
  Serial.printf("[PINS] SCK=%d  MISO=%d  MOSI=%d  CS=%d\n", SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  sdSPI->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
#else
  Serial.printf("[PINS] Using core defaults; CS=%d\n", SD_CS);
  sdSPI->begin(); // use board defaults
#endif

  // Try to mount at multiple speeds
  bool mounted = false;
  uint32_t usedSpeed = 0;

  for (uint32_t f : TRY_SPEEDS) {
    Serial.printf("[MOUNT] Trying SD.begin(CS=%d, freq=%lu Hz)... ", SD_CS, f);
    mounted = SD.begin(SD_CS, *sdSPI, f);
    Serial.println(mounted ? "OK" : "FAIL");
    if (mounted) { usedSpeed = f; break; }
  }

  if (!mounted) {
    Serial.println();
    Serial.println(F("[ERROR] Card Mount Failed."));
    Serial.println(F("        Checklist:"));
    Serial.println(F("         - Ensure FAT32 (not exFAT)"));
    Serial.println(F("         - Verify 3.3V logic & power (some cards draw >100mA bursts)"));
    Serial.println(F("         - Check CS pin wiring and uniqueness (no other device sharing same CS)"));
    Serial.println(F("         - Try another CS pin (e.g., 4, 15) and update SD_CS"));
    Serial.println(F("         - Try lower SPI speed (already attempted 20/10/4/1 MHz)"));
    Serial.println(F("         - If using a TFT/other SPI device, confirm separate CS and no pin conflicts"));
    while (true) { delay(1000); }
  }

  Serial.printf("[MOUNT] Mounted successfully at %lu Hz\n", usedSpeed);
  printCardInfo();

  // Create a test directory
  const char * testDir = "/test_sd";
  Serial.printf("[DIR] Creating dir: %s ... ", testDir);
  if (SD.mkdir(testDir)) Serial.println("OK");
  else Serial.println("EXISTS or FAIL (ignored)");

  // Simple text write/read/append
  const char * txtPath = "/test_sd/hello.txt";
  Serial.printf("[WRITE] %s\n", txtPath);
  if (!writeText(SD, txtPath, "Hello from ESP32 SD test!\nLine 2.\n")) {
    Serial.println("[WRITE] Failed. Aborting.");
    while (true) delay(1000);
  }

  String content;
  Serial.printf("[READ ] %s\n", txtPath);
  if (readText(SD, txtPath, content)) {
    Serial.println("[READ ] Content:");
    Serial.println("-------------------------");
    Serial.print(content);
    Serial.println("-------------------------");
  } else {
    Serial.println("[READ ] Failed!");
  }

  Serial.printf("[APPEND] %s\n", txtPath);
  appendText(SD, txtPath, "Appended line.\n");

  Serial.println("[LIST ] / (root, depth=2)");
  listDir(SD, "/", 2);

  // Rename and delete test
  const char * renamed = "/test_sd/renamed.txt";
  Serial.printf("[RENAME] %s -> %s : %s\n", txtPath, renamed,
                renamePath(SD, txtPath, renamed) ? "OK" : "FAIL");

  Serial.printf("[REMOVE] %s : %s\n", renamed,
                removePath(SD, renamed) ? "OK" : "FAIL");

  // Benchmark read/write with integrity check
  Serial.println("[BENCH ] Starting binary R/W benchmark with verification...");
  if (!benchRW(SD, "/bench.bin", BENCH_MIB, CHUNK_BYTES)) {
    Serial.println("[BENCH ] FAILED integrity or I/O.");
  } else {
    Serial.println("[BENCH ] Completed successfully.");
  }

  Serial.println("\n==== Done. You can reset and re-run if needed. ====");
}

void loop() {
  // Nothing here.
}

// ===================== Helpers =====================

void printCardInfo() {
  uint8_t ct = SD.cardType();
  Serial.printf("[INFO ] Card Type: %s (code=%u)\n", cardTypeName(ct), ct);

  uint64_t sz = SD.cardSize();
  double gb = (double)sz / (1024.0 * 1024.0 * 1024.0);
  double mb = (double)sz / (1024.0 * 1024.0);

  Serial.printf("[INFO ] Card Size: %.2f GB (%.0f MB)  == %llu bytes\n", gb, mb, (unsigned long long)sz);

  if (ct == CARD_NONE) {
    Serial.println("[HINT ] CARD_NONE indicates wiring/power/CS issue or unsupported mode.");
  }
  if (sz == 0) {
    Serial.println("[HINT ] 0 bytes often means mount succeeded but card failed to identify capacity.");
  }
}

const char* cardTypeName(uint8_t t) {
  switch (t) {
    case CARD_MMC:  return "MMC";
    case CARD_SD:   return "SDSC (<=2GB)";
    case CARD_SDHC: return "SDHC/SDXC (>=4GB)";
    case CARD_UNKNOWN: return "UNKNOWN";
    case CARD_NONE: return "NONE";
    default:        return "??";
  }
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("[LIST ] Dir: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("[LIST ] Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("[LIST ] Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.printf("  <DIR> %s\n", file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.printf("  %8llu  %s\n", (unsigned long long)file.size(), file.name());
    }
    file = root.openNextFile();
  }
}

bool writeText(fs::FS &fs, const char * path, const String &content) {
  File f = fs.open(path, FILE_WRITE);
  if (!f) {
    Serial.println("[WRITE] open failed");
    return false;
  }
  size_t w = f.print(content);
  f.flush();
  f.close();
  Serial.printf("[WRITE] wrote %u bytes\n", (unsigned)w);
  return w == content.length();
}

bool readText(fs::FS &fs, const char * path, String &out) {
  File f = fs.open(path, FILE_READ);
  if (!f) {
    Serial.println("[READ ] open failed");
    return false;
  }
  out.reserve(f.size() + 16);
  while (f.available()) {
    out += (char)f.read();
  }
  f.close();
  return true;
}

bool appendText(fs::FS &fs, const char * path, const String &content) {
  File f = fs.open(path, FILE_APPEND);
  if (!f) {
    Serial.println("[APPEND] open failed");
    return false;
  }
  size_t w = f.print(content);
  f.flush();
  f.close();
  Serial.printf("[APPEND] appended %u bytes\n", (unsigned)w);
  return w == content.length();
}

bool renamePath(fs::FS &fs, const char * from, const char * to) {
  if (!fs.exists(from)) {
    Serial.println("[RENAME] source not found");
    return false;
  }
  if (fs.exists(to)) {
    Serial.println("[RENAME] target exists (removing it)");
    fs.remove(to);
  }
  bool ok = fs.rename(from, to);
  return ok;
}

bool removePath(fs::FS &fs, const char * path) {
  if (!fs.exists(path)) {
    Serial.println("[REMOVE] path not found (OK)");
    return true;
  }
  return fs.remove(path);
}

bool benchRW(fs::FS &fs, const char * path, size_t mib, size_t chunk) {
  const size_t totalBytes = mib * 1024UL * 1024UL;

  // --- WRITE ---
  uint8_t *buf = (uint8_t*)malloc(chunk);
  if (!buf) {
    Serial.println("[BENCH ] malloc failed");
    return false;
  }

  // Deterministic pattern for integrity
  uint32_t seq = 0;
  auto fillBuf = [&](uint8_t *b, size_t n){
    for (size_t i=0; i<n; ++i) { b[i] = (uint8_t)(seq++ & 0xFF); }
  };

  Serial.printf("[BENCH ] Writing %u MiB to %s in %u-byte chunks...\n",
                (unsigned)mib, path, (unsigned)chunk);

  File fw = fs.open(path, FILE_WRITE);
  if (!fw) {
    Serial.println("[BENCH ] write open failed");
    free(buf);
    return false;
  }

  uint32_t t0 = millis();
  size_t written = 0;
  while (written < totalBytes) {
    size_t n = (totalBytes - written) < chunk ? (totalBytes - written) : chunk;
    fillBuf(buf, n);
    size_t w = fw.write(buf, n);
    if (w != n) {
      Serial.printf("[BENCH ] write short (%u/%u)\n", (unsigned)w, (unsigned)n);
      fw.close(); free(buf); return false;
    }
    written += w;
  }
  fw.flush();
  fw.close();
  uint32_t t1 = millis();

  double secsW = (t1 - t0) / 1000.0;
  double mbpsW = (double)totalBytes / (1024.0 * 1024.0) / secsW;
  Serial.printf("[BENCH ] WRITE: %.2f MiB in %.2f s  =>  %.2f MiB/s\n", (double)mib, secsW, mbpsW);

  // --- READ & VERIFY ---
  Serial.printf("[BENCH ] Reading %u MiB back and verifying pattern...\n", (unsigned)mib);

  File fr = fs.open(path, FILE_READ);
  if (!fr) {
    Serial.println("[BENCH ] read open failed");
    free(buf);
    return false;
  }

  // Recreate same sequence from start to verify bytes
  seq = 0;
  t0 = millis();
  size_t readBytes = 0;
  while (readBytes < totalBytes) {
    size_t n = (totalBytes - readBytes) < chunk ? (totalBytes - readBytes) : chunk;
    size_t r = fr.read(buf, n);
    if (r != n) {
      Serial.printf("[BENCH ] read short (%u/%u)\n", (unsigned)r, (unsigned)n);
      fr.close(); free(buf); return false;
    }
    // verify
    for (size_t i=0; i<n; ++i) {
      uint8_t expected = (uint8_t)(seq++ & 0xFF);
      if (buf[i] != expected) {
        Serial.printf("[BENCH ] VERIFY FAIL at byte %u: got 0x%02X, expected 0x%02X\n",
                      (unsigned)(readBytes + i), buf[i], expected);
        fr.close(); free(buf); return false;
      }
    }
    readBytes += r;
  }
  fr.close();
  t1 = millis();

  double secsR = (t1 - t0) / 1000.0;
  double mbpsR = (double)totalBytes / (1024.0 * 1024.0) / secsR;
  Serial.printf("[BENCH ] READ : %.2f MiB in %.2f s  =>  %.2f MiB/s (verified OK)\n", (double)mib, secsR, mbpsR);

  // Clean up
  if (!fs.remove(path)) {
    Serial.println("[BENCH ] Warning: failed to remove bench file (manual cleanup needed).");
  } else {
    Serial.println("[BENCH ] bench file removed.");
  }

  free(buf);
  return true;
}

