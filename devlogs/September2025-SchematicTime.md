# September 13 2025 - Saturday 11:44 AM
"It just works". It actually just works! LVGL with Arduino GFX + TouchLib
works! I still had to do the quirky include path editing and moving the demo
folder to make a demo work, but it works! No crazy colour bugs, no rotation
issues, no mismatch betwen coord systems between display and touch, it just
works!

I do have crashing on the button matrix, but a more complicated button matrix
with a scrolling view area works haha. I am very happy. The display is very
responsive and the LVGL grapics work just like they look on the examples. 

OMG the music player demo is sooo sick. The animations are actually pretty
smooth and the layout is perfect (aside from touchscreen bounds not being
dialed in, I can't exit a menu because the touchscreen bounds are smaller than
the drawn button bounds). 

Really satisfied with today's work and graphics setup. Really all there is to
do now, is implement a simple menu with one of the various widgets (and
hopefully one that doesn't crash) and then implement simple test apps for heart
rate, clock, etc. 

Then, when that is done, some sleep and interrupt handling on touch and motion. 

And then when our proof of concept is finished, were good to finalize the
design and properly finish the schematic, knowing the design works :).

# September 13 2025 - Saturday 12:54 AM
The new breadboard is off to an amazing start. All 6! i2C sensors enabled and
detected on i2c scan. Capacitive touch sensor working perfectly and very
responsively. Display working perfectly on test sketch. 

I am so happy it works. 20 pins later haha. I was scared to death too, because
I accidentally wired it up the wrong way after spending so long learning about
current limiting resistors and checking a million schematics to make sure
I wouldn't blow my backlight. Turns out I could smell how hot my touch
controller was, because I thought I was wiring up the display, but I was
actually wiring up the touch controller as my FPC breakout was flipped around.

I was using the silkscreen markings for the bottom side, not the side I was
actually connected to, on the 2 layer breakout board!

Even though my touch controller got super hot, I'm super happy it still works.
Here's to almost finishing a very clean prototype! Only a few more components
are in the mail, and then it's time to call it done, and start serious design
work on the PCB!

![It was cleaner before the display](../img_vid/fresh_breadboarded_display.png)

# September 11 2025 - Thursday 10:33 PM
Today I am taking a break from display software, to work on display hardware!

I got my new touchscreen TFT IPS display in, the same 1.69" I was using before
but not touchscreen. I am traumatized from my day spent with my resistive touch
2.4" and LVGL. 

I also got some new jumpers, breadboard and buck-boost power supply convertors
so my I actually ripped my whole breadboard clean and will rebuild it bigger,
better, faster, stronger in the coming days. My design is more finalized so
I figured it is time to clean it up to help in debugging and schematic design. 

I spent tonight tinkering with the new display and reading about backlight LED
current limiting resistors as I don't want to burn my only one out. If I do,
it's ok but I'd rather not wait another 2 weeks and another $12 for another
one. Looks like the reference schematics, my own model and some reading point
to resistors in the ~1-100 Ohm range, as the backlight drops 3.2V. For a 3.3V
supply, this leaves 0.1 V for the resistor. Limit it to 60 mA, gives you
a solid 1.7 Ohms, or a more conservative 40 mA for 2.5 Ohms. I'll use 10 and go
from there. 

Gonna take a break now. I have a habit of pouring so much time into this
project that it becomes stressful, with poor focus and fatigue being the
driving factors. I want more quality, less quantity. Looking forward to the new
display and the cleaner breadboard setup. Hopefully that inspires LVGL and my
touchscreen to work flawlessly together!

# September 10 2025 - Wednesday 11:47 PM
OMG LVGL is kind of a nightmare. 

I have been at it all day. Installing my touchscreen display, trying out more
examples and demos, finally comfortably using a slider widget, trying Arduino
IDE in WSL to improve horrendous build times, it's been the worst integration
into this project so far. 

