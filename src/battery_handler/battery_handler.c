#include <zephyr.h>
#include <string.h>
#include <drivers/adc.h>
#include <hal/nrf_saadc.h>
#include <power/power.h>

#include "battery_handler.h"

#define ADC_DEVICE_NAME			DT_LABEL(DT_INST(0, nordic_nrf_saadc))
#define ADC_RESOLUTION			10
#define ADC_GAIN				ADC_GAIN_1_6
#define ADC_REFERENCE			ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME	ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
#define ADC_1ST_CHANNEL_ID		0
#define ADC_1ST_CHANNEL_INPUT	NRF_SAADC_INPUT_AIN1

static struct device *adc_dev;
static int16_t adc_buffer[1];

static const struct adc_channel_cfg m_1st_channel_cfg = {
	.gain             = ADC_GAIN,
	.reference        = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id       = ADC_1ST_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
	.input_positive   = ADC_1ST_CHANNEL_INPUT,
#endif
};

int16_t get_battery_level(void)
{	
	static const struct adc_sequence sequence = {
		.channels    = BIT(ADC_1ST_CHANNEL_ID),
		.buffer      = adc_buffer,
		.buffer_size = sizeof(adc_buffer),
		.resolution  = ADC_RESOLUTION,
	};
	
	adc_read(adc_dev, &sequence);
	uint16_t mp = 256;
	switch(ADC_RESOLUTION)
	{
		default :
		case 8 :
			mp = 256;
			break;
		case 10 :
			mp = 1024;
			break;
		case 12 :
			mp = 4096;
			break;
		case 14 :
			mp = 16384;
			break;
	}
	return (int16_t)(adc_buffer[0] * 3600 / mp);
}

bool init_adc(void){
	adc_dev = device_get_binding(ADC_DEVICE_NAME);
	adc_channel_setup(adc_dev, &m_1st_channel_cfg);
	(void)memset(adc_buffer, 0, sizeof(adc_buffer));
	if (!adc_dev) {
		return false;
	}
	else{
		return true;
	}
}