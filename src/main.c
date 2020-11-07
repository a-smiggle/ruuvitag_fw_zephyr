#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <power/reboot.h>
#include <power/power.h>
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
static bool BT_STATE = false;
static int64_t btn_time = 0;

static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	btn_time = k_uptime_get();
	
	if (btn_time - lastPressed > 1000 && !BT_STATE){
		toggle_red(1);
		BT_STATE = !BT_STATE;
		bt_mode_switch(BT_STATE);
	}
	lastPressed = btn_time;
}


void main(void)
{	
	led_init();

	button_init();
	/*
	 * If button B is pressed, the device will be connectable
	 * and ready for a DFU. This state will be reverted after a specific
	 * amount of time, controllable by using CONFIG_RUUVITAG_DFU_TIMEOUT.
	 * Default is 2 minutes.
	 */
	BT_STATE = button_pressed_state();
	if(BT_STATE){
		LOG_INF("Button Pressed at boot.\n");
	}
	button_int_setup(&button_cb_data, button_pressed);
	
	/*
	 * Initialised the sensors and makes them ready to fill the
	 * BLE TX buffer.
	 */
	ruuvi_endpoint_sensor_check();

	/*
	 * This is required to allow the application to easily toggle
	 * between BT modes.
	 */
	dfu_init();

	/* Initialize the Bluetooth Subsystem.*/
	bt_init(BT_STATE);

	/* NFC must be done after BT so that MAC can be received. */
	ruuvi_nfc_init();

	while (true) {
		if(!BT_STATE){
			toggle_green(1);
			bt_update_packet();
			/* Turn LEDs off */
			toggle_green(0);
			
			k_sleep(K_MSEC(980));
		}
		else{
			if(k_uptime_get() - lastPressed > RUUVI_DFU_TIMEOUT){
				BT_STATE = false;
				bt_mode_switch(BT_STATE);
			}
			flash_red();
			k_sleep(K_SECONDS(1));
		}
	}
}
