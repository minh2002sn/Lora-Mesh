#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdint.h>

void COMMAND_LINE_Init();
void COMMAND_LINE_Handle();
void COMMAND_LINE_Receive(uint8_t rx_data);

#endif
