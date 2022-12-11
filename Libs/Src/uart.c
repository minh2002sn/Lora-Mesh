#include "uart.h"
#include "Frame_Sync.h"

#define UART_MAX_LEN 255

static uint8_t uart_buffer[UART_MAX_LEN];
RING_BUFFER_HandleTypeDef uart_ring_buffer;

extern UART_HandleTypeDef huart2;

void UART_Init(){
	RING_BUFFER_Init(&uart_ring_buffer, uart_buffer, UART_MAX_LEN);
}

void UART_Receive(uint8_t Rx_Buffer){
	RING_BUFFER_Push(&uart_ring_buffer, Rx_Buffer);
}

__weak void UART_Handle(){
	if(UART_Available() != 0){
		uint8_t t_data = UART_Read();
		FRAME_SYNC_Receive(t_data);
	}
}

uint8_t UART_Read(){
	uint8_t t_data;
	RING_BUFFER_Pop(&uart_ring_buffer, &t_data);
	return t_data;
}

uint16_t UART_Available(){
	return RING_BUFFER_Available(&uart_ring_buffer);
}
