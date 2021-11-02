#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
#include <tmp117_handler.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(tmp117_handler, CONFIG_RUUVITAG_LOG_LEVEL);

const struct device *tmp117;

struct sensor_value temp_value;

void tmp117_fetch(void)
{
    sensor_sample_fetch(tmp117);
}

int16_t tmp117_get_temp(void)
{
	sensor_channel_get(tmp117, SENSOR_CHAN_AMBIENT_TEMP, &temp_value);
	return (int16_t)(temp_value.val1*100 + temp_value.val2/10000);
}

bool init_tmp117(void)
{
	tmp117 = device_get_binding(DT_LABEL(DT_INST(0, ti_tmp117)));
	if (tmp117 == NULL) 
	{
		return false;
	} 
	else 
	{
		return true;
	}
}
