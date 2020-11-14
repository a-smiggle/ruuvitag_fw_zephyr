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

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/* ********** Function prototypes ********** */
static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param);
static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

static struct k_work ble_work;

/* 
 * Predefined GAP timings can be found at:
 * zephyr/include/bluetooth/gap.h
 */

/* Allows for controlling of Speed */
#define BT_LE_ADV BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY, \
				      BT_GAP_ADV_SLOW_INT_MIN, \
				      BT_GAP_ADV_SLOW_INT_MIN, NULL)

static uint8_t mfg_data[RUUVI_MFG_OFFSET + RUUVI_RAWv2_LEN];

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
		      0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
	BT_DATA_BYTES(BT_DATA_TX_POWER, RUUVI_TX_POWER)
};

static const struct bt_data bc[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data)),
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

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{

}

static struct bt_conn_cb m_conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.le_param_req = le_param_req,
	.le_param_updated = le_param_updated
};

static void advertise(struct k_work *work)
{
	int rc;

	bt_le_adv_stop();

	rc = bt_le_adv_start(BT_LE_ADV, bc, ARRAY_SIZE(bc), sd, ARRAY_SIZE(sd));
	if (rc) {
		LOG_ERR("Advertising failed to start (rc %d)", rc);
		return;
	}
	mfg_data[0] = 0x99;
	mfg_data[1] = 0x04;
    ruuvi_update_endpoint(mfg_data);
		/* Update advertisement data */
	bt_le_adv_update_data(bc, ARRAY_SIZE(bc), sd, ARRAY_SIZE(sd));

	LOG_INF("Ruuvitag is now beaconing.");
}

void bt_update_packet(void){
    ruuvi_update_endpoint(mfg_data);
		/* Update advertisement data */
	bt_le_adv_update_data(bc, ARRAY_SIZE(bc), sd, ARRAY_SIZE(sd));
	LOG_DBG("Updating BLE packet");
}

void bt_init(void)
{
	int err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}

	k_work_init(&ble_work, advertise);

	bt_conn_cb_register(&m_conn_callbacks);

	k_work_submit(&ble_work);

	
#ifdef CONFIG_MCUMGR
	/* Initialize the Bluetooth mcumgr transport. */
	smp_bt_register();
	
#endif

	LOG_INF("Bluetooth initialized");
}

void bt_adv_stop(void)
{
	int err = bt_le_adv_stop();
	if (err) {
		LOG_ERR("Advertising failed to stop (err %d)", err);
		return;
	}
}
