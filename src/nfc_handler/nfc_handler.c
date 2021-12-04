/*
 * Copyright (c) 2020 theB@STI0N
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <nfc_t2t_lib.h>
#include <nfc/ndef/msg.h>
#include <nfc/ndef/text_rec.h>

#include <ctype.h>
#include <ncs_version.h>
#include <drivers/sensor.h>

#include "led_handler.h"
#include "board_info.h"
#include "ruuvi.h"
#include "battery_handler.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(nfc_handler, CONFIG_RUUVITAG_LOG_LEVEL);

static struct k_work nfc_work;
atomic_t nfc_setup = ATOMIC_INIT(0);

/* Buffer used to hold an NFC NDEF message. */
static uint8_t ndef_msg_buf[RUUVI_NDEF_MSG_BUF_SIZE];

/**
 * @brief Code for each record.
 */
static const uint8_t os_code[] = {'O', 'S'};
static const uint8_t ncs_code[] = {'N', 'C', 'S'};
static const uint8_t fw_code[] = {'F', 'W'};
static const uint8_t ad_code[] = {'A', 'D'};
static const uint8_t id_code[] = {'I', 'D'};
static const uint8_t data_code[] = {'D', 'A'};

/**
 * @brief Buffers for each record.r.
 */
char os_payload[RUUVI_NFC_ZEPHYR_VERSION_LEN];
char ncs_payload[RUUVI_NFC_NCS_VERSION_LEN];
char fw_payload[RUUVI_NFC_FW_VERSION_LEN];
char ad_payload[RUUVI_NFC_AD_LEN];
char id_payload[RUUVI_NFC_SERIAL_LEN];
char data_payload[RUUVI_NFC_DATA_LEN];
uint8_t data_payload_raw[RUUVI_RAWv2_LEN];

static void nfc_callback(void *context,
			 enum nfc_t2t_event event,
			 const uint8_t *data,
			 size_t data_length)
{
	ARG_UNUSED(context);
	ARG_UNUSED(data);
	ARG_UNUSED(data_length);

	switch (event) {
	case NFC_T2T_EVENT_FIELD_ON:
		toggle_red(1);
		break;
	case NFC_T2T_EVENT_FIELD_OFF:
		toggle_red(0);
		break;
	default:
		break;
	}
}

/**
 * @brief Function for filling the static data buffers.
 */
static void nfc_fill_static_data(void){
	strcpy(os_payload, "Zephyr: ");
	strcat(os_payload, CONFIG_ZEPHYR_VERSION);
	strcpy(ncs_payload, "NCS: ");
	strcat(ncs_payload, NCS_VERSION_STRING);
        strcpy(fw_payload, "FW: ");
        strcat(fw_payload, CONFIG_FIRMWARE_VERSION);

    mac_address_bin_t device_mac;
    get_mac(&device_mac);
    sprintf(ad_payload, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
					device_mac.mac[5], device_mac.mac[4], device_mac.mac[3], 
                    device_mac.mac[2], device_mac.mac[1], device_mac.mac[0]);
    
	char device_id[RUUVI_DSN_LENGTH_CHAR];
    get_id(device_id);

	size_t l = strlen(device_id);
	const char **fragments;
	fragments = malloc(sizeof(*fragments) * l / 2);
	int i;

	for (i = 0; i < (l / 2); i++)
	{
		fragments[i] = strndup(device_id + 2 * i, 2);
	}

    sprintf(id_payload, "ID: %s:%s:%s:%s:%s:%s:%s:%s", fragments[0], fragments[1],
				fragments[2], fragments[3],fragments[4], fragments[5], fragments[6], fragments[7]);


}

/**
 * @brief Function for filling the RAWv2 data buffer.
 */
static void nfc_fill_sensor_data(void)
{	
	sprintf(data_payload, 
	"Data: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
			data_payload_raw[0], data_payload_raw[1], data_payload_raw[2], data_payload_raw[4],
			data_payload_raw[5], data_payload_raw[6], data_payload_raw[7], data_payload_raw[8],
			data_payload_raw[9], data_payload_raw[10], data_payload_raw[11], data_payload_raw[12],
			data_payload_raw[13], data_payload_raw[14], data_payload_raw[15], data_payload_raw[16],
			data_payload_raw[17], data_payload_raw[18], data_payload_raw[19], data_payload_raw[20],
			data_payload_raw[21], data_payload_raw[22], data_payload_raw[23]);
}

