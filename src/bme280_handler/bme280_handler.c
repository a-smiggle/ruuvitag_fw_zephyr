#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include "bme280_handler.h"
#include <power/power.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(bme280, CONFIG_RUUVITAG_LOG_LEVEL);

#define BME280 DT_INST(0, bosch_bme280)

#if DT_NODE_HAS_STATUS(BME280, okay)
#define BME280_LABEL DT_LABEL(BME280)
#else
#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
#define BME280_LABEL "<none>"
#endif

const struct device *bme280;

int bme280_power_state(bool state){
	int rc;
	if(!state){
		rc = device_set_power_state(bme280, DEVICE_PM_LOW_POWER_STATE, NULL, NULL);
	}
	else{
		rc = device_set_power_state(bme280, DEVICE_PM_ACTIVE_STATE, NULL, NULL);
	}
	return rc;
}

void bme280_fetch(void)
{
	sensor_sample_fetch(bme280);
	return;
}

int16_t bme280_get_temp(void){
	struct sensor_value temp;
	sensor_channel_get(bme280, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	//LOG_INF("temp: %d.%06d, ", temp.val1, temp.val2);
	return (int16_t)(temp.val1*100 + temp.val2/10000);
}

uint16_t bme280_get_press(void){
	struct sensor_value press;
	sensor_channel_get(bme280, SENSOR_CHAN_PRESS, &press);
	uint32_t p = (uint32_t)(press.val1*1000 + press.val2/10000);
	//LOG_INF("press: %d.%06d, ", press.val1, press.val2);
	return (uint16_t)(p - 50000);
}

uint16_t bme280_get_humidity(void){
	struct sensor_value humidity;
	sensor_channel_get(bme280, SENSOR_CHAN_HUMIDITY, &humidity);
	//LOG_INF("humidity: %d.%06d\n", humidity.val1, humidity.val2);
	return (uint16_t)(humidity.val1*100 + humidity.val2/10000);
}

bool init_bme280(void){
	bme280 = device_get_binding(BME280_LABEL);
	if (bme280 == NULL) {
		return false;
	} else {
		return true;
	}
}
