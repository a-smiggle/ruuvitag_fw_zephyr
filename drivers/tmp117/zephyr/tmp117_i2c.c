#define DT_DRV_COMPAT ti_tmp117

#include <string.h>
#include <drivers/i2c.h>
#include <logging/log.h>
#include <sys/byteorder.h>

#include "tmp117.h"

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

LOG_MODULE_DECLARE(tmp117, CONFIG_TMP117_LOG_LEVEL);

static int tmp117_reg_read(const struct device *dev, uint8_t reg,
			   uint16_t *val)
{
	struct tmp117_data *data = dev->data;
	const struct tmp117_config *cfg = dev->config;

	if (i2c_burst_read(data->bus, cfg->bus_cfg.i2c_slv_addr,
                 reg, (uint8_t *)val, 2)
	    < 0) {
		return -EIO;
	}

	*val = sys_be16_to_cpu(*val);

	return 0;
}

static int tmp117_reg_write(const struct device *dev, uint8_t reg,
			    uint16_t val)
{
	struct tmp117_data *tmp117 = dev->data;
	const struct tmp117_config *cfg = dev->config;
	uint8_t tx_buf[3] = {reg, val >> 8, val & 0xFF};

	return i2c_write(tmp117->bus, tx_buf, sizeof(tx_buf),
			cfg->bus_cfg.i2c_slv_addr);
}

static const struct tmp117_transfer_function tmp117_i2c_transfer_fn = {
	.read_reg  = tmp117_reg_read,
	.write_reg  = tmp117_reg_write,
};

int tmp117_i2c_init(const struct device *dev)
{
	struct tmp117_data *data = dev->data;

	data->hw_tf = &tmp117_i2c_transfer_fn;

	return 0;
}

#endif