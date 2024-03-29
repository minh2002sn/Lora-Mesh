#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "stdint.h"
#include "Frame_Sync.h"

typedef struct{
	PACKET_STRUCTURE *buffer;
	uint16_t head;
	uint16_t tail;
	uint16_t max_length;
}RING_BUFFER_HandleTypeDef;

void RING_BUFFER_Init(RING_BUFFER_HandleTypeDef *p_ring_buffer, PACKET_STRUCTURE *p_buffer, uint16_t p_max_length);
int8_t RING_BUFFER_Push(RING_BUFFER_HandleTypeDef *p_ring_buffer, PACKET_STRUCTURE p_data);
int8_t RING_BUFFER_Pop(RING_BUFFER_HandleTypeDef *p_ring_buffer, PACKET_STRUCTURE *p_data);
uint16_t RING_BUFFER_Available(RING_BUFFER_HandleTypeDef *p_ring_buffer);

#endif
