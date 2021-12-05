/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#ifdef CONFIG_DPS310

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>

#include "dps310_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(dps310_handler);

const struct device *dps310;

struct sensor_value temp, press;

void dps310_fetch(void)
{
	sensor_sample_fetch(dps310);
	return;
}

int32_t dps310_get_temp(void){
	sensor_channel_get(dps310, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	return temp.val1;
}

uint16_t dps310_get_press(void){
	sensor_channel_get(dps310, SENSOR_CHAN_PRESS, &press);
	return press.val1;
}


bool init_dps310(void){
	dps310 = device_get_binding(DT_LABEL(DT_INST(0, infineon_dps310)));
	if (dps310 == NULL) {
		return false;
	} else {
		return true;
	}
}

#endif