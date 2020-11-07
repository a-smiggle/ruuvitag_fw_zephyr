/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef RUUVI_H_
#define RUUVI_H_

#pragma once

#include <zephyr.h>
#include <stdint.h>
#include <stdbool.h>

#define RUUVI_DFU_TIMEOUT CONFIG_RUUVITAG_DFU_TIMEOUT * 1000 * 60
#define RUUVI_COMPANY_ID 0x0499
#define PACKET_MODE 0x09
#define RUUVI_RAWv2_LEN 25
#define RUUVI_DSN_LENGTH_BYTES 8
#define RUUVI_DSN_LENGTH_CHAR RUUVI_DSN_LENGTH_BYTES*2

/*
 * NFC Variables
 */
#define MAX_REC_COUNT		5
#define NDEF_MSG_BUF_SIZE	128
#define fw_payload_len		10
#define ad_payload_len 		23
#define id_payload_len 		28

typedef struct mac_address_bin
{
    uint8_t mac[6];
} mac_address_bin_t;

/* The devicetree node identifier for the "sw0" alias. */
#define SW0_NODE	DT_ALIAS(sw0)

#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))
#else
#error "Unsupported board: sw0 devicetree alias is not defined"
#define SW0_GPIO_LABEL	""
#define SW0_GPIO_PIN	0
#define SW0_GPIO_FLAGS	0
#endif


#ifdef __cplusplus
}
#endif

/**
 *@}
 */

#endif /* RUUVI_H_ */
