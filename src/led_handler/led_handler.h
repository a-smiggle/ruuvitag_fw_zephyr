/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef LED_HANDLER_H_
#define LED_HANDLER_H_

void led_init(void);
void toggle_red(uint8_t on);
void flash_red(void);
void toggle_green(uint8_t on);
void flash_green(void);

#endif /* LED_HANDLER_H_ */
