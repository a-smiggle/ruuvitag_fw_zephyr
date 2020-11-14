/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <stdio.h>
#include <drivers/hwinfo.h>
#include <sys/byteorder.h>
#include <bluetooth/bluetooth.h>

#include "ruuvi.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(board_info, CONFIG_RUUVITAG_LOG_LEVEL);

void get_id(char *serial_number)
{
	uint8_t device_id[RUUVI_DSN_LENGTH_BYTES];
	ssize_t device_id_len;
	int len;

	device_id_len = hwinfo_get_device_id(device_id, sizeof(device_id));
	__ASSERT_NO_MSG(device_id_len == RUUVI_DSN_LENGTH_BYTES);

	/* Expand binary device ID to hex string */
	len = bin2hex(device_id, sizeof(device_id), serial_number,
		      RUUVI_DSN_LENGTH_CHAR + 1);
	__ASSERT_NO_MSG(len == RUUVI_DSN_LENGTH_CHAR);

	LOG_INF("Serial: %s", log_strdup(serial_number));
}

void get_mac(mac_address_bin_t *mac){
    static bt_addr_le_t m[CONFIG_BT_ID_MAX];
	
	size_t count = CONFIG_BT_ID_MAX;

	bt_id_get(m, &count);

    memcpy(mac->mac, m->a.val, sizeof(m->a.val));
}