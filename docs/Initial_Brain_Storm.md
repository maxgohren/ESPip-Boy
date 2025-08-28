# Inspiration
https://github.com/Bellafaire/ESP32-Smart-Watch
[Making DIY Smart Watch with ESP32 - Designing Smart Watch Faces](https://circuitdigest.com/microcontroller-projects/designing-smartwatch-using-esp32)
[ZSWatch]([https://github.com/ZSWatch/ZSWatch?tab=readme-ov-file#hardware)
[mike-rankin/ESP32-S3_1.69inch_Cap_Touch](https://github.com/mike-rankin/ESP32-S3_1.69inch_Cap_Touch)
# Research Questions
## Straight up JARVIS Voice Control
1. 🟨Text-to-speech synthesizer? 
	* can definitely do, need a speaker but the audio will be robotic (low processing power)
2. 🟨Speech-to-text machine learning model? 
	* definitely not do on board (would be limited to YES or NO processing, would need to offload to a server with WIFI (doable -> use phone 5G even, or maybe on the phone itself?))
3. ✅ Microphone onboard
	* definitely doable
1. ✅ Speaker on board

## PCB Design and Assembly
1. What tools do I need?
		* Soldering Iron, solder
		* Hot plate for SMD, solder paste
1. What is a cost effective way to get electronic components, and how would it influence my design? 
	1. For example, can I use the same kinds of filtering capacitors all over my board or do different components have different requirements?
			* Yes you can
1. What are some absolute basic design principles to follow for a successful PCB? Test pads? Etc.
	* Test Pads
	* Trace width calculations adequate for expected current
	* Big Ground Plane
	* Via minimization
	* Good silkscreen with clear names and organization of parts (don't put on vias!)
	* Design Rules Checker
1. What are some advanced design features to ensure a successful PCB?


# Design Goals
* Wearable smart watch
* LED Flashlight
* GPS Module with loadable maps and GPS app
* Music storage and playback via Bluetooth
* Learn about PCB Design
	* Trace length
	* High speed traces and interference
		* SD Card reading
		* USB input


# Build Guide
[Circuit Digest](https://circuitdigest.com/microcontroller-projects/diy-smartwatch-using-esp32-part3-interfacing-magnetometer-and-gyroscope-with-esp32)
# Hardware

	#### LiPo Battery Circuit
* TP4056 battery charging circuit
* USB input
* DW01 protection circuit
	* Discharge and Overvoltage protection
	* Requires 2 Dual-Channel N-type MOSFETs from FS8205
* What voltages do I need for all my components?
	* Do I need boost convertors to increase the voltage to 5V?
	*  Definitely need LDO for dropping voltage down to 3.3V or 1.8V
#### ESP32 MCU
* Dual core allows for music playback
### Display
1.69" 240x280 pixel TFT LCD display
2.4" 320x240 capacitive touch TFT LCD Display
**FPC connector** for plugging in display instead of soldering
### Controls
* EC11 Rotary Encoder with Button
* 2 Buttons
### Storage
* SD Card Adapter for storage of Maps and Music
### Flashlight
RBG LED w/ heatsink and power supply?
### Laser BEAM!?
### Sensors
#### BN180 GPS Module
#### LSM303DLHC Accel/Magnetometer OR MPU6050 Accelerometer + QMC5883L Magnetometer
for tracking steps
auto wakeup
Triple Axis Magnetometer for Compass

| IMU Chip                                      | Axes   | Extra features                                                        | Notes                                                             |
| --------------------------------------------- | ------ | --------------------------------------------------------------------- | ----------------------------------------------------------------- |
| **MPU-6050** (InvenSense)                     | 6      | Simple accel+gyro, cheap (~\$2), no magnetometer                      | Old, needs external processing for gestures.                      |
| **MPU-9250** / **ICM-20948** (InvenSense/TDK) | 9      | Has DMP, can output orientation, tap detection                        | More expensive (~\$6-10)                                          |
| **LSM6DSOX** (STMicro)                        | 6      | Built-in machine learning core for gesture classification in hardware | Great for ultra-low-power wearables, supports wrist tilt, pickup. |
| **BNO055** (Bosch)                            | 9      | Built-in sensor fusion/calibration                                    | Simple but more power hungry.                                     |
| **BMI270 + BMM150** (Bosch)                   | 6 + 3  | BMI270 = 6-axis with gesture/tap detection, BMM150 = magnetometer     | Can be paired if needed.                                          |
| **LIS3DH / LSM6DSL** (STMicro)<br>            | 3 or 6 | Basic motion detection                                                | Common in low-end wearables.                                      |
|                                               |        |                                                                       |                                                                   |
|                                               |        |                                                                       |                                                                   |
|                                               |        |                                                                       |                                                                   |
ChatGPT recommends LSM6DSOX as it has a built in ML core for gesture detection. That would be really cool and low power. 
I can then throw on a cheap magnetometer like the LSM6DSL for my compass app!
LGA is a pain in the ass, might have to get it assembled by PCB manufacturer.
Could also do MLC for MPU6050 in python library.

#### BH1750 Light Sensor
Change display brightness based on ambient light
#### MAX30102 Heart Rate / Blood Oxygen Sensor

#### 3D Printed Case
Design Inspo -> rough paint edges for a worn look!!! Super Sick! 
https://www.youtube.com/watch?v=l75A9TUMXOs&ab_channel=abe%27sprojects

Form Factor inspo DM99 Rectangular face smart watch
Maybe offset the strap?
![[Pasted image 20250714144210.png]]


# Software
## OS
* FreeRTOS
* NuttX

### Applications
* Local music player over bluetooth
* GPS app with user-uploadable maps via USB
* Main Display app with configurable nodes
	* Clock
	* Temperature / Humidity
	* Heartbeat
	* Compass
* Resource Monitor App
	* Battery
	* CPU -> only task count is available, CPU usage is not like traditional PCs due to different task scheduling
	* RAM
	* SD Card storage