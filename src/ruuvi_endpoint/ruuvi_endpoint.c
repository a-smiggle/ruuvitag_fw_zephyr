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
static int16_t temperature = 0, x = 0, y= 0, z = 0, vbatt = 0;
static uint16_t humidity = 0, pressure = 0, acceleration_events = 0;
static int64_t battery_check = 0, time = 0;
static uint32_t packet_counter = 0;
static bool tx_pwr_rx = false;
static int8_t tx_pwr = 0;

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

static void ruuvi_raw_v2_encode(uint8_t *data){
    data[0]     =   0x99;
    data[1]     =   0x04;
    data[2]     =   PACKET_MODE;
    data[3] 	= 	((temperature)>>8);
	data[4] 	= 	((temperature) & 0xFF);
	data[5] 	= 	((humidity)>>8);
	data[6] 	= 	((humidity) & 0xFF);
	data[7] 	= 	((pressure)>>8);
	data[8] 	= 	((pressure) & 0xFF);
	data[9] 	= 	((x)>>8);
	data[10] 	= 	((x) & 0xFF);
	data[11] 	= 	((y)>>8);
	data[12] 	= 	((y) & 0xFF);
	data[13] 	= 	((z)>>8);
	data[14] 	= 	((z) & 0xFF);
	vbatt 		-= 	1600; //Bias by 1600 mV
    vbatt 		<<= 5;   //Shift by 5 to fit TX PWR in
    data[15] 	= 	(vbatt)>>8;
    data[16] 	= 	(vbatt)&0xFF; //Zeroes tx-pwr bits
    if (!tx_pwr_rx){
        get_tx_power();
    }
    /* Prepare TX power for packet */
	tx_pwr += 40;
    tx_pwr /= 2;
    data[16] 	|= 	((uint8_t)tx_pwr)&0x1F; //5 lowest bits for TX pwr
	data[17] 	= 	acceleration_events % 256;
	data[18] 	= 	(packet_counter>>8);
	data[19] 	= 	(packet_counter&0xFF);
    if(!device_mac_rx){
        get_mac(&device_mac);
        device_mac_rx = true;
    }
    data[20] = device_mac.mac[5]; 
	data[21] = device_mac.mac[4];
	data[22] = device_mac.mac[3];
	data[22] = device_mac.mac[2];
	data[23] = device_mac.mac[1]; 
	data[24] = device_mac.mac[0];
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
    ruuvi_raw_v2_encode(data);
}