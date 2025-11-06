# November  5 2025 - Wednesday  9:58 PM
The last few days have been a lot of analysis paralysis. Being so close to
finished, I am more nit-picky than ever. Today I did my USB differential pair
and I spent hours wondering if I should include vias or route under the USB
connector or this or that or whatever. I decided I will do my best,
understanding that I won't ever have perfect signal performance and that's
okay. I also have 0 experience, so I have no instincts to base my intuition on,
it's just a mass of information that is overwhelming, and a lot of "that
shouldn't apply to you unless you're doing USB High Speed". I've decided my
differential pair and some vias should be fine.

I went over my BOM and started cleaning that up too. Added a footprint for the
MAX30102 heart rate and blood oxygen sensor, and then removed it from the BOM
because it costs 12$ on LCSC for whatever reason. Now I have an I2C breakout
and an actual footprint for it. Nice.

I am so close to ordering! Let's Go!

# November  1 2025 - Saturday  4:25 PM
Today I looked over every connection in my schematic, placing links to the
relevant reference schematics I've collected over the past few months.

Aside from some small changes including adding some 0 Ohm bypass resistors for
my battery into my battery monitor IC, and some more for the config of my
speaker driver, I found a big problem.

My display connector is backwards! If I leave it how it is, I would have to
wire my display in upside down! I won't be fixing this today, but hopefully it
is not as disastrous as I imagine it will be. I'm so glad I caught this!

Aside from fixing the display connector, my only other plans are to review the
PCB implementation of my now fully reviewed schematic, and then add copper
fills and thick ground traces for some more safety. Other than that, this
design is looking dangerously close to order time. >:).
