#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nfc_t2t_lib.h>
#include <nfc/ndef/msg.h>
#include <nfc/ndef/text_rec.h>
#include <ctype.h>

#include "led_handler.h"
#include "board_info.h"
#include "ruuvi.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(nfc_handler, CONFIG_RUUVITAG_LOG_LEVEL);

/* Buffer used to hold an NFC NDEF message. */
static uint8_t ndef_msg_buf[NDEF_MSG_BUF_SIZE];

/* Create NFC NDEF message description, capacity - MAX_REC_COUNT
	 * records
	 */
NFC_NDEF_MSG_DEF(nfc_text_msg, MAX_REC_COUNT);

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
 * @brief Function for encoding the NDEF text message.
 */
static int ruuvi_nfc_msg_encode(uint8_t *buffer, uint32_t *len)
{
	int err;

	static const uint8_t os_code[] = {'O', 'S'};
    char os_payload[] = "Zephyr 2.4.0";

	static const uint8_t fw_code[] = {'S', 'W'};
    char fw_payload[fw_payload_len];
    strcpy(fw_payload, "SW: ");
    strcat(fw_payload, CONFIG_RUUVITAG_APP_VERSION);
    
	static const uint8_t ad_code[] = {'A', 'D'};
    mac_address_bin_t device_mac;
    get_mac(&device_mac);
    char ad_payload[ad_payload_len];
    sprintf(ad_payload, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", device_mac.mac[5], device_mac.mac[4], device_mac.mac[3], 
                                                        device_mac.mac[2], device_mac.mac[1], device_mac.mac[0]);
    
	static const uint8_t id_code[] = {'I', 'D'};
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

    char id_payload[id_payload_len];
    sprintf(id_payload, "ID: %s:%s:%s:%s:%s:%s:%s:%s", fragments[0], fragments[1],
				fragments[2], fragments[3],fragments[4], fragments[5], fragments[6], fragments[7]);
	
	/* Create NFC NDEF text record for FW version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_os_text_rec, UTF_8,
		os_code, sizeof(os_code), os_payload,sizeof(os_payload));

	/* Create NFC NDEF text record for FW version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_sw_text_rec, UTF_8,
		fw_code, sizeof(fw_code), fw_payload, sizeof(fw_payload));
    
    /* Create NFC NDEF text record for FW version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_ad_text_rec, UTF_8,
		ad_code, sizeof(ad_code), ad_payload, sizeof(ad_payload));
    
    /* Create NFC NDEF text record for FW version */
	NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_id_text_rec, UTF_8,
		id_code, sizeof(id_code), id_payload, sizeof(id_payload));

	/* Add text records to NDEF text message */
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_os_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add OS record!\n");
		return err;
    }
	
	/* Add text records to NDEF text message */
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_sw_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add SW record!\n");
		return err;
    }


    /* Add text records to NDEF text message */
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_ad_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add AD record!\n");
		return err;
    }

    /* Add text records to NDEF text message */
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_text_msg),
				   &NFC_NDEF_TEXT_RECORD_DESC(nfc_id_text_rec));
	if (err < 0) {
		LOG_ERR("Cannot add ID record!\n");
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

int ruuvi_nfc_init(void){
    uint32_t len = sizeof(ndef_msg_buf);

    /* Set up NFC */
	if (nfc_t2t_setup(nfc_callback, NULL) < 0) {
		LOG_ERR("Cannot setup NFC T2T library!\n");
		goto fail;
	}


	/* Encode welcome message */
	if (ruuvi_nfc_msg_encode(ndef_msg_buf, &len) < 0) {
		LOG_ERR("Cannot encode message!\n");
		goto fail;
	}


	/* Set created message as the NFC payload */
	if (nfc_t2t_payload_set(ndef_msg_buf, len) < 0) {
		LOG_ERR("Cannot set payload!\n");
		goto fail;
	}


	/* Start sensing NFC field */
	if (nfc_t2t_emulation_start() < 0) {
		LOG_ERR("Cannot start emulation!\n");
		goto fail;
	}
	LOG_INF("NFC configuration done\n");

	return 0;

fail:
    return -EIO;
}