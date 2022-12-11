#ifndef UART_H
#define UART_H

//#include "read_time.h"
#include "stdint.h"
#include "main.h"
#include "ring_buffer.h"

void UART_Init();
void UART_Handle();
uint8_t UART_Read();
void UART_Receive(uint8_t Rx_Buffer);
uint16_t UART_Available();

#endif
