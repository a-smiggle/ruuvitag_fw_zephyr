/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BOARD_INFO_H_
#define BOARD_INFO_H_

#include <zephyr.h>
#include "ruuvi.h"

void get_id(char *serial_number);
void get_mac(mac_address_bin_t *mac);

#endif /* BOARD_INFO_H_ */
