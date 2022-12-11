#include "ring_buffer.h"

void RING_BUFFER_Init(RING_BUFFER_HandleTypeDef *p_ring_buffer, uint8_t * p_buffer, uint16_t p_max_length){
	p_ring_buffer->buffer = p_buffer;
	p_ring_buffer->head = 0;
	p_ring_buffer->tail = 0;
	p_ring_buffer->max_length = p_max_length;
}

int8_t RING_BUFFER_Push(RING_BUFFER_HandleTypeDef *p_ring_buffer, uint8_t p_data){
	uint16_t t_next;

	t_next = p_ring_buffer->head + 1;
	if(t_next >= p_ring_buffer->max_length)
		t_next = 0;

	if(t_next == p_ring_buffer->tail)
		return -1;

	p_ring_buffer->buffer[p_ring_buffer->head] = p_data;
	p_ring_buffer->head = t_next;

	return 0;
}

int8_t RING_BUFFER_Pop(RING_BUFFER_HandleTypeDef *p_ring_buffer, uint8_t * p_data){
	uint16_t t_next;

	if(p_ring_buffer->tail == p_ring_buffer->head)
		return -1;

	t_next = p_ring_buffer->tail + 1;
	if(t_next >= p_ring_buffer->max_length)
		t_next = 0;

	*p_data = p_ring_buffer->buffer[p_ring_buffer->tail];
	p_ring_buffer->tail = t_next;

	return 0;
}

uint16_t RING_BUFFER_Available(RING_BUFFER_HandleTypeDef *p_ring_buffer){
//	if(p_ring_buffer->head > p_ring_buffer->tail){
//		return p_ring_buffer->max_length - (p_ring_buffer->head - p_ring_buffer->tail);
//	} else if(p_ring_buffer->head < p_ring_buffer->tail){
//		return p_ring_buffer->max_length - (p_ring_buffer->head - p_ring_buffer->tail);
//	} else if(p_ring_buffer->head == p_ring_buffer->tail){
//		return 0;
//	}
	return (p_ring_buffer->head == p_ring_buffer->tail) ? 0 : 1;
}
