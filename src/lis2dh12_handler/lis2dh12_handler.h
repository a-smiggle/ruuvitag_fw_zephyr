/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef LIS2DH12_HANDLER_H_
#define LIS2DH12_HANDLER_H_

void lis2dh12_fetch(void);
int16_t lis2dh12_get(int axis);
bool init_lis2dh12(void);

#endif /* LIS2DH12_HANDLER_H_ */
