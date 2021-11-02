/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef RUUVI_ENDPOINT_H_
#define RUUVI_ENDPOINT_H_

#include "ruuvi.h"

void ruuvi_raw_v2_encode(uint8_t *data, sensor_data_t sensor_data, uint16_t acc_events );

#endif /* RUUVI_ENDPOINT_H_ */
