# Zephyr Ruuvitag Board Definition

This includes the files need to make your Ruuvi work with Zephyr OS.

This has been developed using Zephyr OS v2.3.99.

On 2.4.0 release development will remain on that release as Ruuvitag will be incorporated into repo.

# Usage

There are two methods to use this application. If you  already have zephyr instaleld you can clone this repo or if you are starting from fresh you can use west to pull this repo and all required zephyr components.

## Zephyr already installed

It is assumed that you followed the instructions found at:
https://docs.zephyrproject.org/latest/getting_started/index.html

```bash
cd ~/zephyrproject
git clone https://github.com/theBASTI0N/zephyr-ruuvi.git ruuvi
cd ruuvi

```

### Build
```bash
#navigate to:
cd ~/zephyrproject/ruuvi
west build
#flash your board
west flash
```

## Fresh Install

```bash
cd ~/
mkdir ruuvi_zephyr
cd ruuvi_zephyr
west init -m https://github.com/theBASTI0N/zephyr-ruuvi
west update
pip3 install -r zephyr/scripts/requirements.txt
```

### Build
```bash
#navigate to:
cd ~/ruuvi_zephyr/ruuvi
west build
#flash your board
west flash
```

# Prerequisites

You will be required to have setup the zephyr development environment on your system.

Follow the instructions in the below links to setup the environment:
https://docs.zephyrproject.org/latest/getting_started/index.html
https://docs.zephyrproject.org/latest/guides/west/install.html

## Alternative toolchain

An alternative toolchain can be installed by following the instructions found at:
https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#installing-the-required-tools

You will need to ensure nrfjprog is installed so that the board can be flashed. This is most eaily done via installing nRF Command Line Tools.

To install this download and run the required file for you OS. This can be found at:
https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools

If using linux you may need to add your user to the dialout group to access the serial device. To do this run the following:

```bash
# On debian
sudo usermod -a -G dialout $USER
#On Arch the group is uucp
sudo usermod -a -G uucp $USER
#reboot your system for this to take effect
sudo reboot
```
