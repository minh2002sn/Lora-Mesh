#include "Frame_Sync.h"
#include "checksum.h"
#include "SX1278.h"

#define MY_ID						0

#define WAITING_ACK_TIME			10000
#define WAITING_REPLY_TIME			10000
#define	WAITING_NEXT_BYTE_TIME		500

#define FRAME_SYNC_huart	huart2

extern UART_HandleTypeDef FRAME_SYNC_huart;
extern SX1278_t hlora;

enum{
	STX = 0x02,
	ETX = 0x03,
	DLE = 0x10,
	ACK = 0xFF,
	NACK = 0xFE,
};

enum
{
	SEARCHING_STX,
	RECEIVING_FINAL_DES_ID,
	RECEIVING_TEMP_DES_ID,
	RECEIVING_SRC_ID,
	RECEIVING_TIME_TO_LIVE,
	RECEIVING_DATA_LEN,
	RECEIVING_DATA,
	RECEIVING_CRC,
	RECEIVING_ETX,
};

FRAME_SYNC_DATA_t FS_Data = {MY_ID};

__weak void FRAME_SYNC_Byte_Transmit(uint8_t tx_data)
{
	HAL_UART_Transmit(&FRAME_SYNC_huart, &tx_data, 1, 100);
}

static void Rx_Reset()
{
	FS_Data.rx_checksum = 0xFFFFFFFF;
	FS_Data.rx_state = SEARCHING_STX;
	FS_Data.rx_num_crc_byte = 0;
	FS_Data.rx_packet.crc32 = 0;
	FS_Data.rx_pointer = 0;
}

static void CRC_Update(uint32_t *crc, uint8_t data)
{
	*crc = update_crc_32(*crc, data);
}

static void validate_packet()
{
	if(FS_Data.rx_packet.temp_des_id != FS_Data.my_id) return;
	uint8_t temp_data = 0;
	FS_Data.rx_checksum = ~FS_Data.rx_checksum;
	if(FS_Data.rx_packet.crc32 != FS_Data.rx_checksum)
	{
		temp_data = NACK;
		FRAME_SYNC_Transmit(FS_Data.rx_packet.src_id, FS_Data.rx_packet.src_id, 1, &temp_data, 1, 0);
		FRAME_SYNC_RxFailCallback(FS_Data.rx_packet.buffer, FS_Data.rx_packet.length);
		Rx_Reset();
		return;
	}

	switch(FS_Data.rx_packet.buffer[0])
	{
		case ACK:	// Received ACK
			if(FS_Data.device_state == FRAME_SYNC_WAITING_ACK)
			{
				FS_Data.device_state = FRAME_SYNC_READY_TO_TRANSMIT;
				if(FS_Data.stored_packet.is_requiring_reply == 1)
				{
					FS_Data.device_state = FRAME_SYNC_WAITING_REPLY;
				}
			}
			break;
		case NACK:	// Received NACK
			// Re-transmit
			if(FS_Data.device_state == FRAME_SYNC_WAITING_ACK)
			{
				FS_Data.device_state = FRAME_SYNC_READY_TO_TRANSMIT;
				FRAME_SYNC_Transmit(FS_Data.stored_packet.final_des_id, FS_Data.stored_packet.temp_des_id, FS_Data.stored_packet.time_to_live - 1,
						FS_Data.stored_packet.buffer, FS_Data.stored_packet.length, FS_Data.stored_packet.is_requiring_reply);
			}
			break;
		default:	// Received normal data
			if(FS_Data.rx_packet.temp_des_id == FS_Data.my_id)
			{
				if(FS_Data.device_state == FRAME_SYNC_WAITING_REPLY)
				{
					FS_Data.device_state = FRAME_SYNC_READY_TO_TRANSMIT;
				}
				temp_data = ACK;
				FRAME_SYNC_Transmit(FS_Data.rx_packet.src_id, FS_Data.rx_packet.src_id, 1, &temp_data, 1, 0);
				if(FS_Data.rx_packet.final_des_id != FS_Data.my_id)
				{
					FRAME_SYNC_Transmit(FS_Data.rx_packet.final_des_id, FS_Data.rx_packet.temp_des_id + 1, FS_Data.rx_packet.time_to_live - 1,
							FS_Data.rx_packet.buffer, FS_Data.rx_packet.length, FS_Data.stored_packet.is_requiring_reply);
				}
			}

			break;
	}
	FRAME_SYNC_RxCpltCallback(FS_Data.rx_packet.buffer, FS_Data.rx_packet.length);
	Rx_Reset();
}

