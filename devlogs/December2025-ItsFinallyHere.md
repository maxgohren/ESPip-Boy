# December 31 2025 - Wednesday  5:45 PM
Another tough day working with FreeCAD to get complete my case. Today
I actually sent in a .stl to get printed, but as I sent it in I noticed my
hastily designed screw terminals completely block my PCB from actually fitting
into the case!

That's what you get for throwing things together on the fly on New Year's Eve!

It's been a great 2025, and I am so proud of myself for embarking on this
journey and learning all these amazing skills. I look forward to what insane
projects I get up to next year!

# December 30 2025 - Tuesday  9:47 PM
Today I spent HOURS trying to get my case complete. Between the fillet and thickness 
tool on FreeCAD being very picky and the PCB import being very laggy it was
grueling and arduous process that I still have not completed. I think I will do
a mockup tomorrow to find the correct order to do my components.

For example, I first made a filleted pad and then made holes in it. I was
unable to make a thickness into this structure.

Then, I double back and use the Set Tip feature to add a thickness before I add
my pockets. This then resulted in my pockets not being through holes but actually holes with their bottoms sealed up. Very bizarre. 

Next I will try:
Main body pad -> thickness -> component pockets -> fillet -> watch connectors
-> split case in half -> add top/bottom half connections

I will do a quick run through these will 1 step per each (instead of doing all
the actually required holes) so that I know they work.

One thing I really need is to make the thickness inside the case, and then add
new pads to extrude the watch connectors from, without them being deleted from
the thickness operation.

# December 29 2025 - Monday 10:21 PM
PCB REV2 ASSEMBLED! It works! It's alive!

Touchscreen works great and the display cable is not folded into any sharp
corners. Only smooth bends. I'm so excited! Now I just need to finish my case
and I am all done!

# December 19 2025 - Friday 10:26 PM
PCB Rev2 on the way!

# December 17 2025 - Wednesday  9:25 PM
Today I reviewed my PCB for any flaws like via in traces or pads too close to
each other. I found nothing so far, I think one more review session and I will
be ready to order.

I also checked the rest of my so far untested circuits. This was just the rest
of my I2C devices which I already new somewhat worked due to responding to my
I2C scan. The RTC, magnetometer and light sensor all respond to my commands. So
far on the RTC makes sense though, but that's to be expected. The magnetometer
and light sensor will no doubt need some actual software for calibration.

I also found for the RTC that other brands like EPSON make a RTC with code
8010, which made me discover my YSN8010 is a drop-in clone for these devices.
I will try a library for the EPSON device soon, as the RTC is quite annoying to
use. The time components are binary coded decimal within the device, and
although it's fun to do some bit flipping, it would be even more fun to drop
a library into my project. I got the hours, seconds and minutes working, so if
I can't get a library I can roll my own.

I'm looking forward to another check over of the PCB and firing away on the
order button!

# December 13 2025 - Saturday  5:48 PM
Finished V2 of the board, back on EasyEDA. I realised using KiCAD was too much
effort for the too little amount of changes I needed to make. Tested the rest
of the components and everything is working as expected except for the motor
and the microphone. Speaker, SD card, flashlight and heartrate breakout are all
good!

Looking forward to final checks on V2 and ordering it! Also very happy the hand
assembly using hot air on the bottom worked quite well!

# December 10 2025 - Wednesday 10:00 PM
Received my new display in the mail today! Validated my touchscreen I2C circuit
works! A wonderful step. The display came with an FPC connector, I am scheming
on modifying my current board to use the new one, but flipped, to accept my new
display in a cleaner way without compromising the FPC...

Experiment failed haha. It's okay, this board will still help me to verify the
rest of my circuit designs:
I2C:
* Magnetometer
* Ambient Light + UV Sensor
* RTC
I2S:
* Speaker driver
* Microphone
External Drivers:
* Vibration Motor
* Flashlight

