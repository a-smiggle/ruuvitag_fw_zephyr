/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include "lis2dh12_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(lis2dh12, CONFIG_RUUVITAG_LOG_LEVEL);

const struct device *lis2dh12;
struct sensor_value acc[3];

void lis2dh12_fetch(void)
{
	sensor_sample_fetch(lis2dh12);

	sensor_channel_get(lis2dh12,
					SENSOR_CHAN_ACCEL_XYZ,
					acc);
	//LOG_INF("x: %d.%06d, ", acc[0].val1, acc[0].val2);
	//LOG_INF("y: %d.%06d, ", acc[1].val1, acc[1].val2);
	//LOG_INF("z: %d.%06d\n", acc[2].val1, acc[2].val2);
}

int16_t lis2dh12_get(int axis){
	if (axis == 0){
		return (int16_t)(sensor_value_to_double(&acc[axis])* 1000);
	}
	else if (axis == 1){
		return (int16_t)(sensor_value_to_double(&acc[axis]) * 1000);
	}
	else if (axis == 2){
		return (int16_t)(sensor_value_to_double(&acc[axis]) * 1000);
	}
	else{
		return 0;
	}
}

bool init_lis2dh12(void){
	lis2dh12 = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh12)));

	if (lis2dh12 == NULL) {
		return false;
	} else {
		return true;
	}
}
