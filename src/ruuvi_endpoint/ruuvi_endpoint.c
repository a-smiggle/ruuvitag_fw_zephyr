/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <stdio.h>

#include "ruuvi.h"
#include "bme280_handler.h"
#include "lis2dh12_handler.h"
#include "battery_handler.h"
#include "board_info.h"
#include "led_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(endpoint, CONFIG_RUUVITAG_LOG_LEVEL);

static bool has_lis2dh12 = false;
static bool has_bme280 = false;
static bool has_adc = false;

static bool device_mac_rx = false;
mac_address_bin_t device_mac;
static int32_t temperature = 0;
static int16_t x = 0, y= 0, z = 0, vbatt = 0;
static uint16_t humidity = 0, pressure = 0, acceleration_events = 0;
static int64_t battery_check = 0, time = 0;
static uint32_t packet_counter = 0;
static int8_t tx_pwr = RUUVI_TX_POWER;

/*
 * Needs ENV data to be more accurate.
 * Adjusted from: 
 * https://github.com/ruuvi/ruuvitag_fw/blob/master/libraries/ruuvi_sensor_formats/ruuvi_endpoints.c
 * Data found here is not as accurate as found in on current ruuvi fw.
 */
static void ruuvi_raw_v2_encode(uint8_t *data, uint8_t offset){
	data[0 + offset]    =   RUUVI_RAWv2;
	int32_t t 			= 	temperature / 0.5;
    data[1 + offset] 	= 	((t) >> 8);
	data[2 + offset] 	= 	((t) & 0xFF);
	uint32_t h 			= 	humidity * 4;
	data[3 + offset] 	= 	((h)>>8);
	data[4 + offset] 	= 	((h) & 0xFF);
	data[5 + offset] 	= 	((pressure)>>8);
	data[6 + offset] 	= 	((pressure) & 0xFF);
	data[7 + offset] 	= 	((x)>>8);
	data[8 + offset] 	= 	((x) & 0xFF);
	data[9 + offset] 	= 	((y)>>8);
	data[10 + offset] 	= 	((y) & 0xFF);
	data[11 + offset] 	= 	((z)>>8);
	data[12 + offset] 	= 	((z) & 0xFF);
	vbatt 				-= 	1600; //Bias by 1600 mV
    vbatt 				<<= 5;   //Shift by 5 to fit TX PWR in
    data[13 + offset] 	= 	(vbatt)>>8;
    data[14 + offset] 	= 	(vbatt)&0xFF; //Zeroes tx-pwr bits
	int8_t tx = tx_pwr;
    /* Prepare TX power for packet */
	tx 				+= 40;
    tx 				/= 2;
    data[14 + offset] 	|= 	((uint8_t)tx)&0x1F; //5 lowest bits for TX pwr
	data[15 + offset] 	= 	acceleration_events % 256;
	data[16 + offset] 	= 	(packet_counter>>8);
	data[17 + offset] 	= 	(packet_counter&0xFF);
    if(!device_mac_rx){
        get_mac(&device_mac);
        device_mac_rx = true;
    }
    data[18 + offset] 	= device_mac.mac[5]; 
	data[19 + offset] 	= device_mac.mac[4];
	data[20 + offset] 	= device_mac.mac[3];
	data[21 + offset] 	= device_mac.mac[2];
	data[22 + offset] 	= device_mac.mac[1]; 
	data[23 + offset] 	= device_mac.mac[0];
    ++packet_counter;
}

static void ruuvi_update_battery(void){
    time = k_uptime_get();
    if(time - battery_check > 10000){
        battery_check 	= k_uptime_get();
        vbatt 			= get_battery_level();
        LOG_DBG("Battery: %d mV", vbatt);
	}
}

static void ruuvi_update_bme(void){
    bme280_fetch();
    temperature = 	bme280_get_temp();
    pressure 	= 	bme280_get_press();
    humidity 	= 	bme280_get_humidity();
    LOG_DBG("Temperature: %d, Pressure: %d, Humidity: %d", 
                        temperature, 
                        pressure, 
                        humidity);
}

static void ruuvi_update_lis2dh(void){
    lis2dh12_fetch();
    x = lis2dh12_get(0);
    y = lis2dh12_get(1);
    z = lis2dh12_get(2);
    LOG_DBG("X: %d, Y: %d, Z: %d", x, y, z);
}

void ruuvi_endpoint_sensor_check(void){
    has_adc = init_adc();
	if (!has_adc) {
		LOG_ERR("Failed to initialize ADC\n");
		flash_red();
	}

	has_bme280 = init_bme280();
	if (!has_bme280) {
		LOG_ERR("Failed to initialize BME280\n");
		flash_red();
	}
	
	has_lis2dh12 = init_lis2dh12();
	if (!has_lis2dh12) {
		LOG_ERR("Failed to initialize LIS2DH12\n");
		flash_red();
	}
    return;
}

void ruuvi_update_nfc_endpoint(uint8_t* data)
{
	ruuvi_raw_v2_encode(data, 0); 
}

void ruuvi_update_endpoint(uint8_t* data)
{
    if (has_adc){
        ruuvi_update_battery();
    }
    if (has_bme280){
        ruuvi_update_bme();
    }
    if (has_lis2dh12){
        ruuvi_update_lis2dh();
    }
	/*
	 * Allows for Ruuvi RAWv2 to be used as the data packet.
	 * This will become the default once correct data can be acquired.
	 */
	ruuvi_raw_v2_encode(data, RUUVI_MFG_OFFSET);
    
}