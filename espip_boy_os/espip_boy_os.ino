#include "Wire.h"
#include <Arduino.h>
#include <ESP_Knob.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <JPEGDecoder.h>

#include "AudioTools.h"
#include "AudioTools/AudioLibs/A2DPStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
//#include "AudioTools/AudioLibs/AudioBoardStream.h" // for SPI pins

File file;
MP3DecoderHelix mp3;  // or change to MP3DecoderMAD
EncodedAudioStream decoder(&file, &mp3);
BluetoothA2DPSource a2dp_source;

// callback used by A2DP to provide the sound data - usually len is 128 2 channel int16 frames
int32_t get_sound_data(uint8_t* data, int32_t size) {
  int32_t result = decoder.readBytes((uint8_t*)data, size);
  delay(1); // feed the dog
  return result;
}


#include "pitches.h"
// notes in the melody:
int melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};
// note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo:
int noteDurations[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};

#define SDA 13
#define SCL 27
#define GPIO_NUM_KNOB_PIN_A     35
#define GPIO_NUM_KNOB_PIN_B     34
#define GPIO_NUM_KNOB_SWITCH    32
#define BLACK 0x0000
#define WHITE 0xFFFF
#define BUZZER_PIN 21

const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int16_t h;
int16_t w;
int inc = -2;
float xx, xy, xz;
float yx, yy, yz;
float zx, zy, zz;
float fact;
int Xan, Yan;
int Xoff;
int Yoff;
int Zoff;
struct Point3d
{
  int x;
  int y;
  int z;
};
struct Point2d
{
  int x;
  int y;
};
int LinestoRender; // lines to render.
int OldLinestoRender; // lines to render just in case it changes. this makes sure the old lines all get erased.
struct Line3d
{
  Point3d p0;
  Point3d p1;
};
struct Line2d
{
  Point2d p0;
  Point2d p1;
};
Line3d Lines[20];
Line2d Render[20];
Line2d ORender[20];


TFT_eSPI tft = TFT_eSPI();
ESP_Knob *knob;
int buttonState = HIGH;
enum app {
  CLOCK = 0,
  CUBE,
  BUZZER,
  MENU,
  APP_MAX
};
#define MAX_MENU APP_MAX
int menuState = 0;
int appState = MENU;


void onKnobLeftEventCallback(int count, void *usr_data)
{
    if(appState == MENU){
      if(count < 0 || menuState < 0)
        menuState = 0;
      else
        menuState = count % MAX_MENU;
    }
    Serial.printf("Detect left event, count is %d, menuState %d\n", count, menuState);
}

void onKnobRightEventCallback(int count, void *usr_data)
{
    if(appState == MENU){
      if(count >= MAX_MENU || menuState >= MAX_MENU)
        menuState = MAX_MENU;
      else
        menuState = count % MAX_MENU; //need a way to control count, really hard to use this library
    }
    Serial.printf("Detect right event, count is %d menuState %d\n", count, menuState);
}

void onKnobHighLimitEventCallback(int count, void *usr_data)
{
    Serial.printf("Detect high limit event, count is %d menuState %d\n", count, menuState);
}

void onKnobLowLimitEventCallback(int count, void *usr_data)
{
    Serial.printf("Detect low limit event, count is %d menuState %d\n", count, menuState);
}

void onKnobZeroEventCallback(int count, void *usr_data)
{
    Serial.printf("Detect zero event, count is %d menuState %d\n", count, menuState);
}

void initKnob(){
  knob = new ESP_Knob(GPIO_NUM_KNOB_PIN_A, GPIO_NUM_KNOB_PIN_B);
  // knob->invertDirection();
    knob->begin();
    knob->attachLeftEventCallback(onKnobLeftEventCallback);
    knob->attachRightEventCallback(onKnobRightEventCallback);
    knob->attachHighLimitEventCallback(onKnobHighLimitEventCallback);
    knob->attachLowLimitEventCallback(onKnobLowLimitEventCallback);
    knob->attachZeroEventCallback(onKnobZeroEventCallback);

    pinMode(GPIO_NUM_KNOB_SWITCH, INPUT_PULLUP);
}

