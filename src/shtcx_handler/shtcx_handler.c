/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include "shtcx_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(shtcx_handler, CONFIG_RUUVITAG_LOG_LEVEL);

#define SHTCX DT_INST(0, sensirion_shtcx)

#if DT_NODE_HAS_STATUS(SHTCX, okay)
#define SHTCX_LABEL DT_LABEL(SHTCX)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,shtcx"
#define SHTCX_LABEL "<none>"
#endif

// Required to access raw data without needing another driver.
struct shtcx_data {
	uint16_t temp;
	uint16_t humidity;
} __packed __aligned(2);


const struct device *shtcx;

void shtcx_fetch(void)
{
	sensor_sample_fetch(shtcx);
	return;
}

int32_t shtcx_get_temp(void){
	struct shtcx_data *data = shtcx->data;
	return data->temp;
}


uint32_t shtcx_get_humidity(void){
	struct shtcx_data *data = shtcx->data;
	return data->humidity;
}

bool init_shtcx(void){
	shtcx = device_get_binding(DT_LABEL(DT_INST(0, sensirion_shtcx)));
	if (shtcx == NULL) {
		return false;
	} else {
		return true;
	}
}
