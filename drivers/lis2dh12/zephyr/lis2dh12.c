#define DT_DRV_COMPAT st_lis2dh12

#include <init.h>
#include <kernel.h>
#include <sys/byteorder.h>
#include <sys/__assert.h>
#include <sys/util.h>
#include <zephyr/types.h>


#include <stdint.h>
#include <string.h>

#include <device.h>
#include <drivers/spi.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(lis2dh12, CONFIG_LIS2DH12_LOG_LEVEL);
#include "lis2dh12.h"

struct lis2dh12_data lis2dh12_data = {
	.config.bus_name = DT_INST_BUS_LABEL(0)
};

static float rawToCelsius(int16_t raw_temp)
{
	switch(lis2dh12_data.config.mode)
	{
		case LIS2DH12_LP_8bit:
		return lis2dh12_from_lsb_lp_to_celsius(raw_temp);

		case LIS2DH12_NM_10bit:
		return lis2dh12_from_lsb_nm_to_celsius(raw_temp);

		case LIS2DH12_HR_12bit:
		return lis2dh12_from_lsb_hr_to_celsius(raw_temp);

		default:
		break;
	}
		
	return 0;
}


static float rawToMg(int16_t raw_acceleration)
{
	switch(lis2dh12_data.config.scale)
	{
		case LIS2DH12_2g:
		switch(lis2dh12_data.config.mode)
		{
			case LIS2DH12_LP_8bit:
			return lis2dh12_from_fs2_lp_to_mg(raw_acceleration);

			case LIS2DH12_NM_10bit:
			return lis2dh12_from_fs2_nm_to_mg(raw_acceleration);

			case LIS2DH12_HR_12bit:
			return lis2dh12_from_fs2_hr_to_mg(raw_acceleration);

			default:
			break;
		}
		break;

		case LIS2DH12_4g:
		switch(lis2dh12_data.config.mode)
		{
			case LIS2DH12_LP_8bit:
			return lis2dh12_from_fs4_lp_to_mg(raw_acceleration);

			case LIS2DH12_NM_10bit:
			return lis2dh12_from_fs4_nm_to_mg(raw_acceleration);

			case LIS2DH12_HR_12bit:
			return lis2dh12_from_fs4_hr_to_mg(raw_acceleration);
		}
		break;

		case LIS2DH12_8g:
		switch(lis2dh12_data.config.mode)
		{
			case LIS2DH12_LP_8bit:
			return lis2dh12_from_fs8_lp_to_mg(raw_acceleration);

			case LIS2DH12_NM_10bit:
			return lis2dh12_from_fs8_nm_to_mg(raw_acceleration);

			case LIS2DH12_HR_12bit:
			return lis2dh12_from_fs8_hr_to_mg(raw_acceleration);

			default:
			break;
		}
		break;

		case LIS2DH12_16g:
		switch(lis2dh12_data.config.mode)
		{
			case LIS2DH12_LP_8bit:
			return lis2dh12_from_fs16_lp_to_mg(raw_acceleration);

			case LIS2DH12_NM_10bit:
			return lis2dh12_from_fs16_nm_to_mg(raw_acceleration);

			case LIS2DH12_HR_12bit:
			return lis2dh12_from_fs16_hr_to_mg(raw_acceleration);

			default:
			break;
		}
		break;

		default:
		break;
	}
	return 0;
}

static int get_temp_sample_lis2dh12(const struct device *dev, struct lis2dh12_sample *val)
{
	struct lis2dh12_data *lis2dh12 = dev->data;
 	int16_t raw;
    if (lis2dh12_temperature_raw_get(lis2dh12->ctx, &raw)<0){
		return -EIO;
	}
	val->temp = raw;
	val->temp_c = (int16_t) rawToCelsius(raw);
	return 0;
}