For now the new board design is looking promising. I've laid out some more
components, and I think I can significantly reduce the board footprint, to
allow for the battery to sit on the same plane as the PCB, further reducing
watch thickness!

# December  8 2025 - Monday 11:28 PM
Huge cleanup of git tracking.

Cleaned up entire imported EasyEDA schematic in KiCAD, starting placing
components on a brand new PCB. Racking my brain trying to figure out the best
spot for the battery and display connector. Not proving the easiest challenge.

Looking forward to getting a second board in and making a slick case. Learned
about slicing in FreeCAD today to make a much better clamshell design that the
PCB can sit in, not the V1 design that I had to shove it in due to the deep
cutouts for the antenna and USB port. 

KiCAD and FreeCAD are awesome software. I look forward to contributing soon!

# December  7 2025 - Sunday  7:38 PM
Unfortunately my luck has run out with V1.0. After designing and printing the
case, and placing my components inside, my watch worked at least to the point
of displaying correctly. However, due to the mangled FPC connector of the
display, the I2C bus was broken. The only way to fix it? To move the display
around, jiggling and bending and hoping and praying. Not effective. My luck
finally ran out when I went to remove the friction fit display (which fit
wonderfully), and accidentally pried the backlight off of the display enough to
cause corruption in the display, like unwanted lines of the wrong colour
throughout the display. It's busted. 

It's here I officially can say V1.0 has been tested to the point of critical
failure, and the list of improvements is beginning to emerge for
V2.0:
* Place the display connector in such a way as to remove ALL force on the
  display connector
* Use right angle, SMD LEDs instead of a discrete solution
* Make all SMT pads for external speakers and vibration motors through hole for
  better stability

So far that's mostly it, I am pleasantly surprised with how well everything has
gone with the V1.0 PCB, and I am looking forward to building the next revision.

With that being said, I am learning and moving to KiCAD as my EDA tool. The
main reason for this is that .STEP file export is free, compared to the $20
monthly subscription fee for this feature on EasyEDA. The .STEP export will
allow me to really fine tune the case design, which now that the hardware has
been validated, is the next challenge to really bring this project home.

So far the only bump in the road with regards to KiCAD is that my easyeda2kicad
3D models are sometimes offset from the footprint. Otherwise the symbols and
footprints are perfect!

# December  3 2025 - Wednesday 12:23 AM
Working on the SW to get it to a decent, useable state. Currently trying to
drain the battery to let the fuel gauge learn the battery State of charge. 

I am doing this by just printing the battery voltage to the display, and never
sleeping it. I've disabled the sleeping as I've added it right after the screen
off calls. Soon, I will have a watch that turns on when the user moves the
right amount (the watch focus gesture is pretty bogus so far), displays the
time and then sleeps after a 10 seconds of inactivity. 

Then I will add NTP time sync and time sync with the RTC after perhaps 24 hours
for the WiFi sync and 5 wakeups with the RTC, to be tuned of course, storing
the boot count in the RTC memory!

# December  1 2025 - Monday 12:33 AM
This is a pretty surreal feeling now, as I just finished up the auto wake
feature for the display on my smartwatch. I am actually wearing it, after
I installed a battery disconnect switch for some peace of mind, and installed
the assembly into a crappy plastic chopped up case and taped it in with an
actual watch strap to hold it on my wrist.

I am so happy it works! So far the auto wake is simply polling the
accelerometer, but I hope to use the wrist wake feature so that I can avoid
polling and safe battery by sleeping the CPU when it is in rest. However, the
default really sucks. It's borderline useless. The next thing I want to develop
is a proper, working wrist gesture interrupt with the BMI270, and then
implement low power sleep, and then after those 2 features, start adding low
power polling to my other peripherals like the magnetometer. All the while,
I will be making current draw measurements and battery life measurements using
my multimeter.

Looking forward to installing the real time clock and actually having
a functioning, decently featured smart watch!
