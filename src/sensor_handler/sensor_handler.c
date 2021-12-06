#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <sensor_handler.h>
#include <drivers/gpio.h>

#include "battery_handler.h"
#include "led_handler.h"

#ifdef CONFIG_BME280
#include "bme280_handler.h"
static bool has_bme280 = false;
#endif

#ifdef CONFIG_TMP117
#include "tmp117_handler.h"
static bool has_tmp117 = false;
#endif

#ifdef CONFIG_LIS2DH12
#include "lis2dh12_handler.h"
static bool has_lis2dh12 = false;
#endif

#ifdef CONFIG_SHTCX
#include "shtcx_handler.h"
static bool has_shtcx = false;
#endif

#ifdef CONFIG_DPS310
#include "dps310_handler.h"
static bool has_dps310 = false;
#endif


#include "ruuvi_endpoint.h"
#include "nfc_handler.h"
#include "bt_handler.h"
#include "ruuvi.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(sensor_handler);

/* The devicetree node identifier for the "Sensor Power Pins" alias. */
/*
#define SNP1_NODE DT_ALIAS(snp1)
#define SNP2_NODE DT_ALIAS(snp2)

#if DT_NODE_HAS_STATUS(SNP1_NODE, okay)
#define SNP1	DT_GPIO_LABEL(SNP1_NODE, gpios)
#define SNP1_PIN	DT_GPIO_PIN(SNP1_NODE, gpios)
#if DT_PHA_HAS_CELL(SNP1_NODE, gpios, flags)
#define SNP1_FLAGS	DT_GPIO_FLAGS(SNP1_NODE, gpios)
#endif
#endif

#if DT_NODE_HAS_STATUS(SNP2_NODE, okay)
#define SNP2	DT_GPIO_LABEL(SNP2_NODE, gpios)
#define SNP2_PIN	DT_GPIO_PIN(SNP2_NODE, gpios)
#if DT_PHA_HAS_CELL(SNP1_NODE, gpios, flags)
#define SNP2_FLAGS	DT_GPIO_FLAGS(SNP2_NODE, gpios)
#endif
#endif

#ifndef SNP1_FLAGS
#define SNP1_FLAGS	0
#endif

#ifndef SNP2_FLAGS
#define SNP2_FLAGS	0
#endif

#define SNP_TIME	10

const struct device *sensor_pwr_1;
const struct device *sensor_pwr_2;

static bool snp1_enabled = false;
static bool snp2_enabled = false;
*/
static bool has_adc = false;

static uint16_t acceleration_events = 0;
static int64_t battery_check = 0;
static sensor_data_t sensor_data = {0};
static ble_data_t buffer = { .id = {0x99, 0x04}};
/*
void enable_sensor_power(void){
	LOG_DBG("Enabling Sensor Power Pins.\n");
	snp1_enabled = true;
	snp2_enabled = true;
	gpio_pin_set(sensor_pwr_1, SNP1_PIN, snp1_enabled);
	gpio_pin_set(sensor_pwr_2, SNP2_PIN, snp2_enabled);

}

void disable_sensor_power_01(void){
	LOG_DBG("Disabling Sensor Power Pins.\n");
	snp1_enabled = false;
	gpio_pin_set(sensor_pwr_1, SNP1_PIN, snp1_enabled);
}

void disable_sensor_power_02(void){
	LOG_DBG("Disabling Sensor Power Pins.\n");
	snp2_enabled = false;
	gpio_pin_set(sensor_pwr_2, SNP2_PIN, snp2_enabled);
}


void toggle_sensor_power(void)
{
	snp1_enabled = !snp1_enabled;
	snp2_enabled = !snp2_enabled;
	gpio_pin_set(sensor_pwr_1, SNP1_PIN, snp1_enabled);
	gpio_pin_set(sensor_pwr_2, SNP2_PIN, snp2_enabled);
}

void power_pin_init(void)
{
	sensor_pwr_1 = device_get_binding(SNP1);
	sensor_pwr_2 = device_get_binding(SNP2);

	gpio_pin_configure(sensor_pwr_1, SNP1_PIN, GPIO_OUTPUT_ACTIVE | SNP1_FLAGS);
	gpio_pin_configure(sensor_pwr_2, SNP2_PIN, GPIO_OUTPUT_ACTIVE | SNP2_FLAGS);

	gpio_pin_set(sensor_pwr_1, SNP1_PIN, 0);
	gpio_pin_set(sensor_pwr_2, SNP2_PIN, 0);
	snp1_enabled = false;
	snp2_enabled = false;
	LOG_DBG("Power Pins Configured.\n");
}*/