void initSDCard(){
   // Set all chip selects high to avoid bus contention during initialisation of each peripheral
  digitalWrite(15, HIGH); // TFT screen chip select
  digitalWrite( 5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

  if (!SD.begin(5, tft.getSPIinstance())) {
    Serial.println("Card Mount Failed");
    tft.println("Card Mount Failed");
    return;
  }
  uint8 cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("NO SD Card Attached");
    tft.println("No SD Card Attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println("initialisation done.");



  //testSDCard();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  initKnob();

  
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 20, 4);
  tft.setTextColor(TFT_WHITE);
  tft.printf(" Height: %d\n", tft.height());
  tft.printf(" Width: %d\n", tft.width());
  tft.setTextWrap( true, true );
  delay(2000);

  initSDCard();

  // Music AFTER SD card because we need to init SD first
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
  file = SD.open("/gavin_adcock_loose_strings.mp3", FILE_READ);
  if (!file) {
    Serial.println("file failed");
    stop();
  }
   // make sure we have enough space for the pcm data
  decoder.transformationReader().resizeResultQueue(1024 * 8);
  if (!decoder.begin()) {
    Serial.println("decoder failed");
    stop();
  }
  // start the bluetooth
  Serial.println("starting A2DP...");
  a2dp_source.set_data_callback(get_sound_data);
  a2dp_source.start("H1");
  //a2dp_source.start("Max’s AirPods");
  //a2dp_source.start("esp32");
  

  tft.init();
  tft.fillScreen(TFT_BLACK);

  initCube();
}

void initCube(){
  h = tft.height();
  w = tft.width();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  cube();
  fact = 180 / 3.14159259; // conversion from degrees to radians.
  Xoff = 120; // Position the centre of the 3d conversion space into the centre of the TFT screen.
  Yoff = 140;
  Zoff = 550; // Z offset in 3D space (smaller = closer and bigger rendering)
  // Initialize MPU
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}


void loop() {
  //i2c_scan();
  check_rotary_knob_state();
  updateApp();
 
}

void updateApp(){
  switch (appState){
    default:
    case MENU:
      drawMenu();
      break;
    case CLOCK:
      drawClock();
      break;
    case CUBE:
     drawCube();
     break;
    case BUZZER:
     drawBuzzer();
     break;
  }
}

void drawMenu(){
  const int interval = 300; // redraw 3 times every second
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 20, 4);
    menuState == CLOCK ? tft.setTextColor(TFT_RED) :  tft.setTextColor(TFT_WHITE);
    tft.println("CLOCK");
    menuState == CUBE ? tft.setTextColor(TFT_RED) :  tft.setTextColor(TFT_WHITE);
    tft.println("CUBE");
    menuState == BUZZER ? tft.setTextColor(TFT_RED) :  tft.setTextColor(TFT_WHITE);
    tft.println("BUZZER");
  }
}

void drawBuzzer(){
  // iterate over the notes of the melody:
  int size = sizeof(noteDurations) / sizeof(int);

  for (int thisNote = 0; thisNote < size; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_PIN);
  }
}

void drawClock(){
  const int interval = 300; // redraw 3 times every second
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;

    tft.fillScreen(TFT_BLACK);
    tft.fillCircle(tft.width() / 2, tft.height() / 2, 50, TFT_GREEN);
  }
}

void drawCube(){
  cubeLoop();
}



void check_rotary_knob_state(){
  static int lastButtonState = HIGH;
  buttonState = digitalRead(GPIO_NUM_KNOB_SWITCH);
  if (buttonState != lastButtonState)
  {
    if (buttonState == LOW){
      Serial.println("button down");
    }
    else
    {
      if (appState != MENU)
        appState = MENU;
      else
        appState = menuState;
      Serial.printf("button up, menuState: %d\n", menuState);

    }
    delay(50);
  }
  lastButtonState = buttonState;
}



