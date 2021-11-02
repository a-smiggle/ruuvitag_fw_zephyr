/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef SENSOR_HANDLER_H_
#define SENSOR_HANDLER_H_

void enable_sensor_power(void);
void disable_sensor_power(void);
void toggle_sensor_power(void);
void power_pin_init(void);
void udpate_sensor_data(void);
void sensor_init(void);

#endif /* SENSOR_HANDLER_H_ */
