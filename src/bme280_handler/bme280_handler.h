/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BME280_HANDLER_H_
#define BME280_HANDLER_H_

int bme280_power_state(bool state);
void bme280_fetch(void);
int16_t bme280_get_temp(void);
uint16_t bme280_get_press(void);
uint16_t bme280_get_humidity(void);
bool init_bme280(void);

#ifdef __cplusplus
}
#endif

/**
 *@}
 */

#endif /* BME280_HANDLER_H_ */
