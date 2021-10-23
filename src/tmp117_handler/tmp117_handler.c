/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <tmp117_handler.h>
#include <pm/pm.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#define POWER1_NODE DT_ALIAS(power1)

#if DT_NODE_HAS_STATUS(POWER1_NODE, okay)
#define POWER1	DT_GPIO_LABEL(POWER1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(POWER1_NODE, gpios)
#endif

LOG_MODULE_REGISTER(tmp117, CONFIG_RUUVITAG_LOG_LEVEL);

const struct device *tmp117;
const struct device *power1_pin;

struct sensor_value temp_value;
int ret;


void tmp117_fetch(void)
{
	gpio_pin_set(power1_pin, PIN1, 1);
        k_sleep(K_MSEC(10u));
        
        ret = sensor_sample_fetch(tmp117);
	
        gpio_pin_set(power1_pin, PIN1, 0);
        
        return;
}

int16_t tmp117_get_temp(void){
	struct sensor_value temp;
	gpio_pin_set(power1_pin, PIN1, 1);
        k_sleep(K_MSEC(10u));
        
        sensor_channel_get(tmp117, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	
        gpio_pin_set(power1_pin, PIN1, 0);
        
        LOG_INF("temp: %d.%06d, ", temp.val1, temp.val2);
	return (int16_t)(temp.val1*100 + temp.val2/10000);
}

bool init_tmp117(void){
        power1_pin = device_get_binding(POWER1);
        gpio_pin_set(power1_pin, PIN1, 1);
        k_sleep(K_MSEC(10u));

	tmp117 = device_get_binding(DT_LABEL(DT_INST(0, ti_tmp116)));
        
        gpio_pin_set(power1_pin, PIN1, 0);

	if (tmp117 == NULL) {
		return false;

	} else {
		return true;

	}
}