void i2c_scan(){
  byte error, address;
  int nDevices = 0;
  const int interval = 5000; // scan every 5 seconds
  static unsigned long previousMillis = 0;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){
    Serial.println("Scanning for I2C devices ...");
    for(address = 0x01; address < 0x7f; address++){
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0){
        Serial.printf("I2C found @ addr: 0x%02X\n", address);
        //tft.printf("I2C found @ addr: 0x%02X\n", address);
        nDevices++;
      } else if(error != 2){
        Serial.printf("Error %d at address 0x%02X\n", error, address);
      }
    }
    if (nDevices == 0){
      Serial.println("No I2C devices found");
    }
    previousMillis = currentMillis;
  }
}



void testSDCard(){
  tft.setRotation(2);  // portrait
  tft.fillScreen(random(0xFFFF));

  // The image is 300 x 300 pixels so we do some sums to position image in the middle of the screen!
  // Doing this by reading the image width and height from the jpeg info is left as an exercise!
  int x = (tft.width()  - 300) / 2 - 1;
  int y = (tft.height() - 300) / 2 - 1;

  drawSdJpeg("/EagleEye.jpg", x, y);     // This draws a jpeg pulled off the SD Card
  delay(2000);

  tft.setRotation(2);  // portrait
  tft.fillScreen(random(0xFFFF));
  drawSdJpeg("/Baboon40.jpg", 0, 0);     // This draws a jpeg pulled off the SD Card
  delay(2000);

  tft.setRotation(2);  // portrait
  tft.fillScreen(random(0xFFFF));
  drawSdJpeg("/lena20k.jpg", 0, 0);     // This draws a jpeg pulled off the SD Card
  delay(2000);

  tft.setRotation(1);  // landscape
  tft.fillScreen(random(0xFFFF));
  drawSdJpeg("/Mouse480.jpg", 0, 0);     // This draws a jpeg pulled off the SD Card

  delay(5000);
}