static void update_battery(void){
    if(k_uptime_get() - battery_check > 10000){
        battery_check 	= k_uptime_get();
        sensor_data.vbatt 			= get_battery_level();
        LOG_DBG("Battery: %d mV", sensor_data.vbatt);
	}
}


#ifdef CONFIG_BME280
static void update_bme(void){
    bme280_fetch();
    sensor_data.temperature = 	bme280_get_temp();
    sensor_data.pressure    = 	bme280_get_press();
    sensor_data.humidity    = 	bme280_get_humidity();
    LOG_INF("Temperature: %d, Pressure: %d, Humidity: %d", 
                        sensor_data.temperature, 
                        sensor_data.pressure, 
                        sensor_data.humidity);
}
#endif

#ifdef CONFIG_LIS2DH12
static void update_lis2dh12(void){
    lis2dh12_fetch();
    sensor_data.x = lis2dh12_get(0);
    sensor_data.y = lis2dh12_get(1);
    sensor_data.z = lis2dh12_get(2);
    LOG_DBG("X: %d, Y: %d, Z: %d", sensor_data.x, sensor_data.y, sensor_data.z);
}
#endif

#ifdef CONFIG_SHTCX
static void update_shtcx(void){
    shtcx_fetch();
    sensor_data.temperature = shtcx_get_temp();
    sensor_data.humidity    = shtcx_get_humidity();
    LOG_DBG("Temperature: %d, Humidity: %d",sensor_data.temperature, sensor_data.humidity);
}
#endif


#ifdef CONFIG_DPS310
static void update_dps310(void){
    dps310_fetch();
    sensor_data.temperature = dps310_get_temp();
    LOG_DBG("Temperature: %d", sensor_data.temperature);
}
#endif


#ifdef CONFIG_TMP117
static void update_tmp117(void){
    tmp117_fetch();
    sensor_data.temperature = tmp117_get_temp();
    LOG_DBG("Temperature: %d", sensor_data.temperature);
}
#endif


static void package_sensor_data(void)
{
	ruuvi_raw_v2_encode(buffer.data, sensor_data, acceleration_events);
	bt_update_packet(&buffer);
	nfc_update(&buffer);
}

void udpate_sensor_data(void)
{
    if (has_adc){
        update_battery();
    }
#ifdef CONFIG_BME280
    if (has_bme280){
        update_bme();
    }
#endif

#ifdef CONFIG_LIS2DH12
    if (has_lis2dh12){
        update_lis2dh12();
    }
#endif

#ifdef CONFIG_SHTCX
    if (has_shtcx){
        update_shtcx();
    }
#endif

#ifdef CONFIG_DPS310 
    if (has_dps310){
	update_dps310();
    }
#endif

#ifdef CONFIG_TMP117 
    if (has_tmp117){
	update_tmp117();
    }
#endif

	package_sensor_data();
}

void sensor_init(void){
	//power_pin_init();
	//enable_sensor_power();
       // k_msleep(50);
       

    has_adc = init_adc();
	if (!has_adc) {
		LOG_ERR("Failed to initialize ADC\n");
		flash_red();
	}
#ifdef CONFIG_BME280
	has_bme280 = init_bme280();
	if (!has_bme280) {
		LOG_ERR("Failed to initialize BME280\n");
		flash_red();
	}
#endif

#ifdef CONFIG_LIS2DH12
	has_lis2dh12 = init_lis2dh12();
	if (!has_lis2dh12) {
		LOG_ERR("Failed to initialize LIS2DH12\n");
		flash_red();
	}
#endif

#ifdef CONFIG_DPS310
	has_dps310 = init_dps310();
	if (!has_dps310) {
		LOG_ERR("Failed to initialize DPS310\n");
		flash_red();
	}
#endif

#ifdef CONFIG_TMP117
	has_tmp117 = init_tmp117();
	if (!has_tmp117) {
		LOG_ERR("Failed to initialize TMP117\n");
		flash_red();
	}
#endif
 
#ifdef CONFIG_SHTCX
        has_shtcx = init_shtcx();
	if (!has_shtcx) {
		LOG_ERR("Failed to initialize SHTCx\n");
		flash_red();
	}
#endif



}
