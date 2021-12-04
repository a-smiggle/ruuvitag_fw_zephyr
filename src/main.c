/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <power/reboot.h>
#include <pm/pm.h>
#include <stdio.h>
#include <sys/util.h>
#ifdef CONFIG_BOOTLOADER_MCUBOOT
#include "dfu/mcuboot.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
#include <device.h>
#include <fs/fs.h>
#include "fs_mgmt/fs_mgmt.h"
#include <fs/littlefs.h>
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif

#include "ruuvi.h"
#include "led_handler.h"
#include "button_handler.h"
#include "board_info.h"
#include "bt_handler.h"

#include "sensor_handler.h"
#include "nfc_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(ruuvitag, CONFIG_RUUVITAG_LOG_LEVEL);

#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(cstorage);
static struct fs_mount_t littlefs_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &cstorage,
	.storage_dev = (void *)FLASH_AREA_ID(storage),
	.mnt_point = "/lfs"
};
#endif

static void dfu_init(void){
	/* Register the built-in mcumgr command handlers. */
#ifdef CONFIG_MCUMGR_CMD_FS_MGMT
	int rc = fs_mount(&littlefs_mnt);
	if (rc < 0) {
		LOG_ERR("Error mounting littlefs [%d]", rc);
	}

	fs_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
	os_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
	img_mgmt_register_group();
#endif
}

static int64_t lastPressed = 0;
static int64_t btn_time = 0;

static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	btn_time = k_uptime_get();
	
	if (btn_time - lastPressed > 1000){
		toggle_red(1);
		toggle_green(1);
	}
	lastPressed = btn_time;
}

static void init_peripherals(void)
{
	led_init();

	button_init();
	
	/*
	 * This could be used in the future to perform any numbers 
	 * of tasks.
	 */
	if(button_pressed_state()){
		LOG_INF("Button Pressed at boot.\n");
	}
	button_int_setup(&button_cb_data, button_pressed);

	/*
	 * Initialises the sensors and makes them ready to fill the
	 * BLE TX buffer.
	 */
	sensor_init();
}

void main(void)
{	
	LOG_INF("Ruuvitag Started");
	LOG_INF("FW Version: %s", log_strdup(CONFIG_FIRMWARE_VERSION));

	init_peripherals();

	/*
	 * Enables the filsesystem and mgmt groups that are required to
	 * enable dfu functionality.
	 */
	dfu_init();

	/* Initialize the Bluetooth Subsystem.*/
	bt_init();
	/* NFC must be done after BT so that MAC can be received. */
	nfc_init();

	while (true) {
		toggle_green(1);
		udpate_sensor_data();
		/* Turn LEDs off */
		toggle_green(0);
		k_sleep(K_MSEC(1000));
	}
}
