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