static int get_acc_sample_lis2dh12(const struct device *dev, struct lis2dh12_sample *val)
{
	struct lis2dh12_data *lis2dh12 = dev->data;
 	int16_t raw[3];
    if (lis2dh12_acceleration_raw_get(lis2dh12->ctx, raw)<0){
		return -EIO;
	}
	val->acc[0] = raw[0];
	val->acc[1] = raw[1];
	val->acc[2] = raw[2];
	val->acc_mg[0] = (int16_t) rawToMg(raw[0]);
	val->acc_mg[1] = (int16_t) rawToMg(raw[1]);
	val->acc_mg[2] = (int16_t) rawToMg(raw[2]);
	return 0;
}

static int get_sample_lis2dh12(const struct device *dev, struct lis2dh12_sample *val){
	if(get_acc_sample_lis2dh12(dev, val) < 0){
		return -EIO;
	}

	if(lis2dh12_data.config.temp_en == LIS2DH12_TEMP_ENABLE){
		if( get_temp_sample_lis2dh12(dev, val)< 0){
			return -EIO;
		}
	}

	return 0;
}

static const struct lis2dh12_driver_api lis2dh12_driver_api = {
	.get_temp_sample 	= get_temp_sample_lis2dh12,
	.get_acc_sample 	= get_acc_sample_lis2dh12,
	.get_sample 		= get_sample_lis2dh12
};

static int lis2dh12_config()
{
#if CONFIG_LIS2DH12_OPER_MODE_HIGH_RES
	lis2dh12_data.config.mode = LIS2DH12_HR_12bit;
#elif CONFIG_LIS2DH12_OPER_MODE_NORMAL
	lis2dh12_data.config.mode = LIS2DH12_NM_10bit;
#elif CONFIG_LIS2DH12_OPER_MODE_LOW_POWER
	lis2dh12_data.config.mode = LIS2DH12_LP_8bit;
#else
	lis2dh12_data.config.mode = LIS2DH12_NM_10bit;
#endif

#if CONFIG_LIS2DH12_ODR_1
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_1Hz;
#elif CONFIG_LIS2DH12_ODR_10
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_10Hz;
#elif CONFIG_LIS2DH12_ODR_25
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_25Hz;
#elif CONFIG_LIS2DH12_ODR_50
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_50Hz;
#elif CONFIG_LIS2DH12_ODR_100
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_100Hz;
#elif CONFIG_LIS2DH12_ODR_200
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_200Hz;
#elif CONFIG_LIS2DH12_ODR_400
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_400Hz;
#else
	lis2dh12_data.config.samplerate = LIS2DH12_ODR_1Hz;
#endif

#if CONFIG_LIS2DH12_ACCEL_RANGE_2G
	lis2dh12_data.config.scale = LIS2DH12_2g;
#elif CONFIG_LIS2DH12_ACCEL_RANGE_4G
	lis2dh12_data.config.scale = LIS2DH12_4g;
#elif CONFIG_LIS2DH12_ACCEL_RANGE_8G
	lis2dh12_data.config.scale = LIS2DH12_8g;
#elif CONFIG_LIS2DH12_ACCEL_RANGE_16G
	lis2dh12_data.config.scale = LIS2DH12_16g;
#else
	lis2dh12_data.config.scale = LIS2DH12_2g;
#endif

#if CONFIG_LIS2DH12_TEMP_EN
	lis2dh12_data.config.temp_en = LIS2DH12_TEMP_ENABLE;
#else
	lis2dh12_data.config.temp_en = LIS2DH12_TEMP_DISABLE;
#endif

#if LIS2DH12_FIFO_ENABLED
#if CONFIG_LIS2DH12_BYPASS_MODE
	lis2dh12_data.config.fifo_mode = LIS2DH12_BYPASS_MODE;
#elif CONFIG_LIS2DH12_FIFO_MODE
	lis2dh12_data.config.fifo_mode = LIS2DH12_FIFO_MODE;
#elif CONFIG_LIS2DH12_DYNAMIC_STREAM_MODE
	lis2dh12_data.config.fifo_mode = LIS2DH12_DYNAMIC_STREAM_MODE;
#elif CONFIG_LIS2DH12_STREAM_TO_FIFO_MODE
	lis2dh12_data.config.fifo_mode = LIS2DH12_STREAM_TO_FIFO_MODE;
#else
	lis2dh12_data.config.fifo_mode = LIS2DH12_BYPASS_MODE;
#endif

#if CONFIG_LIS2DH_FIFO_WM
	lis2dh12_data.config.fifo_wm = CONFIG_LIS2DH_FIFO_WM;
#endif
#endif
	return 0;
}

