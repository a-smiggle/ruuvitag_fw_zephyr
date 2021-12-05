#include <zephyr.h>
#include <device.h>
#include <devicetree.h>


#include <shtcx_handler.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(shtcx_handler, CONFIG_RUUVITAG_LOG_LEVEL);

#define SHTCX DT_INST(0, sensirion_shtcx)


const struct device *shtcx;


void shtcx_fetch(void)
{
	sensor_sample_fetch(shtcx);
	return;
}





int16_t shtcx_get_temp(void)
{
	
}

