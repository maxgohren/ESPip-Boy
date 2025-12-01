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
