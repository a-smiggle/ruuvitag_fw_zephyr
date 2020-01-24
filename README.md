# Zephyr Ruuvitag Board Definition

This includes the files need to make your Ruuvi work with Zephyr OS.

This has been tested on Zephyr OS v2.1.99.

# Usage

To use your Ruuvi board simply clone this repository into the boards/arm/ folder:

```bash
#clones into boards/arm as nrf52_ruuvi
git clone https://github.com/theBASTI0N/zephyr-ruuvi-apps.git ~/zephyrproject/zephyr/boards/arm/nrf52_ruuvi

```

You can then test that it works with a sample application.

```bash
#naviagte to:
cd ~/zephyrproject/zephyr
#build the basic blinky application for ruuvi
west build -b nrf52_ruuvi samples/basic/blinky
#flash your board
west flash
```

# Prerequisites

You will be required to have setup the zephyr development environment on your system.

Follow the instructions in the below links to setup the environment:
https://docs.zephyrproject.org/latest/getting_started/index.html
https://docs.zephyrproject.org/latest/guides/west/install.html

You will need to ensure nrfjprog is installed so that the board can be flashed. Thisd is most eaily done via installing nRF Command Line Tools.

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