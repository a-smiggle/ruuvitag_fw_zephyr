/*
 * Copyright (c) 2021 ALueger
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <settings/settings.h>
#include <stdio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include "ruuvi.h"
#include "led_handler.h"
#include "bt_handler.h"
#include "battery_handler.h"

#ifdef CONFIG_MCUMGR
#include <mgmt/mcumgr/smp_bt.h>
#endif

#include <dk_buttons_and_leds.h>

#include <logging/log.h>

#define LOG_MODULE_NAME BT_HANDLER
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define CONFIG_BT_NUS_UART_BUFFER_SIZE 1024
#define CONFIG_BT_NUS_THREAD_STACK_SIZE 1024

#define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)

#define INTERVAL_MIN	0x140	/* 320 units, 400 ms */
#define INTERVAL_MAX	0x140	/* 320 units, 400 ms */

static struct bt_le_conn_param *conn_param =
	BT_LE_CONN_PARAM(INTERVAL_MIN, INTERVAL_MAX, 0, 600);


static bool ble_notify_status = false;

//static uint8_t test_level =100U;

struct uart_data_t {
	void *fifo_reserved;
	uint8_t data[UART_BUF_SIZE];
	uint16_t len;
};

static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);
/*******************************************************************/


/*  Nordic-UART Declarations and  Routines                         */
static uint8_t mfg_notify[18];
static struct bt_nus_cb nus_cb;

static void on_received_nus(struct bt_conn *conn, const uint8_t *const data,
    uint16_t len) {
  //int err;
  char addr[BT_ADDR_LE_STR_LEN] = {0};

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

  LOG_INF("Received data from: %s", log_strdup(addr));

  for (uint16_t pos = 0; pos != len;) {
    struct uart_data_t *tx = k_malloc(sizeof(*tx));

    if (!tx) {
      LOG_WRN("Not able to allocate UART send data buffer");
      return;
    }
    //Keep the last byte of TX buffer for potential LF char. 
    size_t tx_data_size = sizeof(tx->data) - 1;

    if ((len - pos) > tx_data_size) {
      tx->len = tx_data_size;
      } else {
        tx->len = (len - pos);
      }

    memcpy(tx->data, &data[pos], tx->len);
    pos += tx->len;
    // Append the LF character when the CR character triggered transmission from the peer.
	
    if ((pos == len) && (data[len - 1] == '\r')) {
      tx->data[tx->len] = '\n';
      tx->len++;
      }
    k_fifo_put(&fifo_uart_rx_data, tx);
  }
}

static void on_send_enabled_nus(enum bt_nus_send_status status) 
{   
    if (status == BT_NUS_SEND_STATUS_ENABLED) {
      ble_notify_status= true;
      LOG_INF("Notify True ");
      return;
      }
    if (status == BT_NUS_SEND_STATUS_DISABLED) {
      ble_notify_status= false;
      LOG_INF("Notify False ");
      return;
      }
}

static void on_sent_nus(struct bt_conn *conn){
      LOG_INF("Notify Send ");  
}

static struct bt_nus_cb nus_cb = {
    .received = on_received_nus,
    .send_enabled = on_send_enabled_nus,
    .sent  = on_sent_nus,
};

/****************************************/
static void nus_ccc_cfg_changed_svc(const struct bt_gatt_attr *attr,uint16_t value)
{
	if (nus_cb.send_enabled) {
		LOG_INF("Notification has been turned %s",
			value == BT_GATT_CCC_NOTIFY ? "on" : "off");
		nus_cb.send_enabled(value == BT_GATT_CCC_NOTIFY ?
			BT_NUS_SEND_STATUS_ENABLED : BT_NUS_SEND_STATUS_DISABLED);
	}
}


static ssize_t on_receive_nus_svc(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr,
			  const void *buf,uint16_t len,uint16_t offset,uint8_t flags)
{
	LOG_DBG("Received data, handle %d, conn %p",
		attr->handle, (void *)conn);

	if (nus_cb.received) {
		nus_cb.received(conn, buf, len);
        }
	return len;
}


static void on_sent_nus_svc(struct bt_conn *conn, void *user_data)
{
	ARG_UNUSED(user_data);

	LOG_INF("Data nus send, conn %p", (void *)conn);

	if (nus_cb.sent) {
		nus_cb.sent(conn);
	}
}

static ssize_t read_notify_nus_svc (struct bt_conn *conn, 
                            const struct bt_gatt_attr *attr, void *buf, uint16_t len,  uint16_t offset) 
                            
{
    //uint8_t lvl8 = test_level;
    LOG_INF("Read Notify");
    return bt_gatt_attr_read(conn,attr, buf, len, offset, mfg_notify,sizeof(mfg_notify));
}


/* NUS UART Service Declaration */
BT_GATT_SERVICE_DEFINE(nus_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_NUS_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_NUS_TX, 
                              BT_GATT_CHRC_READ |BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			       read_notify_nus_svc, on_sent_nus_svc, &mfg_notify),
	BT_GATT_CCC(nus_ccc_cfg_changed_svc, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_NUS_RX,
			       BT_GATT_CHRC_WRITE |
			       BT_GATT_CHRC_WRITE_WITHOUT_RESP,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			       NULL, on_receive_nus_svc, NULL),
);


