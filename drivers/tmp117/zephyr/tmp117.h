#ifndef __TMP117_DRIVER_H__
#define __TMP117_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel.h>
#include <device.h>
#include <sys/util.h>
#include <stdint.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <string.h>

struct tmp117_sample {
	int32_t raw_sample;
};

union tmp117_bus_cfg {
	uint16_t i2c_slv_addr;
};

struct tmp117_config {
	const char *bus_name;
	int (*bus_init)(const struct device *dev);
	const union tmp117_bus_cfg bus_cfg;
};

struct tmp117_transfer_function {
	int (*read_reg)(const struct device *dev, uint8_t reg,
			uint16_t *val);
	int (*write_reg)(const struct device *dev, uint8_t reg,
			 uint16_t val);
};

struct tmp117_data {
	const struct device *bus;
	const struct tmp117_transfer_function *hw_tf;
	uint8_t sample_rate;
	uint8_t resolution;
	uint8_t scale;
	uint8_t dsp;
	uint8_t mode;
};

int tmp117_i2c_init(const struct device *dev);

__subsystem struct tmp117_driver_api {
	int (*soft_reset)(const struct device *dev);
	int (*sleep)(const struct device *dev);
	int (*get_sample)(const struct device *dev, struct tmp117_sample *val);
};

__syscall int tmp117_soft_reset(const struct device *dev);

static inline int z_impl_tmp117_soft_reset(const struct device *dev)
{
	const struct tmp117_driver_api *api =
		(const struct tmp117_driver_api *)dev->api;

	return api->soft_reset(dev);
}

__syscall int tmp117_sleep(const struct device *dev);

static inline int z_impl_tmp117_sleep(const struct device *dev)
{
	const struct tmp117_driver_api *api =
		(const struct tmp117_driver_api *)dev->api;

	return api->sleep(dev);
}

__syscall int tmp117_get_sample(const struct device *dev, struct tmp117_sample *val);

static inline int z_impl_tmp117_get_sample(const struct device *dev,
					    						struct tmp117_sample *val)
{
	const struct tmp117_driver_api *api =
		(const struct tmp117_driver_api *)dev->api;

	return api->get_sample(dev, val);
}


#ifdef __cplusplus
}
#endif

#include <syscalls/tmp117.h>

#endif /* __TMP117_DRIVER_H__ */