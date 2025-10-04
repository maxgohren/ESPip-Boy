# October  3 2025 - Friday 10:32 PM
Schematic Work! 

The prototype has come along so nicely now that the microphone and speaker have
been proved out, sharing the serial and bit clock lines. This saves me
a couple of pins, and I am using all four input only GPIOs on the ESP32 for my
rotary encoder and Mic in. 

All that is left now to prove out on the prototype is the necessity of a GPS
and the interrupt pin for my BMI270 accelerometer (this will be tested with my
Aliexpress fake MPU6050 haha). 

The necessity of the GPS will be decided after I render a JPEG of a map on my
display. I anticipate it being too low res for any worthwhile usage, which then
makes me question the need of a GPS. My use case for it is rendering my real
position on top of a JPEG of a map, so that I can trace paths I run or hike and
provide a live position/movement of map. If the map render does not look good,
I don't need the GPS. It would be nice to have for a spedometer, but not worth
the development time at that point.

The accelerometer will be very easy to do, I just need to wire it up and attach
the interrupts. Then, do a little sleep test and see if it can wake it up
(which I am confident it can). 

As the hardware prototype finishes up, and I am more interested in removing
parts as much as adding them, I want to prepare for more PCB work. I am closing
in on finishing the schematic along with the prototype, and then all that is
left is my PCB design and order! However, I am really not comfortable when it
comes to the PCB design aspect. It's a big project and I have very little
experience. 

I plan to take some time to do some tutorials, as well as make simple designs
like I2C breakout boards for sensors and battery modules. That way, when it
comes to the interaction of all the different buses and components on the full
project, I will have the experience of doing the smaller components first!

# October  2 2025 - Thursday 11:44 PM
I spent the night tonight writing a WAV File decoder. I read the [1991
specification](https://www.aelius.com/njh/wavemetatools/doc/riffmci.pdf) by IBM and Microsoft, and got confused so I consulted the wikipedia page which helped a lot in describing the actual byte counts of the fields in the header (I got stuck on the omitted chunkSizes in the weird description format used in the official spec.). 

I was able to successfully decode ALL BUT the data size of my recording from my
ESPip-Boy microphone. I am still not sure why the data size is returned as
a zero. My code is dead simple, but alas, the bug is invisible to me.

I am super proud of myself for allowing my curiosity to take me to the real
documents that outline this technology. I am very happy to not need to rely on
generative AI to solve these problems for me, when the information to solve the
problem yourself is so readily available. 

I can't describe the joy of writing a simple C program to load a WAV file and
print the first four bytes and seeing the 'RIFF' pop up in the console. It
feels like my first "Hello World" all over again!

# October  1 2025 - Wednesday  9:37 PM
It's the first of October! Today is the day that I've learned an important part
of embedded software engineering (and really all software engineering). Domain
knowledge vs. programming knowledge.

Over the last 5 years I've really become an avid programmer from basically
not knowing anything about it (I wrote my first program in my last year of
highschool before leaving to uni for electrical engineering). In that time
I have gained a solid grasp of C programming and low level software - bit
manipulation, pointers and memory addressing etc. However, I've only scratched
the surface of other digital domains: audio, video, image processing and
communication protocols. This is why I love this project so much, it exposes me
to ALL of it!

As I have the programming experience, the thing that is missing is the domain
experience. I have no problem reading or writing data to a circular queue of my own design. But sending PCM audio to an I2S device? Both concepts may be fundamentally identical, but the audio problem presents so much new language, so many new concepts, and so many ways to organize the simple flow of data from A to B that it becomes an entirely different situation.

My wake up call was yesterday when I finally recorded audio from my MEMS
Microphone to a WAV File on an SD card. After searching the web and trying so
many code snippets, cutting and pasting and modifying endlessly, I only found
success when asking ChatGPT to do it for me. I am not proud. It made me realize
that while I have a high level understanding of the digital and analog domains
my smart watch operates in, what software and hardware is required to make it
work, I have NO IDEA how it actually operates under the hood. Enter the deep
dive. 

From now on, I would like to deep dive into my components and modules, and
actually learn enough to make them work "from scratch" (within reason haha).
I want to write my own SPI drivers. I2C drivers. Create my own sine waves and
write them out to I2S devices. How do I generate a sine wave to become an audio
signal? How does PCM audio work? What is PCM audio?

Now that I have proved out that all my ideas work over the last couple of
months with boilerplate code, it is time to actually get my hands dirty
(dirtier) and learn the concepts behind all of the wonderful audio, imaging and
communication that makes my prototype ESPip-Boy function so well.

Here's to deeper understanding, here's to endless curiosity and here's to the
wonderful fascination that is embedded software engineering!
