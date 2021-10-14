# ruuvitag_fw_zephyr
Ruuvitag FW based on Zephyr OS.

# Build Environment
Ruuvi Node is developed using nRF Connect SDK(NCS) V1.7.0.

## NCS
Nordic Semiconductor keeps up-to-date instructions on how to setup the SDK for 
Linux, Mac OSX and Windows. Instructions can be found at https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.7.0/nrf/getting_started.html

# Cloning
To clone the directory follow the following.

```bash
cd ~/ncs/nrf/applications
git clone https://github.com/theBASTI0N/ruuvitag_fw_zephyr.git
cd ruuvi.node_nrf91.c
```

## Note 
The examples given assume you are using linux.
All commands assume that nRF Connect SDK has been setup as per the instructions in the above link.

# Building West
Once in the project directory, make can be used to build the different variances of the firmware and flash them to a board.

```bash
# Makes the standard FW and flashes it to a connected board.
make

# Makes the standard FW with or without debugging enabled.
make app
make app_debug

# Makes the full FW with or without debugging enabled. (Includes MCUBOOT)
make app_bootloader
make app_bootloader_debug

# Flashed the last built FW
make flash

# Connects to the device to view debu messages
make connect

# Displays the debug messages
make debug

# Cleans all build directories
make clean
```

# Remote DFU
To enable your ruuvitag to receive an update the following can be done:
- Press B when rebooting the tag.
- Press B at any time whilst running.

Once in the update mode nRF connect can be used on a phone to update the tag. The process is the same as a standard ruuvitag, however the file need is the update.bin file. This can be found at: ruuvitag_fw_zephyr/build/zephyr/app_update.bin or in the releases.

## DFU Timeout
The update mode of the tag will by default time out after 2 minutes. The tag will then start beaconing again.

# Note
The GitHub workflow is adapted from [@bifravst/firmware](https://github.com/bifravst/firmware) and is BSD-5-Clause-Nordic licensed.
