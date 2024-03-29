/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BME280_HANDLER_H_
#define BME280_HANDLER_H_

void bme280_fetch(void);
int32_t bme280_get_temp(void);
uint16_t bme280_get_press(void);
uint32_t bme280_get_humidity(void);
bool init_bme280(void);

#endif /* BME280_HANDLER_H_ */
