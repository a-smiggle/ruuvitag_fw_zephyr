#define DT_DRV_COMPAT st_lis2dh12

#include <string.h>
#include "lis2dh12.h"
#include <logging/log.h>

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)

LOG_MODULE_DECLARE(lis2dh12, 3);


#define LIS2DH12_SPI_READM			(1 << 7)
#define LIS2DH12_SPI_WRITEM			(1 << 6)
#define LIS2DH_SPI_AUTOINC			BIT(6)

/* LIS2DH supports only SPI mode 0, word size 8 bits, MSB first */
#define LIS2DH12_SPI_CFG			SPI_WORD_SET(8)

static struct spi_config lis2dh12_spi_conf = {
	.frequency = DT_INST_PROP(0, spi_max_frequency),
	.operation = (SPI_WORD_SET(8) | SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA),
	.slave     = DT_INST_REG_ADDR(0),
	.cs        = NULL,
};

static int lis2dh12_spi_read(struct lis2dh12_data *ctx, uint8_t reg,
			   uint8_t *data, uint16_t len)
{
	struct spi_config *spi_cfg = &lis2dh12_spi_conf;
	uint8_t buffer_tx[2] = { reg | 0x80};
	const struct spi_buf tx_buf = {
			.buf = buffer_tx,
			.len = 2,
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};
	const struct spi_buf rx_buf[2] = {
		{
			.buf = NULL,
			.len = 1,
		},
		{
			.buf = data,
			.len = len,
		}
	};
	const struct spi_buf_set rx = {
		.buffers = rx_buf,
		.count = 2
	};

	if (len > 64) {
		return -EIO;
	}

	if (len > 1) {
		buffer_tx[0] |= 0x40;
	}

	if (spi_transceive(ctx->bus, spi_cfg, &tx, &rx)) {
		return -EIO;
	}

	return 0;
}

static int lis2dh12_spi_write(struct lis2dh12_data *ctx, uint8_t reg,
			   uint8_t *data, uint16_t len)
{
	struct spi_config *spi_cfg = &lis2dh12_spi_conf;
	uint8_t buffer_tx[1] = { reg };
	const struct spi_buf tx_buf[2] = {
		{
			.buf = buffer_tx,
			.len = 1,
		},
		{
			.buf = data,
			.len = len,
		}
	};
	const struct spi_buf_set tx = {
		.buffers = tx_buf,
		.count = 2
	};

	if (len > 64) {
		return -EIO;
	}

	if (len > 1) {
		buffer_tx[0] |= 0x40;
	}


	if (spi_write(ctx->bus, spi_cfg, &tx)) {
		return -EIO;
	}

	return 0;
}

stmdev_ctx_t lis2dh12_spi_ctx = {
	.read_reg = (stmdev_read_ptr) lis2dh12_spi_read,
	.write_reg = (stmdev_write_ptr) lis2dh12_spi_write,
};

int lis2dh12_spi_init(const struct device *dev)
{
	LOG_INF("lis2dh12_spi_init\n");
	struct lis2dh12_data *data = dev->data;

	data->ctx = &lis2dh12_spi_ctx;
	data->ctx->handle = data;

#if DT_INST_SPI_DEV_HAS_CS_GPIOS(0)
	/* handle SPI CS thru GPIO if it is the case */
	data->cs_ctrl.gpio_dev = device_get_binding(
		DT_INST_SPI_DEV_CS_GPIOS_LABEL(0));
	if (!data->cs_ctrl.gpio_dev) {
		LOG_ERR("Unable to get GPIO SPI CS device");
		return -ENODEV;
	}

	data->cs_ctrl.gpio_pin = DT_INST_SPI_DEV_CS_GPIOS_PIN(0);
	data->cs_ctrl.gpio_dt_flags = DT_INST_SPI_DEV_CS_GPIOS_FLAGS(0);
	data->cs_ctrl.delay = 0U;

	lis2dh12_spi_conf.cs = &data->cs_ctrl;

	LOG_INF("SPI GPIO CS configured on %s:%u",
		    DT_INST_SPI_DEV_CS_GPIOS_LABEL(0),
		    DT_INST_SPI_DEV_CS_GPIOS_PIN(0));
#endif

	return 0;
}
#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */
