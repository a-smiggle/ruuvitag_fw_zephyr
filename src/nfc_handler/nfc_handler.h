/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef NFC_HANDLER_H_
#define NFC_HANDLER_H_

#include "ruuvi.h"

void nfc_update(const ble_data_t * const buffer);
void nfc_init(void);

#endif /* NFC_HANDLER_H_ */
