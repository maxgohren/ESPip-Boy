# July 16 2025 - Wednesday 7:16 PM
RAHH! Started the PCB Design!

Working on [Robert Feranac's EasyEDA ESP32 Dev Board
tutorial](https://www.bing.com/videos/riverview/relatedvideo?q=robert+feranec+esp32+tutorital&&mid=9FBE4342DA6A0779318D9FBE4342DA6A0779318D&FORM=VAMGZC).
It's a daunting 3 hour video, but it is a necessary watch for me. I have never
done a PCB design **EVER**, and this will be a great experience for me. It
seems simple enough to take smaller projects like a development board and
tacking on my modifications for my smart watch: sensors, LED flashlights,
displays!

So far I got through 25 minutes of laying out components and starting to label
the ESP32 module. I have learned a lot about choosing the components
themselves, and started understanding enough to ask how/why we choose
components. Why certain sizes? What limitations do we have as PCB designers? 

I do like EasyEDA as it is connected to JLCPCB's inventory so you can see price
and inventory right when you choose components, and with just one click you can
see how one seemingly identical capacitor can be 10 times as expensive as it's
counterpart. 

This PCB design is the scariest, most daunting part of this project. So much to
learn, so much time to wait and money to be spent on developing, making
mistakes, fixing them and trying again...

Despite all that, I am so happy to have finally started the process!

# July 16 2025 - Wednesday 5:08 PM
Two hours later... got the TFT Display working!

I had a lot of difficulty with this, simply trying different wiring
configurations over and over and different code. I tried the Adafruit library
and the TFT_eSPI library.

Turns out you need to understand the libraries you work with! The TFT_eSPI
library requires the user to fill out User_Setup.h to provide the pins they
used for the display. Once I filled this out, I can now use any example in the
library and it works great!

Also, there is a helpful diagnostic tool that reads the user setup out to you
while writing the whole display red. Here is the pin config that worked for me.
I referred to the previously linked ESP32 datasheet to use proper SPI pins for
CLK and MOSI, but the file already had suitable pins anyways.

[code]
TFT_eSPI ver = 2.5.43
Processor    = ESP32
Frequency    = 240MHz
Transactions = Yes
Interface    = SPI
Display driver = 7789
Display width  = 240
Display height = 280

R0 y offset = 20
MOSI    = GPIO 23 // marked SDA on board
MISO    = GPIO 19 // my board does not have one of these
SCK     = GPIO 18 // marked SCL on board
TFT_CS   = GPIO 15 
TFT_DC   = GPIO 2
                  // maybe not necessary in this library but I have 
                  // RES pin -> pin D4

Font GLCD   loaded
Font 2      loaded
Font 4      loaded
Font 6      loaded
Font 7      loaded
Font 8      loaded
Smooth font enabled

Display SPI frequency = 27.00
[/code]

The display runs great! Animations and frame rate are very
smooth, I am very happy with today's work. 

I forgot to mention that I solved July 14th's problem of not having a big
enough breadboard that by cutting mine in half... with an exacto knife haha.
Took me 20 mins but well worth it, I can now properly use all of the pins with
the breadboard instead of having them unavailable due to the width of the
board.

### Today's Lessons
* Do not use the Dupont connectors I have lying around (I did not crimp them
  properly and they are unreliable - remove them and just use the solid core
wire)
* RTFM (always)
* Display runs fine on 3.3V
* BLK pin can be tied to 3.3V for constant max backlight brightness

# July 14 2025 - Monday 11:35 PM

Attempted to get the TFT display working, but no luck. Only a small lighting of
the display backlight upon grazing a random pin with a jumper cable.

My issue I learned is my breadboard is too small for my curent esp32 dev module,
which is wide enough that only 1 strip of holes is available on one side.
Documentation is also a little scarce, but there are enough YouTube videos
that I am confident I can get it to work. 

I will acquire a larger breadboard so that I can be confident in my
connections instead of holding 3 jumpers against 3 different pins!!!

[TFT Hardware
Setup](https://www.youtube.com/watch?v=3rDjwMwcFRs&ab_channel=DoIt.20)


# July 13 2025 - Friday 04:21 PM
GPS testing! [BE-180 GNSS Module](https://www.beitian.com/en/sys-pd/522.html)
* GPS TX -> ESP32 RX haha, not TX to TX!
* Don't use SoftwareSerial, use HardwareSerial as ESP32 has multiple
* GPS needs to fix, even if PPS/TX are blinking, still may not have a fix: 2-5
  mins outside
Hardware was super easy to setup, the module includes some cables that all
I needed to do was strip and slip into the breadboard. I've never worked with
serial communication though, so I made the mistake of connecting TX to TX haha.

Software was simple as well, I just changed the pins to 16 as this is the
UART2RX (Second Hardware Serial module on the ESP32). Adapted the 
[Arduino Get Started GPS
code](https://arduinogetstarted.com/tutorials/arduino-gps).
I was getting garbage, so ChatGPT helped me with changed the baud rate to see
which one would be correct, and it turns out 384800 works for me. I thought it
would be selectable through the code but perhaps the machine is hardcoded for
a speed until otherwise. 
* Installed TinyGPSPlus by Mikal Hart. 

After about an hour and a half of tinkering and vibe coding, I was able to
translate the coords given from the BE-180 to my live location at 
[GPS Coords and Map](https://www.gps-coordinates.net/). 
This concludes the hardware validation for my GPS App. 
The next step will be downloading map data and then rendering it with
a position marker. But let's not get ahead of ourselves 😈.


Another one bites the DUST!!!

# July 11 2025 - Friday 06:40 PM

Got the LSM303 (GY-511 package) working!  

Compared to my real reference compass, it seems off but I will call this a
calibration/heading issue.  The setup required calibrating the device by moving
it in all 3 axes while running the Calibration example found at [LSM303
Library](https://github.com/pololu/lsm303-arduino/blob/master/examples/Heading/Heading.ino)

Then I ran the Heading example and saw that the reading was not very
accurate, but I am okay with that for now. First sensor interfaced
successfully! [LSM303 Wiring Diagram](https://www.pololu.com/product/2124)

In my shipment I also got a new ESP32 dev board I ran this sensor on. It has a
slightly different pinout from what I am used to with my other boards - a
helpful pinout description was found here: [New Dev Board
Pinout](https://lastminuteengineers.com/esp32-pinout-reference/)

# July 11 2025 - 3:30 p.m.
Today I received my parts from aliexpress. Well, most of them. I am eager to get
to work on them now, but for now I will set up this repo along with some devlogs
to keep track of my design progress.

Yesterday, I finished the most important proof of concept. I got high quality
320 kbps Mp3 files (A Morgan Wallen song) playing over bluetooth A2DP with Phil
Schatzmann's library. It's a work of art it's so simple. Now I am confident
every other piece will go smoothly as it all seems to be very common in the
maker world. New to me yes, but not exactly reinventing the wheel. Here's to the
excitement of building your own tools!!!

