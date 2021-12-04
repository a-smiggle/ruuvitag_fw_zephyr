/*
 * Copyright (c) 2021 AL
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef BT_HANDLER_H_
#define BT_HANDLER_H_

#include <zephyr/types.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "ruuvi.h"

/** @brief UUID of the NUS Service. **/
#define BT_UUID_NUS_VAL \
	BT_UUID_128_ENCODE(0x6e400001, 0xb5a3, 0xf393, 0xe0a9, 0xe50e24dcca9e)

/** @brief UUID of the TX Characteristic. **/
#define BT_UUID_NUS_TX_VAL \
	BT_UUID_128_ENCODE(0x6e400003, 0xb5a3, 0xf393, 0xe0a9, 0xe50e24dcca9e)

/** @brief UUID of the RX Characteristic. **/
#define BT_UUID_NUS_RX_VAL \
	BT_UUID_128_ENCODE(0x6e400002, 0xb5a3, 0xf393, 0xe0a9, 0xe50e24dcca9e)

#define BT_UUID_NUS_SERVICE   BT_UUID_DECLARE_128(BT_UUID_NUS_VAL)
#define BT_UUID_NUS_RX        BT_UUID_DECLARE_128(BT_UUID_NUS_RX_VAL)
#define BT_UUID_NUS_TX        BT_UUID_DECLARE_128(BT_UUID_NUS_TX_VAL)


// @brief NUS send status. 
enum bt_nus_send_status {
	// Send notification enabled. 
	BT_NUS_SEND_STATUS_ENABLED,
	// Send notification disabled. 
	BT_NUS_SEND_STATUS_DISABLED,
};


/** @brief Pointers to the callback functions for service events. */
struct bt_nus_cb {
	/** @brief Data received callback.
	 *
	 * The data has been received as a write request on the NUS RX
	 * Characteristic.
	 *
	 * @param[in] conn  Pointer to connection object that has received data.
	 * @param[in] data  Received data.
	 * @param[in] len   Length of received data.
	 */
	void (*received)(struct bt_conn *conn,
			 const uint8_t *const data, uint16_t len);

	/** @brief Data sent callback.
	 *
	 * The data has been sent as a notification and written on the NUS TX
	 * Characteristic.
	 *
	 * @param[in] conn Pointer to connection object, or NULL if sent to all
	 *                 connected peers.
	 */
	void (*sent)(struct bt_conn *conn);

	/** @brief Send state callback.
	 *
	 * Indicate the CCCD descriptor status of the NUS TX characteristic.
	 *
	 * @param[in] status Send notification status.
	 */
	void (*send_enabled)(enum bt_nus_send_status status);

};

/**@brief Initialize the service.
 *
 * @details This function registers a GATT service with two characteristics,
 *          TX and RX. A remote device that is connected to this service
 *          can send data to the RX Characteristic. When the remote enables
 *          notifications, it is notified when data is sent to the TX
 *          Characteristic.
 *
 * @param[in] callbacks  Struct with function pointers to callbacks for service
 *                       events. If no callbacks are needed, this parameter can
 *                       be NULL.
 *
 * @retval 0 If initialization is successful.
 *           Otherwise, a negative value is returned.
 */
int bt_nus_init(struct bt_nus_cb *callbacks);

/**@brief Send data.
 *
 * @details This function sends data to a connected peer, or all connected
 *          peers.
 *
 * @param[in] conn Pointer to connection object, or NULL to send to all
 *                 connected peers.
 * @param[in] data Pointer to a data buffer.
 * @param[in] len  Length of the data in the buffer.
 *
 * @retval 0 If the data is sent.
 *           Otherwise, a negative value is returned.
 */
int bt_nus_send(struct bt_conn *conn, const uint8_t *data, uint16_t len);

/**@brief Get maximum data length that can be used for @ref bt_nus_send.
 *
 * @param[in] conn Pointer to connection Object.
 *
 * @return Maximum data length.
 */
static inline uint32_t bt_nus_get_mtu(struct bt_conn *conn)
{
	/* According to 3.4.7.1 Handle Value Notification off the ATT protocol.
	 * Maximum supported notification is ATT_MTU - 3 */
	return bt_gatt_get_mtu(conn) - 3;
}


void bas_notify(void);
void bt_init(void);
void bt_update_packet(const ble_data_t * const buffer);

#endif /* BT_HANDLER_H_ */