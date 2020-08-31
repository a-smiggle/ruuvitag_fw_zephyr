#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <power/power.h>
#include <stdio.h>
#include <sys/util.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "ruuvi.h"
#include "led_handler.h"
#include "button_handler.h"
#include "bme280_handler.h"
#include "lis2dh12_handler.h"
#include "battery_handler.h"
#include <power/power.h>

/* BLE Settings */
/* 1600 * 0.625ms = 1 second */
static int MIN_ADV_INT=	1600;
static int MAX_ADV_INT=	1600;
static bt_addr_le_t mac[CONFIG_BT_ID_MAX];
/* Allows for controlling of Speed */
#define BT_LE_ADV_PRJ BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				      MIN_ADV_INT, \
				      MAX_ADV_INT, NULL)


static int SLEEP_TIME = 1;
static bool FAST_MODE = true;
static int16_t vbatt = 0;
static uint16_t acceleration_events = 0;
static uint32_t packet_counter = 0;
static int8_t tx_pwr = 0;
static bool has_lis2dh12 = false;
static bool has_bme280 = false;
static bool has_adc = false;
static int64_t pressed = 0;

static int get_tx_power()
{
	if(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_PLUS_4)){
		return 4;
	}
	else if(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_MINUS_4)){
		return -4;
	}
	else if(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_MINUS_8)){
		return -8;
	}
	else if(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_MINUS_12)){
		return -12;
	}
	else if(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_MINUS_16)){
		return -16;
	}
	else if(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_MINUS_20)){
		return -20;
	}
    else{
		return 0;
	}
}

static struct gpio_callback button_cb_data;

static void button_pressed(struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	int64_t time = k_uptime_get();
	
	if (time - pressed > 1000 && FAST_MODE){
		toggle_red(1);
		MIN_ADV_INT = 3200;
		MAX_ADV_INT = 3200;
		SLEEP_TIME = 2;
		FAST_MODE = !FAST_MODE;
		/* Will change mode of RuuviTag */
	}
	else if(time - pressed > 1000 && !FAST_MODE){
		toggle_green(1);
		MIN_ADV_INT = 1600;
		MAX_ADV_INT = 1600;
		SLEEP_TIME = 1;
		FAST_MODE = !FAST_MODE;
	}
	pressed = time;
}

static uint8_t mfg_data[] = { 0xff, 0xff, /* MFG_ID */
						0x09, 		/* Packet Mode */
						0x00, 0x00, /* Temperature */
						0x00, 0x00, /* Humidity */
						0x00, 0x00, /* Pressure */
						0x00, 0x00, /* X */
						0x00, 0x00, /* Y */
						0x00, 0x00, /* Z */
						0x00, 0x00,	/* vBatt & TX */
						0x00,		/* Acceleration Events */
						0x00, 0x00, /* Packets Sent */
						0x00, 0x00, /* MAC address */
						0x00, 0x00, /* MAC address */
						0x00		/* MAC address */
};

void get_mac(void){
	
	size_t count = CONFIG_BT_ID_MAX;

	bt_id_get(mac, &count);
}

void set_pk_mac(void){
	mfg_data[20] = mac->a.val[5]; 
	mfg_data[21] = mac->a.val[4];
	mfg_data[22] = mac->a.val[3];
	mfg_data[22] = mac->a.val[2];
	mfg_data[23] = mac->a.val[1]; 
	mfg_data[24] = mac->a.val[0];
}

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data))
  
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	//printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_PRJ, ad, ARRAY_SIZE(ad),
					NULL, 0);
	if (err) {
		//printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	get_mac();

	set_pk_mac();

	//printk("Advertising successfully started\n");
}

static void sensor_handler(void)
{
	if (FAST_MODE){
		toggle_green(1);
	}
	else{
		toggle_red(1);
	}
	int16_t t = 0;
	uint16_t h = 0, p = 0;
	int16_t x = 0, y= 0, z = 0;

	if (has_adc){
		if( packet_counter % 10 == 0){
			vbatt = get_battery_level();
			//printk("Battery: %d \n", vbatt);
		}
	}

	if(has_bme280){
		bme280_fetch();
		t = bme280_get_temp();
		p = bme280_get_press();
		h = bme280_get_humidity();
		//printk("T: %d, P: %d, H: %d\n", t, p, h);
	}

	if(has_lis2dh12){
		lis2dh12_fetch();
		x = lis2dh12_get(0);
		y = lis2dh12_get(1);
		z = lis2dh12_get(2);
		//printk("X: %d, Y: %d, Z: %d\n", x, y, z);
	}
	
	mfg_data[3] 	= 	((t)>>8);
	mfg_data[4] 	= 	((t) & 0xFF);
	mfg_data[5] 	= 	((h)>>8);
	mfg_data[6] 	= 	((h) & 0xFF);
	mfg_data[7] 	= 	((p)>>8);
	mfg_data[8] 	= 	((p) & 0xFF);
	mfg_data[9] 	= 	((x)>>8);
	mfg_data[10] 	= 	((x) & 0xFF);
	mfg_data[11] 	= 	((y)>>8);
	mfg_data[12] 	= 	((y) & 0xFF);
	mfg_data[13] 	= 	((z)>>8);
	mfg_data[14] 	= 	((z) & 0xFF);
	vbatt 			-= 	1600; //Bias by 1600 mV
    vbatt 			<<= 5;   //Shift by 5 to fit TX PWR in
    mfg_data[15] 	= 	(vbatt)>>8;
    mfg_data[16] 	= 	(vbatt)&0xFF; //Zeroes tx-pwr bits
    mfg_data[16] 	|= 	((uint8_t)tx_pwr)&0x1F; //5 lowest bits for TX pwr
	mfg_data[17] 	= 	acceleration_events % 256;
	mfg_data[18] 	= 	(packet_counter>>8);
	mfg_data[19] 	= 	(packet_counter&0xFF);
	/* Update advtisement data */
	bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
	++packet_counter;
	toggle_green(0);
	toggle_red(0);
}

void main(void)
{
	int err;
	
	led_init();

	button_init();

	if(button_pressed_state()){
		/* Idea is to have MCUboot functionilty start here */
		printk("Button Pressed at boot\n");
#ifdef CONFIG_BOOTLOADER_MCUBOOT
		#include "dfu_common.h"
		dfu_init();
#endif // CONFIG_BOOTLOADER_MCUBOOT
	}
	else{
		button_int_setup(&button_cb_data, button_pressed);
		
	}

	/*has_adc = init_adc();
	if (!has_adc) {
		printk("Failed initialize ADC\n");
		flash_red();
		return;
	}*/

	has_bme280 = init_bme280();
	if (!has_bme280) {
		printk("Failed initialize BME280\n");
		flash_red();
		return;
	}
	
	has_lis2dh12 = init_lis2dh12();
	if (!has_lis2dh12) {
		printk("Failed initialize LIS2DH12\n");
		flash_red();
		return;
	}

	/* Checks Tx power */
	tx_pwr = get_tx_power();

	printf("TX Power set to: %d \n", tx_pwr);
	/* Prepare TX power for packet */
	tx_pwr += 40;
    tx_pwr /= 2;

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		//printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	while (true) {
		sensor_handler();
		k_sleep(K_SECONDS(SLEEP_TIME));
	}
}