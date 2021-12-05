/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <stdio.h>

#include "ruuvi.h"
#include "board_info.h"
#include "ruuvi_endpoint.h"

static bool device_mac_rx = false;
mac_address_bin_t device_mac;
static int8_t tx_pwr = RUUVI_TX_POWER;

/*
 * Needs ENV data to be more accurate.
 * Adjusted from: 
 * https://github.com/ruuvi/ruuvitag_fw/blob/master/libraries/ruuvi_sensor_formats/ruuvi_endpoints.c
 * Data found here is not as accurate as found in on current ruuvi fw.
 */
void ruuvi_raw_v2_encode(uint8_t *data, sensor_data_t sensor_data, uint16_t acc_events ){
    static uint32_t packet_counter ;
    data[0]    =   RUUVI_RAWv2;
    int32_t t 	= 	sensor_data.temperature * 2;
    data[1] 	= 	((t) >> 8);
    data[2] 	= 	((t) & 0xFF);
    uint32_t h 	= 	sensor_data.humidity * 400 / 1024;
    data[3] 	= 	((h)>>8);
    data[4] 	= 	((h) & 0xFF);
    data[5] 	= 	((sensor_data.pressure)>>8);
    data[6] 	= 	((sensor_data.pressure) & 0xFF);
    data[7] 	= 	((sensor_data.x)>>8);
    data[8] 	= 	((sensor_data.x) & 0xFF);
    data[9] 	= 	((sensor_data.y)>>8);
    data[10] 	= 	((sensor_data.y) & 0xFF);
    data[11] 	= 	((sensor_data.z)>>8);
    data[12] 	= 	((sensor_data.z) & 0xFF);
    int16_t vbatt = sensor_data.vbatt;
    vbatt 	+= 0	; //Bias by 1600 mV
    vbatt 				<<= 5;   //Shift by 5 to fit TX PWR in
    data[13] 	= 	(vbatt)>>8;
    data[14] 	= 	(vbatt)&0xFF; //Zeroes tx-pwr bits
	int8_t tx = tx_pwr;
    /* Prepare TX power for packet */
    tx 		+= 40;
    tx 		/= 2;
    data[14] 	|= 	((uint8_t)tx)&0x1F; //5 lowest bits for TX pwr
    data[15] 	= 	acc_events % 256;
    data[16] 	= 	(packet_counter>>8);
    data[17] 	= 	(packet_counter&0xFF);
    if(!device_mac_rx){
        get_mac(&device_mac);
        device_mac_rx = true;
    }
    data[18] 	= device_mac.mac[5]; 
    data[19] 	= device_mac.mac[4];
    data[20] 	= device_mac.mac[3];
    data[21] 	= device_mac.mac[2];
    data[22] 	= device_mac.mac[1]; 
    data[23] 	= device_mac.mac[0];
    ++packet_counter;
}