//####################################################################################################
// Draw a JPEG on the TFT pulled from SD Card
//####################################################################################################
// xpos, ypos is top left corner of plotted image
void drawSdJpeg(const char *filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library

  if ( !jpegFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }

  Serial.println("===========================");
  Serial.print("Drawing file: "); Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
  //bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded) {
    // print information about the image to the serial port
    jpegInfo();
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos) {

  //jpegInfo(); // Print information from the JPEG file (could comment this line out)

  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read()) {    // While there is more data in the file
    pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ( (mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  tft.setSwapBytes(swapBytes);

  showTime(millis() - drawTime); // These lines are for sketch testing only
}

//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
// JpegDec.decodeFile(...) or JpegDec.decodeArray(...) must be called before this info is available!
void jpegInfo() {

  // Print information extracted from the JPEG file
  Serial.println("JPEG image info");
  Serial.println("===============");
  Serial.print("Width      :");
  Serial.println(JpegDec.width);
  Serial.print("Height     :");
  Serial.println(JpegDec.height);
  Serial.print("Components :");
  Serial.println(JpegDec.comps);
  Serial.print("MCU / row  :");
  Serial.println(JpegDec.MCUSPerRow);
  Serial.print("MCU / col  :");
  Serial.println(JpegDec.MCUSPerCol);
  Serial.print("Scan type  :");
  Serial.println(JpegDec.scanType);
  Serial.print("MCU width  :");
  Serial.println(JpegDec.MCUWidth);
  Serial.print("MCU height :");
  Serial.println(JpegDec.MCUHeight);
  Serial.println("===============");
  Serial.println("");
}

//####################################################################################################
// Show the execution time (optional)
//####################################################################################################
// WARNING: for UNO/AVR legacy reasons printing text to the screen with the Mega might not work for
// sketch sizes greater than ~70KBytes because 16-bit address pointers are used in some libraries.

// The Due will work fine with the HX8357_Due library.

void showTime(uint32_t msTime) {
  //tft.setCursor(0, 0);
  //tft.setTextFont(1);
  //tft.setTextSize(2);
  //tft.setTextColor(TFT_WHITE, TFT_BLACK);
  //tft.print(F(" JPEG drawn in "));
  //tft.print(msTime);
  //tft.println(F(" ms "));
  Serial.print(F(" JPEG drawn in "));
  Serial.print(msTime);
  Serial.println(F(" ms "));
}

void cubeLoop(){
  //For cube rotation
  int xOut = 0;
  int yOut = 0;
  // Rotate around x and y axes in 1 degree increments
  xOut = map(AcX, -17000, 17000, -50, 50);
  yOut = map(AcY, -17000, 17000, -50, 50);
  Xan = xOut;
  Yan = yOut;
  Yan = Yan % 360;
  Xan = Xan % 360; // prevents overflow.
  SetVars(); //sets up the global vars to do the 3D conversion.
  Zoff = 240;
    // Zoom in and out on Z axis within limits
    // the cube intersects with the screen for values < 160
    Zoff += inc;
    if (Zoff > 500) inc = -1;     // Switch to zoom in
    else if (Zoff < 160) inc = 1; // Switch to zoom out
  for (int i = 0; i < LinestoRender ; i++)
  {
    ORender[i] = Render[i]; // stores the old line segment so we can delete it later.
    ProcessLine(&Render[i], Lines[i]); // converts the 3d line segments to 2d.
  }
  RenderImage(); // go draw it!
  delay(14); // Delay to reduce loop rate (reduces flicker caused by aliasing with TFT screen refresh rate)
}
/***********************************************************************************************************************************/
void RenderImage( void)
{
  // renders all the lines after erasing the old ones.
  // in here is the only code actually interfacing with the OLED. so if you use a different lib, this is where to change it.
  for (int i = 0; i < OldLinestoRender; i++ )
  {
    tft.drawLine(ORender[i].p0.x, ORender[i].p0.y, ORender[i].p1.x, ORender[i].p1.y, BLACK); // erase the old lines.
  }
  for (int i = 0; i < LinestoRender; i++ )
  {
    uint16_t color = TFT_BLUE;
    if (i < 4) color = TFT_RED;
    if (i > 7) color = TFT_GREEN;
    tft.drawLine(Render[i].p0.x, Render[i].p0.y, Render[i].p1.x, Render[i].p1.y, color);
  }
  OldLinestoRender = LinestoRender;
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(true);
  Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}
/***********************************************************************************************************************************/
// Sets the global vars for the 3d transform. Any points sent through "process" will be transformed using these figures.
// only needs to be called if Xan or Yan are changed.
void SetVars(void)
{
  float Xan2, Yan2, Zan2;
  float s1, s2, s3, c1, c2, c3;
  Xan2 = Xan / fact; // convert degrees to radians.
  Yan2 = Yan / fact;
  // Zan is assumed to be zero
  s1 = sin(Yan2);
  s2 = sin(Xan2);
  c1 = cos(Yan2);
  c2 = cos(Xan2);
  xx = c1;
  xy = 0;
  xz = -s1;
  yx = (s1 * s2);
  yy = c2;
  yz = (c1 * s2);
  zx = (s1 * c2);
  zy = -s2;
  zz = (c1 * c2);
}
/***********************************************************************************************************************************/
// processes x1,y1,z1 and returns rx1,ry1 transformed by the variables set in SetVars()
// fairly heavy on floating point here.
// uses a bunch of global vars. Could be rewritten with a struct but not worth the effort.
void ProcessLine(struct Line2d *ret, struct Line3d vec)
{
  float zvt1;
  int xv1, yv1, zv1;
  float zvt2;
  int xv2, yv2, zv2;
  int rx1, ry1;
  int rx2, ry2;
  int x1;
  int y1;
  int z1;
  int x2;
  int y2;
  int z2;
  int Ok;
  x1 = vec.p0.x;
  y1 = vec.p0.y;
  z1 = vec.p0.z;
  x2 = vec.p1.x;
  y2 = vec.p1.y;
  z2 = vec.p1.z;
  Ok = 0; // defaults to not OK
  xv1 = (x1 * xx) + (y1 * xy) + (z1 * xz);
  yv1 = (x1 * yx) + (y1 * yy) + (z1 * yz);
  zv1 = (x1 * zx) + (y1 * zy) + (z1 * zz);
  zvt1 = zv1 - Zoff;
  if ( zvt1 < -5) {
    rx1 = 256 * (xv1 / zvt1) + Xoff;
    ry1 = 256 * (yv1 / zvt1) + Yoff;
    Ok = 1; // ok we are alright for point 1.
  }
  xv2 = (x2 * xx) + (y2 * xy) + (z2 * xz);
  yv2 = (x2 * yx) + (y2 * yy) + (z2 * yz);
  zv2 = (x2 * zx) + (y2 * zy) + (z2 * zz);
  zvt2 = zv2 - Zoff;
  if ( zvt2 < -5) {
    rx2 = 256 * (xv2 / zvt2) + Xoff;
    ry2 = 256 * (yv2 / zvt2) + Yoff;
  } else
  {
    Ok = 0;
  }
  if (Ok == 1) {
    ret->p0.x = rx1;
    ret->p0.y = ry1;
    ret->p1.x = rx2;
    ret->p1.y = ry2;
  }
  // The ifs here are checks for out of bounds. needs a bit more code here to "safe" lines that will be way out of whack, so they don't get drawn and cause screen garbage.
}
// line segments to draw a cube. basically p0 to p1. p1 to p2. p2 to p3 so on.
void cube(void)
{
  // Front Face.
  Lines[0].p0.x = -50;
  Lines[0].p0.y = -50;
  Lines[0].p0.z = 50;
  Lines[0].p1.x = 50;
  Lines[0].p1.y = -50;
  Lines[0].p1.z = 50;
  Lines[1].p0.x = 50;
  Lines[1].p0.y = -50;
  Lines[1].p0.z = 50;
  Lines[1].p1.x = 50;
  Lines[1].p1.y = 50;
  Lines[1].p1.z = 50;
  Lines[2].p0.x = 50;
  Lines[2].p0.y = 50;
  Lines[2].p0.z = 50;
  Lines[2].p1.x = -50;
  Lines[2].p1.y = 50;
  Lines[2].p1.z = 50;
  Lines[3].p0.x = -50;
  Lines[3].p0.y = 50;
  Lines[3].p0.z = 50;
  Lines[3].p1.x = -50;
  Lines[3].p1.y = -50;
  Lines[3].p1.z = 50;
  //back face.
  Lines[4].p0.x = -50;
  Lines[4].p0.y = -50;
  Lines[4].p0.z = -50;
  Lines[4].p1.x = 50;
  Lines[4].p1.y = -50;
  Lines[4].p1.z = -50;
  Lines[5].p0.x = 50;
  Lines[5].p0.y = -50;
  Lines[5].p0.z = -50;
  Lines[5].p1.x = 50;
  Lines[5].p1.y = 50;
  Lines[5].p1.z = -50;
  Lines[6].p0.x = 50;
  Lines[6].p0.y = 50;
  Lines[6].p0.z = -50;
  Lines[6].p1.x = -50;
  Lines[6].p1.y = 50;
  Lines[6].p1.z = -50;
  Lines[7].p0.x = -50;
  Lines[7].p0.y = 50;
  Lines[7].p0.z = -50;
  Lines[7].p1.x = -50;
  Lines[7].p1.y = -50;
  Lines[7].p1.z = -50;
  // now the 4 edge lines.
  Lines[8].p0.x = -50;
  Lines[8].p0.y = -50;
  Lines[8].p0.z = 50;
  Lines[8].p1.x = -50;
  Lines[8].p1.y = -50;
  Lines[8].p1.z = -50;
  Lines[9].p0.x = 50;
  Lines[9].p0.y = -50;
  Lines[9].p0.z = 50;
  Lines[9].p1.x = 50;
  Lines[9].p1.y = -50;
  Lines[9].p1.z = -50;
  Lines[10].p0.x = -50;
  Lines[10].p0.y = 50;
  Lines[10].p0.z = 50;
  Lines[10].p1.x = -50;
  Lines[10].p1.y = 50;
  Lines[10].p1.z = -50;
  Lines[11].p0.x = 50;
  Lines[11].p0.y = 50;
  Lines[11].p0.z = 50;
  Lines[11].p1.x = 50;
  Lines[11].p1.y = 50;
  Lines[11].p1.z = -50;
  LinestoRender = 12;
  OldLinestoRender = LinestoRender;
}
