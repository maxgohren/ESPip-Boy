
# When finished assembly
* Check for shorts on all SMT components
* Check for shorts between chips and power rails
# Before Ordering
* No DRC errors of course
* Manully check each component and circuit on the board to make sure it was wired correctly
* 
# USB Impedance
USB 2.0 spec defines a differential characteristic impedance of 90 ohms, +/- 15%.
[Controlled Impedance PCB Layer Stackup- JLCPCB](https://jlcpcb.com/impedance)
https://jlcpcb.com/pcb-impedance-calculator

# Standard 
Changing all signal tracks to 6 mil for 60 Ohms, increasing space between tracks as well.
All power tracks not in polygons are 21 mil
# Copper Polygons
Surround power paths with copper fills (polygons) to lower resistance and give power an easier time to flow.
# Design Notes
* Start with an existing schematic and adapt it to your needs (like all learning)
* Thicker traces for power traces, thinner for signal
* Never go to minimum trace width/spacing -> you are increasing crosstalk
* Space traces 3 * tracewidth from each other
* Don't put silkscreen on vias or pads, it will not print on copper -> can tent vias to be able to print on them

## ESP32 Flash and PSRAM Design Guidelines
[PCB Layout Design - ESP32 - — ESP Hardware Design Guidelines latest documentation](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/pcb-layout-design.html)
[esp32-sche-core.png (1602×816)](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/_images/esp32-sche-core.png)
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32/schematic-checklist.html

# Notes on Battery + Power Circuit Design
https://www.best-microcontroller-projects.com/tp4056-page2.html
* TP4056 for charging of battery
* DW01 + Dual NMOSFET for overcharge and undercharge protection
* "LOAD SHARING": Final PMOSFET for disconnecting battery from load while charging: if load is still connected, it can throw off battery voltage readings

# Series Termination
Matching the impedance of the transmission line to improve signal quality

# Good PCB Design: 
* Working and Functional
* Signal Integrity
* EMI Performance
* IPC Standards Conformance
Submitting to Reddit r/PrintedCircuitBoard and Friends for Review
#### Sources
*Start with free resources before spending money*
Robert Federac
[How to Achieve Proper Grounding - Rick Hartley](https://www.youtube.com/watch?v=ySuUZEjARPY)
[PCB Design PDF](https://alternatezone.com/electronics/files/PCBDesignTutorialRevA.pdf)
#### Tools
[Saturn PCB Toolkit](https://saturnpcb.com/pcb_toolkit/)


#### Tutorials
[ESP32 Dev Board](https://www.youtube.com/watch?v=S_p0YV-JlfU&ab_channel=RobertFeranec)
[LiPo Battery Circuit](https://www.youtube.com/watch?v=Fj0XuYiE7HU&ab_channel=GreatScott%21)