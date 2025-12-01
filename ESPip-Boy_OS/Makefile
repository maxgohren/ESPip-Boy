all: build up mon

build:
	#arduino-cli compile -b esp32:esp32:esp32 -v --output-dir build
	arduino-cli compile -b esp32:esp32:esp32 -v

up:
	arduino-cli upload -b esp32:esp32:esp32 -p /dev/ttyUSB0

mon:
	arduino-cli monitor -p /dev/ttyUSB0 --config 115200 --timestamp

clean:
	rm -rf build

