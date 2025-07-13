### July 2025

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

