#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

#include "led_controller.h"

#define ONE_LED_PORT	DT_ALIAS_LED0_GPIOS_CONTROLLER
#define ONE_LED			DT_ALIAS_LED0_GPIOS_PIN
#define TWO_LED_PORT	DT_ALIAS_LED1_GPIOS_CONTROLLER
#define TWO_LED		    DT_ALIAS_LED1_GPIOS_PIN
#define THREE_LED_PORT	DT_ALIAS_LED2_GPIOS_CONTROLLER
#define THREE_LED		DT_ALIAS_LED2_GPIOS_PIN
#define FOUR_LED_PORT	DT_ALIAS_LED3_GPIOS_CONTROLLER
#define FOUR_LED		DT_ALIAS_LED3_GPIOS_PIN

#define LED_TIME	50

static struct device *led_one;
static struct device *led_two;


void led_init(void)
{
	led_one = device_get_binding(ONE_LED_PORT);
	led_two = device_get_binding(TWO_LED_PORT);

	gpio_pin_configure(led_one, ONE_LED, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure(led_two, TWO_LED, GPIO_OUTPUT_ACTIVE);

	gpio_pin_set(led_one, ONE_LED, 1);
	gpio_pin_set(led_two, TWO_LED, 1);
}

void toggle_red(u8_t on)
{
    if(on){
        gpio_pin_set(led_one, ONE_LED, 0);
    }
    else {
        gpio_pin_set(led_one, ONE_LED, 1);
    }    
}

void flash_red(void)
{
    gpio_pin_set(led_one, ONE_LED, 0);
    k_sleep(LED_TIME);
    gpio_pin_set(led_one, ONE_LED, 1);
}

void toggle_green(u8_t on)
{
    if(on){
        gpio_pin_set(led_two, TWO_LED, 0);
    }
    else{
        gpio_pin_set(led_two, TWO_LED, 1);
    }
}

void flash_green(void)
{
    gpio_pin_set(led_two, TWO_LED, 0);
    k_sleep(LED_TIME);
    gpio_pin_set(led_two, TWO_LED, 1);
}