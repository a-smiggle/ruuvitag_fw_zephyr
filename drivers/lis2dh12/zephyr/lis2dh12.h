#define DT_DRV_COMPAT st_lis2dh12

#ifndef __LIS2DH12_DRIVER_H__
#define __LIS2DH12_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif




#include <kernel.h>
#include <device.h>
#include <sys/util.h>
#include <stdint.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <string.h>

#include "lis2dh12_reg.h"

struct lis2dh12_sample {
	int16_t acc[3];
	int16_t acc_mg[3];
	int16_t temp;
	int16_t temp_c;
};

struct lis2dh12_device_config {
	const char *bus_name;
	lis2dh12_op_md_t mode; //!< Resolution, bits. 8, 10, or 12.
    lis2dh12_fs_t scale;         //!< Scale, gravities. 2, 4, 8 or 16.
    lis2dh12_odr_t samplerate;   //!< Sample rate, 1 ... 200, or custom values for higher.
	lis2dh12_temp_en_t temp_en;
	lis2dh12_fm_t fifo_mode;
	uint8_t fifo_wm;
};

struct lis2dh12_data {
	const struct device *bus;
	struct lis2dh12_device_config config;
	stmdev_ctx_t *ctx;

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)
	struct spi_cs_control cs_ctrl;
#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */
};


int lis2dh12_spi_init(const struct device *dev);


__subsystem struct lis2dh12_driver_api {
	int (*get_temp_sample)(const struct device *dev, struct lis2dh12_sample *val);
	int (*get_acc_sample)(const struct device *dev, struct lis2dh12_sample *val);
	int (*get_sample)(const struct device *dev, struct lis2dh12_sample *val);
};

__syscall int lis2dh12_get_temp_sample(const struct device *dev, struct lis2dh12_sample *val);

static inline int z_impl_lis2dh12_get_temp_sample(const struct device *dev,
					    						struct lis2dh12_sample *val)
{
	const struct lis2dh12_driver_api *api =
		(const struct lis2dh12_driver_api *)dev->api;

	return api->get_temp_sample(dev, val);
}

__syscall int lis2dh12_get_acc_sample(const struct device *dev, struct lis2dh12_sample *val);

static inline int z_impl_lis2dh12_get_acc_sample(const struct device *dev,
					    						struct lis2dh12_sample *val)
{
	const struct lis2dh12_driver_api *api =
		(const struct lis2dh12_driver_api *)dev->api;

	return api->get_acc_sample(dev, val);
}

__syscall int lis2dh12_get_sample(const struct device *dev, struct lis2dh12_sample *val);

static inline int z_impl_lis2dh12_get_sample(const struct device *dev,
					    						struct lis2dh12_sample *val)
{
	const struct lis2dh12_driver_api *api =
		(const struct lis2dh12_driver_api *)dev->api;

	return api->get_sample(dev, val);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/lis2dh12.h>

#endif /* __SENSOR_LIS2DH__ */