int bt_nus_init(struct bt_nus_cb *callbacks)
{
	if (callbacks) {
            nus_cb.received = callbacks->received;
            nus_cb.sent = callbacks->sent;
            nus_cb.send_enabled = callbacks->send_enabled;
        }
	return 0;
}


int bt_nus_send(struct bt_conn *conn, const uint8_t *data, uint16_t len)
{
	struct bt_gatt_notify_params params = {0};
	const struct bt_gatt_attr *attr = &nus_svc.attrs[2];
	params.attr = attr;
	params.data = data;
	params.len = len;
	params.func = on_sent_nus_svc;

	if (!conn) {
		LOG_DBG("Notification send to all connected peers");
		return bt_gatt_notify_cb(NULL, &params);
	} else if (bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)) {
		return bt_gatt_notify_cb(conn, &params);
	} else {
		return -EINVAL;
	}
}
/* End Nus *********************************************************/



/*BLE Advisery,Connection Declarations & Routines   *************/

static struct bt_conn *default_conn;
static K_SEM_DEFINE(ble_init_ok, 0, 1);

static uint8_t mfg_data[RUUVI_MFG_OFFSET + RUUVI_RAWv2_LEN];

static bool ble_connected = false;

static uint8_t mfg_data[RUUVI_MFG_OFFSET + RUUVI_RAWv2_LEN];
#define BT_LE_ADV BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY, \
				      BT_GAP_ADV_SLOW_INT_MIN, \
                                      BT_GAP_ADV_SLOW_INT_MIN, NULL)

static const struct bt_data ad[] = {
                       BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
                       BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, sizeof(mfg_data)),
};

 static const struct bt_data sd[] = {
                      BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
                      BT_DATA_BYTES(BT_DATA_UUID128_ALL,BT_UUID_NUS_TX_VAL,BT_UUID_DIS_VAL),
	
};



/********************************************************/
static int connection_configuration_set(
			const struct bt_le_conn_param *conn_param)
{
	int err;
	struct bt_conn_info info = {0};

	err = bt_conn_get_info(default_conn, &info);
	if (err) {
		
		return err;
	}

	/*
	err = bt_conn_le_phy_update(default_conn, phy);
	if (err) {
		LOG_WRN("PHY update failed: %d\n", err);
		return err;
	}

	LOG_WRN( "PHY update pending");
        err = k_sem_take(&ble_init_ok, K_FOREVER);
        
	if (err) {
		LOG_WRN( "PHY update timeout");
		return err;
	}

      if (info.le.data_len->tx_max_len != data_len->tx_max_len) {
		data_length_req = true;

		err = bt_conn_le_data_len_update(default_conn, data_len);
		if (err) {
			LOG_WRN( "LE data length update failed: %d",err);
			return err;
		}

		LOG_WRN( "LE Data length update pending");
		
		 err = k_sem_take(&ble_init_ok, K_FOREVER);
       
                if (err) {
			LOG_WRN( "LE Data Length update timeout");
			return err;
		}
	}
*/
	if (info.le.interval != conn_param->interval_max) {
		err = bt_conn_le_param_update(default_conn, conn_param);
		if (err) {
			 LOG_INF("Connection parameters update failed: %d",err);
			return err;
		}

          /* LOG_WRN("Connection parameters update pending");
		 err = k_sem_take(&ble_init_ok, K_FOREVER);
       
		if (err) {
			LOG_WRN("Connection parameters update timeout");
			return err;
		}
	*/
        }

	return 0;
}



/*******************************************************************/
static void connected(struct bt_conn *conn, uint8_t err) {
  char addr[BT_ADDR_LE_STR_LEN];
  
  if (err) {
    LOG_ERR("Connection failed (err %u)", err);
    return;
  }

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  LOG_INF("Connected %s", log_strdup(addr));

  default_conn = bt_conn_ref(conn);

  err = connection_configuration_set(conn_param);
	if (err) {
        LOG_INF("Error");
		return ;
	}
        ble_connected=true;
        toggle_red(1);
}


static void disconnected(struct bt_conn *conn, uint8_t reason) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Disconnected: %s (reason %u)", log_strdup(addr), reason);

  if (default_conn) {
    bt_conn_unref(default_conn);
    default_conn = NULL;
    ble_connected=false;
    toggle_red(0);
  }
}


#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
    enum bt_security_err err) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  if (!err) {
    LOG_INF("Security changed: %s level %u", log_strdup(addr),
        level);
  } else {
    LOG_WRN("Security failed: %s level %u err %d", log_strdup(addr),
        level, err);
  }
}
#endif

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected, 
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
    .security_changed = security_changed,
#endif
};


/*******************************************************************/
#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Passkey for %s: %06u", log_strdup(addr), passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey) {
  char addr[BT_ADDR_LE_STR_LEN];

  default_conn = bt_conn_ref(conn);
  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Passkey for %s: %06u", log_strdup(addr), passkey);
  LOG_INF("Press Button 1 to confirm, Button 2 to reject.");
}

