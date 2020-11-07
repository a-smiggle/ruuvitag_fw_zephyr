/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BATTERY_HANDLER_H_
#define BATTERY_HANDLER_H_

int adc_power_state(bool state);
int16_t get_battery_level(void);
bool init_adc(void);

#ifdef __cplusplus
}
#endif

/**
 *@}
 */

#endif /* BATTERY_HANDLER_H_ */