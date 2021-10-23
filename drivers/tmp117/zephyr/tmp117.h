/*
 * Copyright (c) 2019 Centaur Analytics, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_TMP117_TMP117_H_
#define ZEPHYR_DRIVERS_SENSOR_tmp117_tmp117_H_

#define tmp117_REG_TEMP				0x0
#define tmp117_REG_CFGR				0x1
#define tmp117_REG_HIGH_LIM			0x2
#define tmp117_REG_LOW_LIM			0x3
#define tmp117_REG_EEPROM_UL		0x4
#define tmp117_REG_EEPROM1			0x5
#define tmp117_REG_EEPROM2			0x6
#define tmp117_REG_EEPROM3			0x7
#define TMP117_REG_TEMP_OFFSET		0x7
#define tmp117_REG_EEPROM4			0x8
#define tmp117_REG_DEVICE_ID		0xF

#define tmp117_RESOLUTION			78125	/* in tens of uCelsius*/
#define tmp117_RESOLUTION_DIV		10000000

#define TMP116_DEVICE_ID			0x1116
#define TMP117_DEVICE_ID			0x0117

#define TMP117_CFGR_DATA_READY BIT(13)

#define TMP117_MASK_RESET			(0x0002U)
#define TMP117_MASK_MODE			(0x0C00U)
#define TMP117_POS_MODE				(10U)
#define TMP117_VALUE_MODE_SLEEP		(0x01U << TMP117_POS_MODE)
#define TMP117_VALUE_MODE_SINGLE	(0x03U << TMP117_POS_MODE)
#define TMP117_VALUE_MODE_CONT		(0x00U << TMP117_POS_MODE)

#define TMP117_POS_OS            (5U)
#define TMP117_VALUE_OS_1        (0x00U << TMP117_POS_OS)
#define TMP117_VALUE_OS_8        (0x01U << TMP117_POS_OS)
#define TMP117_VALUE_OS_32       (0x02U << TMP117_POS_OS)
#define TMP117_VALUE_OS_64       (0x03U << TMP117_POS_OS)

#define TMP117_POS_DRDY          (13U)
#define TMP117_MASK_DRDY         (1U << TMP117_POS_DRDY)

#define TMP117_VALUE_TEMP_NA     (0x8000U)
#define TMP117_OS_1_TSAMPLE_MS   (16U)
#define TMP117_OS_8_TSAMPLE_MS   (125U)
#define TMP117_OS_32_TSAMPLE_MS  (500U)
#define TMP117_OS_64_TSAMPLE_MS  (1000U)

// POR reset 1.5 ms, soft reset 2 ms, margin.
#define TMP117_CC_RESET_DELAY_MS (4U)


struct tmp117_data {
	const struct device *i2c;
	uint16_t sample;
	uint16_t id;
};

struct tmp117_dev_config {
	uint16_t i2c_addr;
	char *i2c_bus_label;
};

static int tmp117_sample_fetch(const struct device *dev,enum sensor_channel chan);
static int tmp117_channel_get(const struct device *dev,enum sensor_channel chan,struct sensor_value *val);
static int tmp117_init(const struct device *dev);
static int tmp117_soft_reset(const struct device *dev);
static int tmp117_sleep(const struct device *dev);


#endif /*  ZEPHYR_DRIVERS_SENSOR_tmp117_tmp117_H_ */