/**
 * @brief Function for encoding the NDEF text message.
 */
static int nfc_msg_encode(uint8_t *buffer, uint32_t *len)
{
	int err;

	if (!atomic_get(&nfc_setup)){
		nfc_fill_static_data();
		atomic_set(&nfc_setup, 1);
	}
	
	nfc_fill_sensor_data();

	/* Create NFC NDEF text record for Zephyr version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_os_text_rec, UTF_8,
		os_code, sizeof(os_code), os_payload,sizeof(os_payload));

	/* Create NFC NDEF text record for NCS version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_ncs_text_rec, UTF_8,
		ncs_code, sizeof(ncs_code), ncs_payload,sizeof(ncs_payload));

	/* Create NFC NDEF text record for FW version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_sw_text_rec, UTF_8,
		fw_code, sizeof(fw_code), fw_payload, sizeof(fw_payload));
    
    /* Create NFC NDEF text record for tags MAC */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_ad_text_rec, UTF_8,
		ad_code, sizeof(ad_code), ad_payload, sizeof(ad_payload));
    
    /* Create NFC NDEF text record for tags serial */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_id_text_rec, UTF_8,
		id_code, sizeof(id_code), id_payload, sizeof(id_payload));

	/* Create NFC NDEF text record for Ruuvi RAWv2 data */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_data_text_rec, UTF_8,
		data_code, sizeof(data_code), data_payload, sizeof(data_payload));

	/* Create NFC NDEF message description, capacity - RUUVI_MAX_REC_COUNT
	 * records	
	 */
	NFC_NDEF_MSG_DEF(nfc_text_msg, RUUVI_MAX_REC_COUNT);

	/* Add text records to NDEF text messages */
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_os_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add OS record!\n");
		return err;
    }

	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_ncs_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add NCS record!\n");
		return err;
    }
	
	/* Add text records to NDEF text message */
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_sw_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add SW record!\n");
		return err;
    }

	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_ad_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add AD record!\n");
		return err;
    }

	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_id_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add ID record!\n");
		return err;
    }

	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_data_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add data record!\n");
		return err;
    }

	err = nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_text_msg),
				      buffer,
				      len);
	if (err < 0) {
		LOG_ERR("Cannot encode message!\n");
	}

	return err;
}

/**
 * @brief Function allow the controlling of the NFC records on 
 * INIT and UPDATE calls.
 */
static void update_nfc_records(struct k_work *work){
	uint32_t len = sizeof(ndef_msg_buf);
	uint8_t nfc_update = atomic_get(&nfc_setup);
	if(nfc_update){
		/* Stop sensing NFC field */
		if (nfc_t2t_emulation_stop() < 0) {
			LOG_ERR("Cannot stop emulation!");
			return;
		}
		memset(ndef_msg_buf, 0, sizeof(ndef_msg_buf));
	}

	/* Encode messages */
	if (nfc_msg_encode(ndef_msg_buf, &len) < 0) {
		LOG_ERR("Cannot encode message!");
	}

	/* Set created message as the NFC payload */
	if (nfc_t2t_payload_set(ndef_msg_buf, len) < 0) {
		LOG_ERR("Cannot set payload!");
	}

	/* Start sensing NFC field */
	if (nfc_t2t_emulation_start() < 0) {
		LOG_ERR("Cannot start emulation!");
	}
	
	if (!nfc_update){
		LOG_INF("NFC configuration done");
	}
	else{
		LOG_DBG("NFC Updated");
	}	
}

/**
 * @brief Function for updating NFC records..
 */
void nfc_update(const ble_data_t * const buffer){
	memcpy(data_payload_raw, buffer->data, sizeof(data_payload_raw));
	k_work_submit(&nfc_work);
}

/**
 * @brief Function for setting up NFC.
 */
void nfc_init(void){
	/* Set up NFC */
	if (nfc_t2t_setup(nfc_callback, NULL) < 0) {
		LOG_ERR("Cannot setup NFC T2T library!\n");
		return;
	}

	k_work_init(&nfc_work, update_nfc_records);

	k_work_submit(&nfc_work);
}
