BUILD_DIR = build
SRC_DIR = src

default: build

build:
	avr-gcc -DF_CPU=16000000UL -mmcu=atmega328p ${SRC_DIR}/main.c -o ${BUILD_DIR}/main

assembly:
	avr-gcc -S -o ${BUILD_DIR}/main.s -DF_CPU=16000000UL -mmcu=atmega328p ${SRC_DIR}/main.c

burn: build
	avr-objcopy -O ihex -R .eeprom ${BUILD_DIR}/main ${BUILD_DIR}/main.hex
	avrdude -F -V -c arduino -p m328p -P /dev/ttyACM0 -b 115200 -U flash:w:${BUILD_DIR}/main.hex
