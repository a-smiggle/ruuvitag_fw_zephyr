/*
 * Copyright (c) 2019 Centaur Analytics, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_tmp117

#include <device.h>
#include <drivers/i2c.h>
#include <drivers/sensor.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <sys/__assert.h>
#include <logging/log.h>
#include <kernel.h>

#include "tmp117.h"

LOG_MODULE_REGISTER(tmp117, CONFIG_SENSOR_LOG_LEVEL);

static int tmp117_reg_read(const struct device *dev, uint8_t reg,
			   uint16_t *val)
{
	struct tmp117_data *drv_data = dev->data;
	const struct tmp117_dev_config *cfg = dev->config;

	if (i2c_burst_read(drv_data->i2c, cfg->i2c_addr, reg, (uint8_t *)val, 2)
	    < 0) {
		return -EIO;
	}

	*val = sys_be16_to_cpu(*val);

	return 0;
}

static int tmp117_reg_write(const struct device *dev, uint8_t reg,
			    uint16_t val)
{
	struct tmp117_data *drv_data = dev->data;
	const struct tmp117_dev_config *cfg = dev->config;
	uint8_t tx_buf[3] = {reg, val >> 8, val & 0xFF};

	return i2c_write(drv_data->i2c, tx_buf, sizeof(tx_buf),
			cfg->i2c_addr);
}

/**
 * @brief Check the Device ID
 *
 * @param[in]   dev     Pointer to the device structure
 * @param[in]	id	Pointer to the variable for storing the device id
 *
 * @retval 0 on success
 * @retval -EIO Otherwise
 */
static inline int tmp117_device_id_check(const struct device *dev, uint16_t *id)
{
	if (tmp117_reg_read(dev, tmp117_REG_DEVICE_ID, id) != 0) {
		LOG_ERR("%s: Failed to get Device ID register!",
			dev->name);
		return -EIO;
	}

	if ((*id != TMP117_DEVICE_ID) && (*id != TMP117_DEVICE_ID)) {
		LOG_ERR("%s: Failed to match the device IDs!",
			dev->name);
		return -EINVAL;
	}

	return 0;
}

static int tmp117_sample_fetch(const struct device *dev,
			       enum sensor_channel chan)
{
	struct tmp117_data *drv_data = dev->data;
	uint16_t value;
	uint16_t cfg_reg = 0;
	int rc;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL ||
			chan == SENSOR_CHAN_AMBIENT_TEMP);

	/* clear sensor values */
	drv_data->sample = 0U;

	/* Make sure that a data is available */
	rc = tmp117_reg_read(dev, tmp117_REG_CFGR, &cfg_reg);
	if (rc < 0) {
		LOG_ERR("%s, Failed to read from CFGR register",
			dev->name);
		return rc;
	}

	if ((cfg_reg & TMP117_CFGR_DATA_READY) == 0) {
		LOG_DBG("%s: no data ready", dev->name);
		return -EBUSY;
	}

	/* Get the most recent temperature measurement */
	rc = tmp117_reg_read(dev, tmp117_REG_TEMP, &value);
	if (rc < 0) {
		LOG_ERR("%s: Failed to read from TEMP register!",
			dev->name);
		return rc;
	}

	/* store measurements to the driver */
	drv_data->sample = (int16_t)value;

	return 0;
}

static int tmp117_channel_get(const struct device *dev,
			      enum sensor_channel chan,
			      struct sensor_value *val)
{
	struct tmp117_data *drv_data = dev->data;
	int32_t tmp;

	if (chan != SENSOR_CHAN_AMBIENT_TEMP) {
		return -ENOTSUP;
	}

	/*
	 * See datasheet "Temperature Results and Limits" section for more
	 * details on processing sample data.
	 */
	tmp = ((int16_t)drv_data->sample * (int32_t)tmp117_RESOLUTION) / 10;
	val->val1 = tmp / 1000000; /* uCelsius */
	val->val2 = tmp % 1000000;

	return 0;
}

static int tmp117_attr_set(const struct device *dev,
			   enum sensor_channel chan,
			   enum sensor_attribute attr,
			   const struct sensor_value *val)
{
	struct tmp117_data *drv_data = dev->data;
	int16_t value;

	if (chan != SENSOR_CHAN_AMBIENT_TEMP) {
		return -ENOTSUP;
	}

	switch (attr) {
	case SENSOR_ATTR_OFFSET:
		if (drv_data->id != TMP117_DEVICE_ID) {
			LOG_ERR("%s: Offset is only supported by TMP117",
			dev->name);
			return -EINVAL;
		}
		/*
		 * The offset is encoded into the temperature register format.
		 */
		value = (((val->val1) * 10000000) + ((val->val2) * 10))
						/ (int32_t)tmp117_RESOLUTION;

		return tmp117_reg_write(dev, TMP117_REG_TEMP_OFFSET, value);

	default:
		return -ENOTSUP;
	}
}

static const struct sensor_driver_api tmp117_driver_api = {
	.attr_set = tmp117_attr_set,
	.sample_fetch = tmp117_sample_fetch,
	.channel_get = tmp117_channel_get
};

static int tmp117_init(const struct device *dev)
{
	struct tmp117_data *drv_data = dev->data;
	const struct tmp117_dev_config *cfg = dev->config;
	int rc;
	uint16_t id;

	/* Bind to the I2C bus that the sensor is connected */
	drv_data->i2c = device_get_binding(cfg->i2c_bus_label);
	if (!drv_data->i2c) {
		LOG_ERR("Cannot bind to %s device!",
			cfg->i2c_bus_label);
		return -EINVAL;
	}

	/* Check the Device ID */
	rc = tmp117_device_id_check(dev, &id);
	if (rc < 0) {
		return rc;
	}
	LOG_DBG("Got device ID: %x", id);
	drv_data->id = id;

	return 0;
}



static int tmp117_soft_reset(const struct device *dev)
  {
      uint16_t err_code;
      uint16_t reset = TMP117_MASK_RESET & 0xFFFF;

      tmp117_reg_write(dev, tmp117_REG_CFGR, reset);
      k_sleep(K_MSEC(TMP117_CC_RESET_DELAY_MS));
      return err_code;
  }


static int tmp117_sleep(const struct device *dev)
  {
      uint16_t reg_val;
      uint16_t err_code;
      err_code = tmp117_reg_read(dev, tmp117_REG_CFGR, &reg_val);
      reg_val &= ~TMP117_MASK_MODE;
      reg_val |= TMP117_VALUE_MODE_SLEEP;
      err_code |= tmp117_reg_write(dev, tmp117_REG_CFGR, reg_val);
      return  err_code;
  }



#define DEFINE_tmp117(_num) \
	static struct tmp117_data tmp117_data_##_num; \
	static const struct tmp117_dev_config tmp117_config_##_num = { \
		.i2c_addr = DT_INST_REG_ADDR(_num), \
		.i2c_bus_label = DT_INST_BUS_LABEL(_num) \
	}; \
	DEVICE_DT_INST_DEFINE(_num, tmp117_init, NULL,			\
		&tmp117_data_##_num, &tmp117_config_##_num, POST_KERNEL, \
		CONFIG_SENSOR_INIT_PRIORITY, &tmp117_driver_api);

DT_INST_FOREACH_STATUS_OKAY(DEFINE_tmp117)
