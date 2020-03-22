/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <stdio.h>
#include <sys/util.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include "led_controller.h"

//Allows for controlling of Speed
#define BT_LE_ADV_PRJ BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				      MIN_ADV_INT, \
				      MAX_ADV_INT)
/* BLE settings*/

// 1600 * 0.625ms = 1 second
#define MIN_ADV_INT		1600
#define MAX_ADV_INT		3200
//Allow Setting of BLE name
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

//Sleep times
#define SLEEP_TIME	1

//Allows for controlling of Speed
#define BT_LE_ADV_PRJ BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				      MIN_ADV_INT, \
				      MAX_ADV_INT)

/* Sensors and LEDs */
static struct device *lis2dh_dev;
static struct device *bme280_dev;

static unsigned int accel = 0;
static uint32_t packet_counter = 0;

static int tx_power = 0;

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

static u8_t mfg_data[] = { 0xff, 0xff, /* MFG_ID*/
						0x09, 		/* Mode*/
						0x00, 0x00, /*Temperature*/
						0x00, 0x00, /*Humidity*/
						0x00, 0x00, /*Pressure*/
						0x00, 0x00, /*X*/
						0x00, 0x00, /*Y*/
						0x00, 0x00, /*Z*/
						0x00,		/*TX*/
						0x00, 0x00 /*Packets*/};

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data)),
};

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};



static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_PRJ, ad, ARRAY_SIZE(ad),
					sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}
static int process_bme280(struct device *dev, struct sensor_value *val)
{
	if (sensor_sample_fetch(dev) < 0) {
		printf("Sensor sample update error\n");
		return -1;
	}

	if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &val[0]) < 0) {
		printf("Cannot read bme280 temperature channel\n");
		return -1;
	}

	if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &val[1]) < 0) {
			printf("Cannot read bme280 humidity channel\n");
			return -1;
	}

	if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &val[2]) < 0) {
		printf("Cannot read bme280 humidity channel\n");
		return -1;
	}

	
	return 0;
}

static void process_lis2dh(struct device *dev, struct sensor_value *val)
{
	const char *overrun = "";
	int rc = sensor_sample_fetch(dev);

	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(dev,
					SENSOR_CHAN_ACCEL_XYZ,
					val);
	}
	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
	}
}

#ifdef CONFIG_LIS2DH_TRIGGER
static void trigger_handler(struct device *lis2dh_dev,
			    struct sensor_trigger *trig)
{
	++accel;
	printk("Accelration Event: %d\n", accel);
}
#endif


static void sensor_handler(void)
{
	struct sensor_value lis2dh_val[3];
    struct sensor_value bme280_val[3];

	process_bme280(bme280_dev, bme280_val);
	process_lis2dh(lis2dh_dev, lis2dh_val);

	int16_t t = (int16_t)(sensor_value_to_double(&bme280_val[0]) * 100);
	uint16_t h = (uint16_t)(sensor_value_to_double(&bme280_val[1]) * 100);
	uint32_t p1 = (uint32_t)(sensor_value_to_double(&bme280_val[2]) * 1000);
	uint16_t p = (uint16_t)(p1 -50000);
	int32_t x = (uint32_t)(sensor_value_to_double(&lis2dh_val[0]) * 1000);
	int32_t y = (uint32_t)(sensor_value_to_double(&lis2dh_val[1]) * 1000);
	int32_t z = (uint32_t)(sensor_value_to_double(&lis2dh_val[2]) * 1000);
	
	mfg_data[3] = ((t)>>8);
	mfg_data[4] = ((t) & 0xFF);
	mfg_data[5] = ((h)>>8);
	mfg_data[6] = ((h) & 0xFF);
	mfg_data[7] = ((p)>>8);
	mfg_data[8] = ((p) & 0xFF);
	mfg_data[9] = ((x)>>8);
	mfg_data[10] = ((x) & 0xFF);
	mfg_data[11] = ((y)>>8);
	mfg_data[12] = ((y) & 0xFF);
	mfg_data[13] = ((z)>>8);
	mfg_data[14] = ((z) & 0xFF);
	mfg_data[15] = (int8_t)tx_power;
	mfg_data[16] = (packet_counter>>8);
	mfg_data[17] = (packet_counter&0xFF);
   	printk("%d	X: %d, Y: %d, Z: %d \n", k_uptime_get_32(), x, y, z);
	if (packet_counter%10 == 0){
		flash_green(); //Flash Green every 10 packets
	}
	
}

static void sensor_init(void)
{
	bme280_dev = device_get_binding("BME280");

	if (bme280_dev == NULL) {
		printk("Could not get BME280 device\n");
		flash_red();
		return;
	}

	lis2dh_dev = device_get_binding(DT_INST_0_ST_LIS2DH_LABEL);

	if (lis2dh_dev == NULL) {
		printf("Could not get %s device\n",
		       DT_INST_0_ST_LIS2DH_LABEL);
		flash_red();
		return;
	}

#if CONFIG_LIS2DH_TRIGGER
	{
		struct sensor_trigger trig;
		int rc;

		trig.type = SENSOR_TRIG_DATA_READY;
		trig.chan = SENSOR_CHAN_ACCEL_XYZ;

		if (IS_ENABLED(CONFIG_LIS2DH_ODR_RUNTIME)) {
			struct sensor_value odr = {
				.val1 = 1,
			};

			rc = sensor_attr_set(lis2dh_dev, trig.chan,
					     SENSOR_ATTR_SAMPLING_FREQUENCY,
					     &odr);
			if (rc != 0) {
				printf("Failed to set odr: %d\n", rc);
				return;
			}
		}

		rc = sensor_trigger_set(lis2dh_dev, &trig, trigger_handler);
		if (rc != 0) {
			printf("Failed to set trigger: %d\n", rc);
			return;
		}
		printk("Trigger Set\n");
	}
#endif 
}

void main(void)
{
	int err;

	led_init();
	sensor_init();
	tx_power = get_tx_power();

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
	
	printf("TX Power set to: %d \n", tx_power);


	while (true) {
		++packet_counter;
		sensor_handler();
		
		// update adv data
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		k_sleep(K_SECONDS(SLEEP_TIME));
	}
}
