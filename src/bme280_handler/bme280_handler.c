/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#ifdef CONFIG_BME280

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>

#include "bme280_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(bme280_handler, CONFIG_RUUVITAG_LOG_LEVEL);

#define BME280 DT_INST(0, bosch_bme280)

#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif

// Required to access raw data without needing another driver.
struct bme280_data {
	/* Compensation parameters. */
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
	uint8_t dig_h1;
	int16_t dig_h2;
	uint8_t dig_h3;
	int16_t dig_h4;
	int16_t dig_h5;
	int8_t dig_h6;

	/* Compensated values. */
	int32_t comp_temp;
	uint32_t comp_press;
	uint32_t comp_humidity;

	/* Carryover between temperature and pressure/humidity compensation. */
	int32_t t_fine;

	uint8_t chip_id;

#ifdef CONFIG_PM_DEVICE
	enum pm_device_state pm_state; /* Current power state */
#endif
};

const struct device *bme280;

void bme280_fetch(void)
{
	sensor_sample_fetch(bme280);
	return;
}

int32_t bme280_get_temp(void){
	struct bme280_data *data = bme280->data;
	return data->comp_temp;
}

uint16_t bme280_get_press(void){
	struct bme280_data *data = bme280->data;
	return (uint16_t)(data->comp_press + 50000);
}

uint32_t bme280_get_humidity(void){
	struct bme280_data *data = bme280->data;
	return data->comp_humidity;
}

bool init_bme280(void){
	bme280 = device_get_binding(DT_LABEL(DT_INST(0, bosch_bme280)));
	if (bme280 == NULL) {
		return false;
	} else {
		return true;
	}
}

#endif