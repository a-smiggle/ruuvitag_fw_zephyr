#define DT_DRV_COMPAT ti_tmp117

#include <init.h>
#include <kernel.h>
#include <sys/byteorder.h>
#include <sys/__assert.h>
#include <sys/util.h>
#include <zephyr/types.h>

#include <stdint.h>
#include <string.h>

#include <device.h>
#include <drivers/i2c.h>

#include "tmp117.h"
#include "tmp117_reg.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(tmp117, CONFIG_TMP117_LOG_LEVEL);

struct tmp117_config tmp117_config = {
	.bus_name = DT_INST_BUS_LABEL(0),
	.bus_init = tmp117_i2c_init,
	.bus_cfg = { .i2c_slv_addr = DT_INST_REG_ADDR(0) },
};

struct tmp117_data tmp117_data;

static int sample_read(const struct device *dev)
{
	struct tmp117_data *tmp117 = dev->data;
	uint16_t cfg_reg = 0;

	if( tmp117->hw_tf->read_reg(dev, TMP117_REG_CONFIGURATION, &cfg_reg) < 0)
	{
		LOG_ERR("%s, Failed to read from CFGR register.",
			dev->name);
		return -EINVAL;
	}

	if ((cfg_reg & TMP117_CFGR_DATA_READY) == 0) {
		LOG_DBG("%s: no data ready", dev->name);
		return -EBUSY;
	}
	return 0;
}

static int get_sample_tmp117(const struct device *dev, struct tmp117_sample *val)
{
	struct tmp117_data *tmp117 = dev->data;
	uint16_t value;
	int rc = sample_read(dev);

	if (rc < 0) {
		return rc;
	}

	
	rc = tmp117->hw_tf->read_reg(dev, TMP117_REG_TEMP_RESULT, &value);
	if(rc < 0)
	{
		LOG_ERR("%s, Failed to read temperature.",
			dev->name);
		return -EINVAL;
	}

	if (value > 0x7FFFU)
    {
        val->raw_sample = (int32_t)(((int32_t) value - 0xFFFF)* TMP117_RESOLUTION) ;
    }
    else
    {
        val->raw_sample = (int32_t)(value * TMP117_RESOLUTION);
    }
	
	return rc;
}

/**
 * @brief Check the Device ID
 *
 * @param[in]   dev     Pointer to the device structure
 *
 * @retval 0 on success
 * @retval -EIO Otherwise
 */
static inline int tmp117_device_id_check(const struct device *dev)
{
	struct tmp117_data *tmp117 = dev->data;
	uint16_t id;
	if (tmp117->hw_tf->read_reg(dev, TMP117_REG_DEVICE_ID, &id) != 0) {
		LOG_ERR("%s: Failed to get Device ID register!",
			dev->name);
		LOG_ERR("%d\n", id);
		return -EIO;
	}

	if ((id != TMP117_VALUE_ID) && (id != TMP117_VALUE_ID)) {
		LOG_ERR("%s: Failed to match the device IDs!",
			dev->name);
		return -EINVAL;
	}
	LOG_INF("Got device ID: %x", id);
	return 0;
}

static int soft_reset_tmp117(const struct device *dev)
{
	struct tmp117_data *tmp117 = dev->data;
	uint16_t err_code;
	uint16_t reset = TMP117_MASK_RESET & 0xFFFF;

	err_code = tmp117->hw_tf->write_reg(dev, TMP117_REG_CONFIGURATION, reset);
	k_sleep(K_MSEC(TMP117_CC_RESET_DELAY_MS));
	return err_code;
}


static int sleep_tmp117(const struct device *dev)
{
	struct tmp117_data *tmp117 = dev->data;
	uint16_t reg_val;
	uint16_t err_code;
	err_code = tmp117->hw_tf->read_reg(dev, TMP117_REG_CONFIGURATION, &reg_val);
	reg_val &= ~TMP117_MASK_MODE;
	reg_val |= TMP117_VALUE_MODE_SLEEP;
	err_code |= tmp117->hw_tf->write_reg(dev, TMP117_REG_CONFIGURATION, reg_val);
	return  err_code;
}

static const struct tmp117_driver_api tmp117_driver_api = {
	.soft_reset			= soft_reset_tmp117,
	.sleep			= sleep_tmp117,
	.get_sample 		= get_sample_tmp117
};

static int tmp117_init(const struct device *dev)
{
	struct tmp117_data *tmp117 = dev->data;
	const struct tmp117_config *cfg = dev->config;
	int rc;

	/* Bind to the I2C bus that the sensor is connected */
	tmp117->bus = device_get_binding(cfg->bus_name);
	if (!tmp117->bus) {
		LOG_ERR("Cannot bind to %s device!",
			cfg->bus_name);
		return -EINVAL;
	}

	cfg->bus_init(dev);

	/* Check the Device ID */
	rc = tmp117_device_id_check(dev);
	if (rc < 0) {
		return rc;
	}

	return 0;
}

#ifdef CONFIG_USERSPACE
static inline void z_vrfy_tmp117_soft_reset(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_TMP117(dev, soft_reset));
	z_impl_tmp117_soft_reset((const struct device *)dev);
}
#include <syscalls/tmp117_soft_reset_mrsh.c>

static inline void z_vrfy_tmp117_sleep(const struct device *dev)
{
	Z_OOPS(Z_SYSCALL_DRIVER_TMP117(dev, sleep));
	z_impl_tmp117_sleep((const struct device *)dev);
}
#include <syscalls/tmp117_sleep_mrsh.c>

static inline void z_vrfy_tmp117_get_sample(const struct device *dev, struct tmp117_sample *val)
{
	Z_OOPS(Z_SYSCALL_DRIVER_TMP117(dev, get_sample));
	Z_OOPS(Z_SYSCALL_MEMORY_WRITE(val, sizeof(struct tmp117_sample)));
	z_impl_tmp117_get_sample((const struct device *)dev, (struct tmp117_sample *)val);
}
#include <syscalls/tmp117_get_sample_mrsh.c>
#endif /* CONFIG_USERSPACE */

DEVICE_DT_INST_DEFINE(0, tmp117_init, NULL,
	     &tmp117_data, &tmp117_config, POST_KERNEL,
	     CONFIG_TMP117_INIT_PRIORITY, &tmp117_driver_api);