void FRAME_SYNC_Init()
{

}

void FRAME_SYNC_Change_Setting(FRAME_SYNC_DATA_t *p_new_data)
{
	FS_Data = *p_new_data;
}

void FRAME_SYNC_Transmit(uint8_t final_des_id, uint8_t temp_des_id, uint8_t time_to_live, uint8_t *tx_frame, uint8_t size, uint8_t is_requiring_reply)
{
	uint8_t temp_packet[MAX_LENGTH_DATA] = {STX};
	uint8_t packet_length = 1;
	if(FS_Data.device_state != FRAME_SYNC_READY_TO_TRANSMIT) return;
	if(time_to_live <= 0) return;
	FS_Data.tx_checksum = 0xFFFFFFFF;

	// Transmit STX
//	FRAME_SYNC_Byte_Transmit(STX);

	// Transmit final destination device id
	CRC_Update(&FS_Data.tx_checksum, final_des_id);
//	FRAME_SYNC_Byte_Transmit(final_des_id);
	temp_packet[packet_length++] = final_des_id;

	// Transmit temporary destination device id
	CRC_Update(&FS_Data.tx_checksum, temp_des_id);
//	FRAME_SYNC_Byte_Transmit(temp_des_id);
	temp_packet[packet_length++] = temp_des_id;

	// Transmit source device id
	CRC_Update(&FS_Data.tx_checksum, FS_Data.my_id);
//	FRAME_SYNC_Byte_Transmit(FS_Data.my_id);
	temp_packet[packet_length++] = FS_Data.my_id;

	// Transmit time to live
	CRC_Update(&FS_Data.tx_checksum, time_to_live);
//	FRAME_SYNC_Byte_Transmit(time_to_live);
	temp_packet[packet_length++] = time_to_live;

	// Transmit data length
	CRC_Update(&FS_Data.tx_checksum, size);
//	FRAME_SYNC_Byte_Transmit(size);
	temp_packet[packet_length++] = size;

	// Transmit data
	for(int i = 0; i < size; i++)
	{
		CRC_Update(&FS_Data.tx_checksum, tx_frame[i]);
//		FRAME_SYNC_Byte_Transmit(tx_frame[i]);
		FS_Data.stored_packet.buffer[i] = tx_frame[i];
		temp_packet[packet_length++] = tx_frame[i];
	}

	// Transmit crc
	FS_Data.tx_checksum = ~FS_Data.tx_checksum;
//	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 0));
//	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 1));
//	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 2));
//	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 3));
	temp_packet[packet_length++] = *((uint8_t *)&FS_Data.tx_checksum + 0);
	temp_packet[packet_length++] = *((uint8_t *)&FS_Data.tx_checksum + 1);
	temp_packet[packet_length++] = *((uint8_t *)&FS_Data.tx_checksum + 2);
	temp_packet[packet_length++] = *((uint8_t *)&FS_Data.tx_checksum + 3);

	// Transmit ETX
//	FRAME_SYNC_Byte_Transmit(ETX);
	temp_packet[packet_length++] = ETX;

	if(size != 1 && *tx_frame != ACK && *tx_frame != NACK)
	{
		FS_Data.device_state = FRAME_SYNC_WAITING_ACK;
		FS_Data.stored_packet.final_des_id = final_des_id;
		FS_Data.stored_packet.temp_des_id = temp_des_id;
		FS_Data.stored_packet.src_id = FS_Data.my_id;
		FS_Data.stored_packet.time_to_live = time_to_live;
		FS_Data.stored_packet.length = size;
		for(int i = 0; i < size; i++)
		{
			FS_Data.stored_packet.buffer[i] = tx_frame[i];
		}
		FS_Data.stored_packet.crc32 = FS_Data.tx_checksum;
		FS_Data.stored_packet.is_requiring_reply = is_requiring_reply;
	}

	FS_Data.last_transmit_frame_timer = HAL_GetTick();

	FRAME_SYNC_Packet_Transmit(temp_packet, packet_length);
}

