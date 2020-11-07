/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#ifdef CONFIG_MCUMGR
#include <mgmt/mcumgr/smp_bt.h>
#endif

#include "ruuvi_endpoint.h"
#include "ruuvi.h"
#include <logging/log.h>
LOG_MODULE_REGISTER(bt_handler, CONFIG_RUUVITAG_LOG_LEVEL);

/* ********** Function prototypes ********** */
static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param);
static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

static struct k_work bc_work;
static struct k_work smp_work;

/* 
 * BLE Settings
 * 1600 * 0.625ms = 1 second 
 */
static int MIN_ADV_INT=	1600;
static int MAX_ADV_INT=	1600;

/* Allows for controlling of Speed */
#define BT_LE_BC BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, \
				      MIN_ADV_INT, \
				      MAX_ADV_INT, NULL)

/* Allows for controlling of Speed */
#define BT_LE_SMP BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY, \
				      MIN_ADV_INT, \
				      MAX_ADV_INT, NULL)


static uint8_t mfg_data[RUUVI_RAWv2_LEN] = {};
static uint8_t mcu_data[2] = {0x99, 0x04};

static const struct bt_data bc[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data)),
};

static const struct bt_data smp[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mcu_data, sizeof(mcu_data)),
#ifdef CONFIG_MCUMGR
	/* SMP */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
		      0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
#endif
};

static struct bt_conn_cb m_conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_req = le_param_req,
	.le_param_updated = le_param_updated
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		return;
	}
	LOG_INF("connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("disconnected (reason: %u)", reason);
}

static void beacon(struct k_work *work)
{
	int rc;

	bt_le_adv_stop();

	rc = bt_le_adv_start(BT_LE_BC, bc, ARRAY_SIZE(bc), NULL, 0);
	if (rc) {
		LOG_ERR("Advertising failed to start (rc %d)", rc);
		return;
	}
    ruuvi_update_endpoint(mfg_data);
		/* Update advertisement data */
	bt_le_adv_update_data(bc, ARRAY_SIZE(bc), NULL, 0);


	LOG_INF("Ruuvitag is now beaconing.");
}

static void advertise_connectable(struct k_work *work)
{
	int rc;

	bt_le_adv_stop();

	rc = bt_le_adv_start(BT_LE_SMP, smp, ARRAY_SIZE(smp), NULL, 0);
	if (rc) {
		LOG_ERR("Advertising failed to start (rc %d)", rc);
		return;
	}

	bt_le_adv_update_data(smp, ARRAY_SIZE(smp), NULL, 0);

	LOG_INF("Ruuvitag is now connectable.");
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{

}

void bt_update_packet(void){
    ruuvi_update_endpoint(mfg_data);
		/* Update advertisement data */
	bt_le_adv_update_data(bc, ARRAY_SIZE(bc), NULL, 0);
}

void bt_init(bool btn)
{
	int err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	k_work_init(&bc_work, beacon);

	k_work_init(&smp_work, advertise_connectable);

	
	bt_conn_cb_register(&m_conn_callbacks);
	if(!btn){
		k_work_submit(&bc_work);
	}
	else{
		k_work_submit(&smp_work);
	}
	
#ifdef CONFIG_MCUMGR
	/* Initialize the Bluetooth mcumgr transport. */
	smp_bt_register();
	
#endif

	LOG_DBG("Bluetooth initialized");
}

void bt_adv_stop(void)
{
	int err = bt_le_adv_stop();
	if (err) {
		LOG_ERR("Advertising failed to stop (err %d)", err);
		return;
	}
}

void bt_mode_switch(bool btn){
	if(!btn){
		k_work_submit(&bc_work);
	}
	else{
		k_work_submit(&smp_work);
	}
}
