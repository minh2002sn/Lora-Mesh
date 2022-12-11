#include "Frame_Sync.h"
#include "checksum.h"

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
	RECEIVING_DES_ID,
	RECEIVING_SRC_ID,
	RECEIVING_TIME_TO_LIVE,
	RECEIVING_DATA_LEN,
	RECEIVING_DATA,
	RECEIVING_CRC,
	RECEIVING_ETX,
};

#define FRAME_SYNC_huart	huart2

extern UART_HandleTypeDef FRAME_SYNC_huart;

FRAME_SYNC_DATA_t FS_Data = {};

__weak void FRAME_SYNC_Byte_Transmit(uint8_t tx_data)
{
	HAL_UART_Transmit(&FRAME_SYNC_huart, &tx_data, 1, 100);
}

static void Rx_Reset()
{
	FS_Data.rx_checksum = 0xFFFFFFFF;
	FS_Data.rx_state = SEARCHING_STX;
	FS_Data.rx_num_crc_byte = 0;
	FS_Data.rx_crc = 0;
	FS_Data.rx_pointer = 0;
}

static void CRC_Update(uint32_t *crc, uint8_t data)
{
	*crc = update_crc_32(*crc, data);
}

static void validate_packet()
{
	uint8_t temp_data = 0;
	FS_Data.rx_checksum = ~FS_Data.rx_checksum;
	if(FS_Data.rx_crc != FS_Data.rx_checksum)
	{
		temp_data = NACK;
		FRAME_SYNC_Transmit(FS_Data.rx_src_id, 1, &temp_data, 1);
		FRAME_SYNC_RxFailCallback(FS_Data.rx_buf, FS_Data.rx_length);
		Rx_Reset();
		return;
	}

	switch(FS_Data.rx_buf[0])
	{
		case ACK:
			if(FS_Data.device_state == WAITING_ACK)
			{
				FS_Data.device_state = READY_TO_TRANSMIT;
				FRAME_SYNC_RxCpltCallback(FS_Data.rx_buf, FS_Data.rx_length);
			}
			break;
		case NACK:
			// Re-transmit
			if(FS_Data.device_state == WAITING_ACK)
			{
				FRAME_SYNC_RxCpltCallback(FS_Data.rx_buf, FS_Data.rx_length);
			}
			break;
		default:
			temp_data = ACK;
			FRAME_SYNC_Transmit(FS_Data.rx_src_id, 1, &temp_data, 1);
			FRAME_SYNC_RxCpltCallback(FS_Data.rx_buf, FS_Data.rx_length);
			break;
	}
	Rx_Reset();
}

void FRAME_SYNC_Init()
{

}

void FRAME_SYNC_Change_Setting(FRAME_SYNC_DATA_t *p_new_data)
{
	FS_Data = *p_new_data;
}

void FRAME_SYNC_Transmit(uint8_t des_id, uint8_t time_to_live, uint8_t *tx_frame, uint8_t size)
{
	if(FS_Data.device_state != READY_TO_TRANSMIT) return;
	if(time_to_live == 0) return;
	FS_Data.tx_checksum = 0xFFFFFFFF;

	// Transmit STX
	FRAME_SYNC_Byte_Transmit(STX);

	// Transmit destination device id
	CRC_Update(&FS_Data.tx_checksum, des_id);
	FRAME_SYNC_Byte_Transmit(des_id);

	// Transmit source device id
	CRC_Update(&FS_Data.tx_checksum, FS_Data.id);
	FRAME_SYNC_Byte_Transmit(FS_Data.id);

	// Transmit time to live
	CRC_Update(&FS_Data.tx_checksum, time_to_live);
	FRAME_SYNC_Byte_Transmit(time_to_live);

	// Transmit data length
	CRC_Update(&FS_Data.tx_checksum, size);
	FRAME_SYNC_Byte_Transmit(size);

	// Transmit data
	for(int i = 0; i < size; i++)
	{
		CRC_Update(&FS_Data.tx_checksum, tx_frame[i]);
		FRAME_SYNC_Byte_Transmit(tx_frame[i]);
	}

	// Transmit crc
	FS_Data.tx_checksum = ~FS_Data.tx_checksum;
	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 0));
	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 1));
	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 2));
	FRAME_SYNC_Byte_Transmit(*((uint8_t *)&FS_Data.tx_checksum + 3));

	// Transmit ETX
	FRAME_SYNC_Byte_Transmit(ETX);

	FS_Data.device_state = WAITING_ACK;
}

void FRAME_SYNC_Receive(uint8_t rx_data)
{
	switch(FS_Data.rx_state)
	{
		case SEARCHING_STX:
			if(rx_data == STX)
			{
				FS_Data.rx_checksum = 0xFFFFFFFF;
				FS_Data.rx_state = RECEIVING_DES_ID;
			}
			else
			{
				Rx_Reset();
				FRAME_SYNC_RxFailCallback(FS_Data.rx_buf, FS_Data.rx_length);
			}
			break;
		case RECEIVING_DES_ID:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_des_id = rx_data;
			FS_Data.rx_state = RECEIVING_SRC_ID;
			break;
		case RECEIVING_SRC_ID:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_src_id = rx_data;
			FS_Data.rx_state = RECEIVING_TIME_TO_LIVE;
			break;
		case RECEIVING_TIME_TO_LIVE:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_time_to_live = rx_data;
			FS_Data.rx_state = RECEIVING_DATA_LEN;
			break;
		case RECEIVING_DATA_LEN:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_length = rx_data;
			FS_Data.rx_state = RECEIVING_DATA;
			break;
		case RECEIVING_DATA:
			CRC_Update(&FS_Data.rx_checksum, rx_data);
			FS_Data.rx_buf[FS_Data.rx_pointer++] = rx_data;
			if(FS_Data.rx_pointer == FS_Data.rx_length)
			{
				FS_Data.rx_state = RECEIVING_CRC;
			}
			break;
		case RECEIVING_CRC:
			FS_Data.rx_crc |= (uint32_t)rx_data << ((FS_Data.rx_num_crc_byte) * 8);
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
				FRAME_SYNC_RxFailCallback(FS_Data.rx_buf, FS_Data.rx_length);
			}
			break;
		default:
			break;
	}

	FS_Data.last_receive_byte_timer = HAL_GetTick();

}

void FRAME_SYNC_Handle(){
//	if(FS_Data.rx_state != SEARCHING_STX && (HAL_GetTick() - FS_Data.last_receive_byte_timer > 500))
//	{
//		Rx_Reset();
//		FRAME_SYNC_RxFailCallback(FS_Data.rx_buf, FS_Data.rx_length);
//	}
}