The last headache (I hope it's the last) has been the touchscreen functionality
with TFT_eSPI and LVGL. I've found many threads complaing about how the
touchscreen seems to rotate completely opposite the display so it never lines up
correctly.

https://forum.lvgl.io/t/screen-rotation/19316/12
https://github.com/lvgl/lvgl/issues/7841
https://forum.lvgl.io/t/inconsistency-between-display-rotation-and-touchscreen-coordinate-rotation/20242

I've been trying solutions posted and playing with swapping coordinates around
but no success. I don't really understand what's happening under the hood, who
rotates what where, and how many times and which direction things are getting
rotated. I need to do some more experiments to see which rotations from which
libraries are required when. 

Thankfully I only need one orientation, I hope that may make this easier. But
man has this been a difficult ride.

# September 10 2025 - Wednesday 01:06 AM
Man LVGL is not simple to use. Manual install of folders to different locations
in the library. Copy the config to the libraries folder. Compile and get the
hello world running ok? Now try the demos, that in order to get it to just
barely work (it doesn't seem to respond to my choice of demo from the
		aformentioned config file) I need to manually massage a relative include
path of a header. WTF?

The one thing keeping me going right now is this demo of a smartwatch
[UI](https://lvgl.io/demos). It looks so amazing, I really hope I can get this
stable and working!

Time for bed now though, I am happy I got my demo working when this morning I
could only get the Hello World working.

# September  9 2025 - Tuesday 09:50 AM
Waiting for hardware. Stuck on power supply choices. Simple discrete components
linked together or a powerful PMIC with everything but the kitchen sink? I do
not like making schematics for components I don't have a working reference
design for right in front of me. I check JLC PCBA costs, and right out of the
gate my design, without any cost optimizations, will be $200 for two boards.
Pretty crazy.

I would then not like to experiment with new components I am not comfortable
with as this price drastically reduces the amount of hardware revisions I would
like to do. Instead, if I would like to use the cheap but ubiquitous AXP2101
with multiple bucks and LDOs, along with an integrated LiPo charger and fuel
gauge, I will order it on AliExpress along with a breakout board. This will
satisfy my need to have a working demo before I try to make a schematic
including it. 

It will be a parallel development/prototype with both a discrete setup (MAX17048
		Fuel Gauge + TP63020 buck-boost 3.3V supply + TP4056/DW01
		charger/protection) and an  integrated PMIC solution - AXP2101. Having a
working prototype will allow me to learn about the devices and have real world
measurements on how they perform in best and worst case battery life scenarios. 

No more endless document reading, get your hands dirty and see what works!

While waiting for the aformentioned hardware, I am working on LVGL integration.
It is the best solution for my graphic needs and looks amazing. I got the hello
world demo today, but man the examples/demo work is so annoying, they have this
requirement you copy the examples or demo folders into the src/ folder but it
always throws an error. It can't find the actual src, which according to the
include directives is in the place it would be if I never copied the demo folder
in the first place! So confusing, I will try again later!

# September  6 2025 - Saturday 12:36 AM
Today on my schematic, I wired up the display, the microphone and some roughed-in enable pins for
my various high power peripherals like my vibration motor, buzzer and speaker. I
did this so that I can see how many pins I have available and if my design is
going to all fit on the ESP32's GPIO selection. So far I think it will be okay,
and it will be helped if I can select a great PMIC with some I2C
selectable outputs. This would allow me to have more I2C control instead
of GPIO control which I am lacking. I really don't need the power of a
GPIO anyways for simple enable pins, but for stuff like vibration motors
or buzzers I do need a PWM output. All part of the balancing act haha.

I've asked ChatGPT a bunch of times for my perfect PMIC and it keeps
landing on the same modules, the MAX77658 and the TI BQ25190. Both are
great chips and both do what I need. I haven't yet narrowed it down, but I
would like to as it will influence the GPIO bottleneck and overall design.

I'm aiming at the following power path:
3.7V lipo -> buck boost @ 3.5V -> LDO @ 3.3V.
I'm weary of these PMICs and using the buck boost output to directly power my
ESP32 as the LDOs are more for the audio/RF paths, but I need the power spike
handling ability. I will read more about ripple and how the ESP32 handles it.

I'm really happy to have done more experimenting with the power supply and
found why I need what I need. Running the battery straight into the 3.3V
actually works and doesn't fry anything. This of course means my
aliexpress LDO is not rated high enough for the BT/WiFi spike. I would
like to try some bigger caps across the output to see if that helps. I've
got a big 220uF right here, but it's late.

I've made another order of parts like a buck-boost convertor, touch screen
and breadboard. I've also got a cart full of audio supplies like a MAX
mono amp and a small selection of tiny speakers. When these components
come in, I will (for the 15th time) be done prototyping haha. Well, I just
want it all working on a single board with all the devices connected so
that there are no suprises when the PCB comes in. I don't want to find out
theres another 500mA current spike somewhere and not have the hardware to
deal with it. 

LETSSS GOOOO!!!

# September  3 2025 - Wednesday  6:39 PM
It's been non-stop schematic time. Pouring over datasheets and reading
requirements to find the perfect components. I have almost finished this quest.
The last beast to slay, and the most important one, is the power management.
I want a complex system right now, and that may need to change:
* LiPo Battery Charging
* Battery Protection
* Battery Fuel Gauge
* Buck Convertor for ESP32 and display
* LDO for Audio and RF

I have found many suitable PMICs that have these requirements, but I am not
sure if I want to have this level of complexity. Part of me, just to keep it
simple, wants to do LiPo -> TP4056 -> 3.3V LDO -> power whole system. If
I can't decide or commit to a real PMIC, this discrete solution will work
perfectly fine :). Here's to knocking out the last of the part selection so
that the schematic can be finished and fine-tuned.