static int lis2dh12_init_interface(const struct device *dev)
{
	struct lis2dh12_data *lis2dh12 = dev->data;

	lis2dh12->bus = device_get_binding(lis2dh12->config.bus_name);
	if (!lis2dh12->bus) {
		LOG_DBG("master bus not found: %s", lis2dh12->config.bus_name);
		return -EINVAL;
	}

	lis2dh12_spi_init(dev);
	return 0;
}

static int lis2dh12_init(const struct device *dev)
{
	
	struct lis2dh12_data *lis2dh12 = dev->data;
	uint8_t wai;

	if (lis2dh12_init_interface(dev)) {
		return -EINVAL;
	}

	/* check chip ID */
	if (lis2dh12_device_id_get(lis2dh12->ctx, &wai) < 0) {
		return -EIO;
	}

	if (wai != LIS2DH12_ID) {
		LOG_ERR("Invalid chip ID: %02x", wai);
		return -EINVAL;
	}

	if (lis2dh12_block_data_update_set(lis2dh12->ctx, PROPERTY_ENABLE) < 0) {
		return -EIO;
	}

	lis2dh12_config();

	if (lis2dh12_operating_mode_set(lis2dh12->ctx, lis2dh12_data.config.mode)) {
		return -EIO;
	}

	/* set default odr */
	if (lis2dh12_data_rate_set(lis2dh12->ctx, lis2dh12_data.config.samplerate) < 0) {
		return -EIO;
	}

	/* set default full scale for acc */
	if (lis2dh12_full_scale_set(lis2dh12->ctx, lis2dh12_data.config.scale) < 0) {
		return -EIO;
	}

	if(lis2dh12_data.config.temp_en == LIS2DH12_TEMP_ENABLE){
		/* Enable temp readings */
		if (lis2dh12_temperature_meas_set(lis2dh12->ctx, lis2dh12_data.config.temp_en) < 0) {
			return -EIO;
		}
	}

	return 0;
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_lis2dh12_get_temp_sample(const struct device *dev, struct lis2dh12_sample *val)
{
	Z_OOPS(Z_SYSCALL_DRIVER_LIS2DH12(dev, get_temp_sample));
	Z_OOPS(Z_SYSCALL_MEMORY_WRITE(val, sizeof(struct lis2dh12_sample)));
	z_impl_lis2dh12_get_temp_sample((const struct device *)dev, (struct lis2dh12_sample *)val);
}
#include <syscalls/lis2dh12_get_temp_sample_mrsh.c>

static inline void z_vrfy_lis2dh12_get_acc_sample(const struct device *dev, struct lis2dh12_sample *val)
{
	Z_OOPS(Z_SYSCALL_DRIVER_LIS2DH12(dev, get_acc_sample));
	Z_OOPS(Z_SYSCALL_MEMORY_WRITE(val, sizeof(struct lis2dh12_sample)));
	z_impl_lis2dh12_get_acc_sample((const struct device *)dev, (struct lis2dh12_sample *)val);
}
#include <syscalls/lis2dh12_get_acc_sample_mrsh.c>

static inline void z_vrfy_lis2dh12_get_sample(const struct device *dev, struct lis2dh12_sample *val)
{
	Z_OOPS(Z_SYSCALL_DRIVER_LIS2DH12(dev, get_sample));
	Z_OOPS(Z_SYSCALL_MEMORY_WRITE(val, sizeof(struct lis2dh12_sample)));
	z_impl_lis2dh12_get_sample((const struct device *)dev, (struct lis2dh12_sample *)val);
}
#include <syscalls/lis2dh12_get_sample_mrsh.c>
#endif /* CONFIG_USERSPACE */


DEVICE_DT_INST_DEFINE(0, lis2dh12_init, NULL,
	     &lis2dh12_data, NULL, POST_KERNEL,
	     CONFIG_LIS2DH12_INIT_PRIORITY, &lis2dh12_driver_api);
