#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

#include "led_handler.h"

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED0_NODE, gpios)
#if DT_PHA_HAS_CELL(LED0_NODE, gpios, flags)
#define FLAGS0	DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#if DT_PHA_HAS_CELL(LED1_NODE, gpios, flags)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)
#endif
#endif

#ifndef FLAGS0
#define FLAGS0	0
#endif

#ifndef FLAGS1
#define FLAGS1	0
#endif

#define LED_TIME	10

static struct device *green_led;
static struct device *red_led;


void led_init(void)
{
	green_led = device_get_binding(LED0);
	red_led = device_get_binding(LED1);

	gpio_pin_configure(green_led, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS0);
	gpio_pin_configure(red_led, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1);

	gpio_pin_set(green_led, PIN0, 0);
	gpio_pin_set(red_led, PIN1, 0);
}

void toggle_red(uint8_t on)
{
    if(on){
        gpio_pin_set(green_led, PIN0, 1);
    }
    else {
        gpio_pin_set(green_led, PIN0, 0);
    }    
}

void flash_red(void)
{
    gpio_pin_set(green_led, PIN0, 1);
    k_sleep(K_MSEC(LED_TIME));
    gpio_pin_set(green_led, PIN0, 0);
}

void toggle_green(uint8_t on)
{
    if(on){
        gpio_pin_set(red_led, PIN1, 1);
    }
    else{
        gpio_pin_set(red_led, PIN1, 0);
    }
}

void flash_green(void)
{
    gpio_pin_set(red_led, PIN1, 1);
    k_sleep(K_MSEC(LED_TIME));
    gpio_pin_set(red_led, PIN1, 0);
}