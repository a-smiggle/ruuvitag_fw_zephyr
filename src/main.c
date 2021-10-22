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
#include "bme280_handler.h"
#include "lis2dh12_handler.h"
#include "battery_handler.h"
#include "board_info.h"
#include "ruuvi_endpoint.h"
#include "nfc_handler.h"
#include "bt_handler.h"
#include "tmp117_handler.h"

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

#if CONFIG_RUUVITAG_NFC_SENSOR_DATA
static int64_t last_nfc_update = 0;
#endif
static int64_t lastPressed = 0;
static int64_t btn_time = 0;

static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	btn_time = k_uptime_get();
	
	if (btn_time - lastPressed > 1000){
		toggle_red(1);
		toogle_green(1);
	}
	lastPressed = btn_time;
}

void main(void)
{	
	LOG_INF("Ruuvitag Started");
	LOG_INF("Version: %s", log_strdup(CONFIG_RUUVITAG_APP_VERSION));
	
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
	ruuvi_endpoint_sensor_check();

	/*
	 * Enables the filsesystem and mgmt groups that are required to
	 * enable dfu functionality.
	 */
	dfu_init();

	/* Initialize the Bluetooth Subsystem.*/
	bt_init();

	/* NFC must be done after BT so that MAC can be received. */
	ruuvi_nfc_init();
#if CONFIG_RUUVITAG_NFC_SENSOR_DATA
	last_nfc_update = k_uptime_get();
#endif
	while (true) {
		toggle_green(1);
		bt_update_packet();
		/* Turn LEDs off */
		toggle_green(0);
#if CONFIG_RUUVITAG_NFC_SENSOR_DATA
		if(k_uptime_get() - last_nfc_update > RUUVI_NFC_REFRESH){
			ruuvi_nfc_update();
			last_nfc_update = k_uptime_get();
		}
#endif

		k_sleep(K_MSEC(980));
	}
}