void FRAME_SYNC_Receive(uint8_t rx_data)
{
	switch(FS_Data.rx_state)
	{
		case SEARCHING_STX:
			if(rx_data == STX)
			{
				FS_Data.rx_checksum = 0xFFFFFFFF;
				FS_Data.rx_state = RECEIVING_FINAL_DES_ID;
			}
			else
			{
				Rx_Reset();
				FRAME_SYNC_RxFailCallback(FS_Data.rx_packet.buffer, FS_Data.rx_packet.length);
			}
			break;
		case RECEIVING_FINAL_DES_ID:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_packet.final_des_id = rx_data;
			FS_Data.rx_state = RECEIVING_TEMP_DES_ID;
			break;
		case RECEIVING_TEMP_DES_ID:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_packet.temp_des_id = rx_data;
			FS_Data.rx_state = RECEIVING_SRC_ID;
			break;
		case RECEIVING_SRC_ID:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_packet.src_id = rx_data;
			FS_Data.rx_state = RECEIVING_TIME_TO_LIVE;
			break;
		case RECEIVING_TIME_TO_LIVE:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_packet.time_to_live = rx_data;
			FS_Data.rx_state = RECEIVING_DATA_LEN;
			break;
		case RECEIVING_DATA_LEN:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_packet.length = rx_data;
			FS_Data.rx_state = RECEIVING_DATA;
			break;
		case RECEIVING_DATA:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_packet.buffer[FS_Data.rx_pointer++] = rx_data;
			if(FS_Data.rx_pointer == FS_Data.rx_packet.length)
			{
				FS_Data.rx_state = RECEIVING_CRC;
			}
			break;
		case RECEIVING_CRC:
			FS_Data.rx_packet.crc32 |= (uint32_t)rx_data << ((FS_Data.rx_num_crc_byte) * 8);
			FS_Data.rx_num_crc_byte++;
			if(FS_Data.rx_num_crc_byte == 4)
			{
				FS_Data.rx_state = RECEIVING_ETX;
			}
			break;
		case RECEIVING_ETX:
			if(rx_data == ETX)
			{
				validate_packet();
			}
			else
			{
				Rx_Reset();
				FRAME_SYNC_RxFailCallback(FS_Data.rx_packet.buffer, FS_Data.rx_packet.length);
			}
			break;
		default:
			break;
	}

	FS_Data.last_receive_byte_timer = HAL_GetTick();
}

void FRAME_SYNC_Handle(){
	if(FS_Data.rx_state != SEARCHING_STX && (HAL_GetTick() - FS_Data.last_receive_byte_timer > WAITING_NEXT_BYTE_TIME))
	{
		Rx_Reset();
		FRAME_SYNC_RxFailCallback(FS_Data.rx_packet.buffer, FS_Data.rx_packet.length);
	}

	if(FS_Data.device_state == FRAME_SYNC_WAITING_ACK && (HAL_GetTick() - FS_Data.last_transmit_frame_timer > WAITING_ACK_TIME))
	{
		if(FS_Data.stored_packet.time_to_live - 1 != 0)
		{
			FS_Data.device_state = FRAME_SYNC_READY_TO_TRANSMIT;
			FRAME_SYNC_Transmit(FS_Data.stored_packet.final_des_id, FS_Data.stored_packet.temp_des_id, FS_Data.stored_packet.time_to_live - 1,
					FS_Data.stored_packet.buffer, FS_Data.stored_packet.length, FS_Data.stored_packet.is_requiring_reply);
		}
		else
		{
			FS_Data.device_state = FRAME_SYNC_READY_TO_TRANSMIT;
		}
	}

	if(FS_Data.device_state == FRAME_SYNC_WAITING_REPLY && (HAL_GetTick() - FS_Data.last_transmit_frame_timer > WAITING_REPLY_TIME))
	{
		FS_Data.device_state = FRAME_SYNC_READY_TO_TRANSMIT;
	}

	FRAME_SYNC_Packet_Receive();
}
