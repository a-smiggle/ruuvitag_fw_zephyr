
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <tmp117_handler.h>
#include <pm/pm.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(tmp117, CONFIG_RUUVITAG_LOG_LEVEL);

const struct device *tmp117;
struct sensor_value temp_value;
int ret;


void tmp117_fetch(void)
{
	ret = sensor_sample_fetch(tmp117);
	return;
}

int16_t tmp117_get_temp(void){
	struct sensor_value temp;
	sensor_channel_get(tmp117, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	//LOG_INF("temp: %d.%06d, ", temp.val1, temp.val2);
	return (int16_t)(temp.val1*100 + temp.val2/10000);
}

bool init_tmp117(void){
	tmp117 = device_get_binding(DT_LABEL(DT_INST(0, ti_tmp116)));
	if (tmp117 == NULL) {
		return false;
	} else {
		return true;
	}
}
