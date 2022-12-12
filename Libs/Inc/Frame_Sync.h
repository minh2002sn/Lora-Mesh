#ifndef FRAME_SYNC_H
#define FRAME_SYNC_H

#include "main.h"

#define MAX_LENGTH_DATA 255

enum{
	FRAME_SYNC_READY_TO_TRANSMIT,
	FRAME_SYNC_WAITING_ACK,
	FRAME_SYNC_WAITING_REPLY,
};

typedef struct
{
	//	Common variable of device
	uint8_t my_id;											// ID of this device
	uint8_t device_state;

	// Transmitting variable
	uint32_t tx_checksum;									// Transmiting crc bits

	// Receiving variable
	uint32_t rx_checksum;									// Reveiving crc bits
	uint8_t rx_state;										// Receiving state
	uint8_t rx_pointer;										// Where next byte is store in rx_buf
	uint8_t rx_final_des_id;								// ID of final destination device that packet is transmitted to
	uint8_t rx_temp_des_id;									// ID of temporary destination device that packet is transmitted to
	uint8_t rx_src_id;										// ID of source device that transmit packet
	uint8_t rx_time_to_live;								// Max number of transmit time of packet
	uint8_t rx_length;										// Number of data byte in packet
	uint8_t rx_num_crc_byte;								// Number of CRC byte received
	uint8_t rx_buf[MAX_LENGTH_DATA];						// Buffer to store data in received packet
	uint32_t rx_crc;										// Received CRC string
	uint32_t last_receive_byte_timer;						// Last time receiving 1 byte

	// Re-transmiting variable
	uint32_t last_transmit_frame_timer;						// Last time transmitting 1 packet
	uint8_t tx_stored_packet_buf[MAX_LENGTH_DATA];			// Buffer to store the last transmitting packet
	uint8_t tx_stored_packet_lenght;						// Last transmitting packet length
} FRAME_SYNC_DATA_t;

void FRAME_SYNC_Init();
void FRAME_SYNC_Change_Setting(FRAME_SYNC_DATA_t *p_new_data);
void FRAME_SYNC_Transmit(uint8_t final_des_id, uint8_t temp_des_id, uint8_t time_to_live, uint8_t *tx_frame, uint8_t size);
void FRAME_SYNC_Receive(uint8_t rx_data);
void FRAME_SYNC_RxCpltCallback(uint8_t *p_rx_data, uint8_t data_size);
void FRAME_SYNC_RxFailCallback(uint8_t *p_rx_data, uint8_t data_size);
void FRAME_SYNC_Handle();

void FRAME_SYNC_Byte_Transmit(uint8_t tx_data);

#endif
