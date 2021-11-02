/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <string.h>
#include <drivers/adc.h>
#include <hal/nrf_saadc.h>

#include "battery_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(battery_handler, CONFIG_RUUVITAG_LOG_LEVEL);

#define ADC_DEVICE_NAME			DT_LABEL(DT_INST(0, nordic_nrf_saadc))
#define ADC_RESOLUTION			10
#define ADC_GAIN				ADC_GAIN_1_6
#define ADC_REFERENCE			ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME	ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
#define ADC_1ST_CHANNEL_ID		0
#define ADC_1ST_CHANNEL_INPUT	NRF_SAADC_INPUT_AIN1
#define ADC_2ND_CHANNEL_ID	2
#define ADC_2ND_CHANNEL_INPUT	NRF_SAADC_INPUT_AIN2

const struct device *adc_dev;
static int16_t adc_buffer[6];

static const struct adc_channel_cfg m_1st_channel_cfg = {
	.gain             = ADC_GAIN,
	.reference        = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id       = ADC_1ST_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
	.input_positive   = ADC_1ST_CHANNEL_INPUT,
#endif
};

static const struct adc_channel_cfg m_2nd_channel_cfg = {
	.gain             = ADC_GAIN,
	.reference        = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id       = ADC_2ND_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
	.input_positive   = ADC_2ND_CHANNEL_INPUT,
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

	static const struct adc_sequence sequence2 = {
		.channels    = BIT(ADC_2ND_CHANNEL_ID),
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
	int16_t mv = (int16_t)(adc_buffer[0] * 3600 / mp);
	/*
	 * Read from second channel used to prevent value errors
	 * from BME280
	 */
	adc_read(adc_dev, &sequence2);

	return mv;
}

bool init_adc(void){
	int ret;
	adc_dev = device_get_binding(ADC_DEVICE_NAME);
	ret = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
	if(ret){
		LOG_ERR("Setting up of the first channel failed with code %d", ret);
	}
#if defined(ADC_2ND_CHANNEL_ID)
	ret = adc_channel_setup(adc_dev, &m_2nd_channel_cfg);
	if(ret){
		LOG_ERR("Setting up of the second channel failed with code %d", ret);
	}
#endif
	(void)memset(adc_buffer, 0, sizeof(adc_buffer));
	if (!adc_dev) {
		return false;
	}
	else{
		return true;
	}
}