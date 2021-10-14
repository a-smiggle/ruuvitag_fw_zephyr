# Main makefile of ruuvi zephyr

BUILD_DIRECTORY := build

.PHONY: clean app flash

all: clean app flash

clean:
	@echo Removing build build directory
	rm -f -d -r build

app:
	@echo Building App
	west build -b ruuvi_ruuvitag

app_debug:
	@echo Building App with Debugging
	west build -b ruuvi_ruuvitag -- -DOVERLAY_CONFIG=debugging.conf

app_bootloader:
	@echo Building MCUBOOT compatible app
	west build -b ruuvi_ruuvitag -- -DOVERLAY_CONFIG=mcuboot.conf

app_bootloader_debug:
	@echo Building MCUBOOT compatible app
	west build -b ruuvi_ruuvitag -- -DOVERLAY_CONFIG=mcuboot_debugging.conf

flash:
	@echo Flashing last built image
	west flash

connect:
	JLinkExe -device NRF52 -if SWD -speed 4000 -autoconnect 1

debug:
	JLinkRTTClient

erase:
	nrfjprog --family nRF52 --eraseall