static void auth_cancel(struct bt_conn *conn) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  LOG_INF("Pairing cancelled: %s", log_strdup(addr));
}

static void pairing_confirm(struct bt_conn *conn) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
  bt_conn_auth_pairing_confirm(conn);

  LOG_INF("Pairing confirmed: %s", log_strdup(addr));
}

static void pairing_complete(struct bt_conn *conn, bool bonded) {
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Pairing completed: %s, bonded: %d", log_strdup(addr),
      bonded);
}


static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason){
  char addr[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  LOG_INF("Pairing failed conn: %s, reason %d", log_strdup(addr),
      reason);
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .passkey_display = auth_passkey_display,
    .passkey_confirm = auth_passkey_confirm,
    .cancel = auth_cancel,
    .pairing_confirm = pairing_confirm,
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
#endif
/*******************************************************************/

void error(void) {
  
  while (true) {
    // Spin for ever 
    k_sleep(K_MSEC(1000));
  }
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void num_comp_reply(bool accept) {
      if (accept) {
        bt_conn_auth_passkey_confirm(default_conn);
        LOG_INF("Numeric Match, conn %p", (void *)default_conn);
      } else {
        bt_conn_auth_cancel(default_conn);
        LOG_INF("Numeric Reject, conn %p", (void *)default_conn);
      }

      bt_conn_unref(default_conn);
      default_conn = NULL;
}

void button_changed(uint32_t button_state, uint32_t has_changed) {
      uint32_t buttons = button_state & has_changed;

      if (default_conn) {
        if (buttons & KEY_PASSKEY_ACCEPT) {
          num_comp_reply(true);
        }

        if (buttons & KEY_PASSKEY_REJECT) {
          num_comp_reply(false);
        }
      }
}
#endif /* CONFIG_BT_NUS_SECURITY_ENABLED */


/*   Setting Device Information System **********/
static int settings_runtime_load(void)
{
#if defined(CONFIG_BT_DIS_SETTINGS)
	settings_runtime_set("bt/dis/model",
			     CONFIG_BT_DIS_MODEL,
			     sizeof(CONFIG_BT_DIS_MODEL));
	settings_runtime_set("bt/dis/manuf",
			     CONFIG_BT_DIS_MANUF,
			     sizeof(CONFIG_BT_DIS_MANUF));
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
	settings_runtime_set("bt/dis/serial",
			     CONFIG_BT_DIS_SERIAL_NUMBER_STR,
			     sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
	settings_runtime_set("bt/dis/sw",
			     CONFIG_BT_DIS_SW_REV_STR,
			     sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
	settings_runtime_set("bt/dis/fw",
			     CONFIG_BT_DIS_FW_REV_STR,
			     sizeof(CONFIG_BT_DIS_FW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
	settings_runtime_set("bt/dis/hw",
			     CONFIG_BT_DIS_HW_REV_STR,
			     sizeof(CONFIG_BT_DIS_HW_REV_STR));
#endif
#endif
	return 0;
}


/* Init BLE Connection        */
void bt_init(void){
      int err = 0;

      bt_conn_cb_register(&conn_callbacks);

      if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED)) {
        bt_conn_auth_cb_register(&conn_auth_callbacks);
        }

      err = bt_enable(NULL);
      if (err) {
        error();
        }
      LOG_INF("Bluetooth initialized");

      k_sem_give(&ble_init_ok);

      if (IS_ENABLED(CONFIG_SETTINGS)) {
                    settings_load();
            }

      settings_runtime_load();

      err = bt_nus_init(&nus_cb);
      if (err) {
        LOG_ERR("Failed to initialize UART service (err: %d)", err);
        return;
      }
 
       bt_le_adv_stop();

            err = bt_le_adv_start(BT_LE_ADV, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
            if (err) {
                    LOG_ERR("Advertising failed to start (err %d)", err);
                    return;
            }
            LOG_INF("Advertising started"); 
}



void bt_update_packet(const ble_data_t * const buffer){
if (ble_connected==false)
      {
      memset(mfg_data, 0, sizeof(mfg_data));
      memcpy(mfg_data, buffer->id, 2);
      memcpy(mfg_data + 2, buffer->data, 24);

      //Update advertisement data 
      bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
      return;
      }
  
if (ble_notify_status==true)
      {
      memset(mfg_notify, 0, sizeof(mfg_notify));
      memcpy(mfg_notify, buffer->data, sizeof(mfg_notify));

      bt_gatt_notify(NULL,&nus_svc.attrs[1],&mfg_notify,sizeof(mfg_notify));
      }
}


void ble_write_thread(void) {
  //Don't go any further until BLE is initialized 
  k_sem_take(&ble_init_ok, K_FOREVER);

  for (;;) {
    // Wait indefinitely for data to be sent over bluetooth 
    struct uart_data_t *buf = k_fifo_get(&fifo_uart_rx_data, K_FOREVER);
    LOG_INF("blethread send");
    if (bt_nus_send(NULL, buf->data, buf->len)) {
      LOG_WRN("Failed to send data over BLE connection");
    }
    k_free(buf);
  }
}


K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL,
    NULL, PRIORITY, 0, 0);