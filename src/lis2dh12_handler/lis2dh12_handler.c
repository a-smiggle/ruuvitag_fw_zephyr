/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#ifdef CONFIG_LIS2DH12

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <lis2dh12.h>
#include "lis2dh12_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(lis2dh12_handler, CONFIG_RUUVITAG_LOG_LEVEL);

const struct device *lis2dh12;
struct lis2dh12_sample lis2dh12_sample;

void lis2dh12_fetch(void)
{
	lis2dh12_get_acc_sample(lis2dh12, &lis2dh12_sample);
}

int16_t lis2dh12_get(int axis){
	if (axis == 0){
		return lis2dh12_sample.acc_mg[0];
	}
	else if (axis == 1){
		return lis2dh12_sample.acc_mg[1];
	}
	else if (axis == 2){
		return lis2dh12_sample.acc_mg[2];
	}
	else{
		return 0;
	}
}

bool init_lis2dh12(void){
	lis2dh12 = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh12)));

	if (lis2dh12 == NULL) {
		return false;
	} else {
		return true;
	}
}
#endif