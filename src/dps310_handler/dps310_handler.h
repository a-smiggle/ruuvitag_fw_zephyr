/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef DPS310_HANDLER_H_
#define DPS310_HANDLER_H_

void dps310_fetch(void);
int32_t dps310_get_temp(void);
uint16_t dps310_get_press(void);
bool init_dps310(void);

#endif /* DPS310_HANDLER_H_ */
