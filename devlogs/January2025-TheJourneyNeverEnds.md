# January 14 2026 - Wednesday  2:14 AM
Just spend a while working on using EEZ Studio to configure an LVGL interface.
It's very easy to use and while a little janky to interface with the Arduino
IDE, it's not a big deal.

While I got LVGL up and running quickly as I have experience with it now, every
time I loaded my EEZ Studio files I had no errors, just a blank screen.

I did this:

1) Create project in EEZ Studio
2) Set display height and width to match project H/W
3) Set some test labels and buttons
4) Saved the project
5) Copied all EEZ files to ~/Arduino/libraries/ui/
6) Changed all "lvgl/lvgl.h" to "lvgl.h" (I also modified my .eez-project to
automatically do this.
7) Loaded a working LVGL hello world sketch
8) To #7, included "ui.h", called ui_init() in setup and ui_tick() in loop().
9) Uploaded and got a blank screen. What's wrong?

I got it! I wasn't calling lv_tick_inc() anywhere!
I copied some more timing code that uses esp32 timers from my other lvgl
testing script and it works now!

# January 13 2026 - Tuesday  7:38 PM
After waiting for my 3D printed case to come in, I assembled my watch into it's
case and wore it to work for the first time, taped together as I am still
waiting for my M1.2 aliexpress screws to hold the thing together.

I used the watch throughout the day to tell my (around 15 minutes) skewed time,
as I haven't yet programmed the watch to update it's system time from the RTC. 

I think I am happy to call this project mostly done now, as I don't see myself
redesigning the case or hardware any time soon. If the screws don't work I will
probably glue the case together. At this point, it's just software work to make
it all come together properly. And even that, I am almost done (from
a time-telling watch's functionality point of view).

I titled this month's devlog as the journey never ends because the whole point
of this watch project is to have a development platform to use the ESP32's
(mostly) entire peripheral suite in a useable tool. I say mostly because the
only thing I don't use is the ADC, which I have not heard good things about on
the ESP32 anyways.
