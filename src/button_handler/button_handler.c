#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>

#include "ruuvi.h"
#include "button_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(button_handler, CONFIG_RUUVITAG_LOG_LEVEL);


const struct device *button;

void button_int_setup(struct gpio_callback *handle, gpio_callback_handler_t cbh){
    int ret = gpio_pin_interrupt_configure(button,
					   SW0_GPIO_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	gpio_init_callback(handle, cbh, BIT(SW0_GPIO_PIN));
	gpio_add_callback(button, handle);
    return;
}

bool button_pressed_state(void){
    int pressed = gpio_pin_get_raw(button, SW0_GPIO_PIN);
    if(pressed){
        return false;
    }
    else{
        return true;
    }
}

void button_init(void)
{
	button = device_get_binding(SW0_GPIO_LABEL);
	if (button == NULL) {
		LOG_ERR("Error: didn't find %s device\n", SW0_GPIO_LABEL);
		return;
	}

	int ret = gpio_pin_configure(button, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
		       ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}
}
