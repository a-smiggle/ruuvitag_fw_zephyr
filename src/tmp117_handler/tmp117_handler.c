#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include "tmp117.h"
#include <tmp117_handler.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(tmp117_handler, CONFIG_RUUVITAG_LOG_LEVEL);

const struct device *tmp117;

struct tmp117_sample tmp117_sample;

void tmp117_fetch(void)
{
    tmp117_get_sample(tmp117, &tmp117_sample);
}

int16_t tmp117_get_temp(void)
{
	return tmp117_sample.raw_sample;
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
