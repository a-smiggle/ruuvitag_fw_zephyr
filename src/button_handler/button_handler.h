/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BUTTON_HANDLER_H_
#define BUTTON_HANDLER_H_

void button_int_setup(struct gpio_callback *handle, gpio_callback_handler_t cbh);
bool button_pressed_state(void);
void button_init(void);

#endif /* BUTTON_HANDLER_H_ */
