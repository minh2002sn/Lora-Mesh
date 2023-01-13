#include "Command_Line.h"
#include "Frame_Sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern UART_HandleTypeDef huart1;
extern uint8_t tx_frame_data[255];
extern uint8_t tx_frame_len;

uint8_t cl_buf[255];
uint8_t cl_pointer = 0;
uint8_t cl_flag = 0;

static uint8_t str2hex(char *str)
{
	uint8_t result = 0;
	for(int i = 0; i < 2; i++)
	{
		uint8_t temp_data;
		if(str[i] >= '0' && str[i] <= '9')
		{
			temp_data = str[i] - '0';
		}
		else if(str[i] >= 'A' && str[i] <= 'F')
		{
			temp_data = str[i] - 55;
		}
		else if(str[i] >= 'a' && str[i] <= 'f')
		{
			temp_data = str[i] - 87;
		}
		result |= temp_data << ((1 - i) * 4);
	}
	return result;
}

void COMMAND_LINE_Init()
{

}

void COMMAND_LINE_Handle(){
	if(cl_flag)
	{
		char *arg_list[10];
		uint8_t arg_num = 0;

		char *temp_token = strtok((char *)cl_buf, " ");
		while(temp_token != NULL)
		{
			arg_list[arg_num++] = temp_token;
			temp_token = strtok(NULL, " ");
		}

		if(strstr(arg_list[0], "DATA") != NULL)
		{
			tx_frame_len = arg_num - 1;
			for(int i = 0; i < arg_num - 1; i++)
			{
				tx_frame_data[i] = str2hex(arg_list[i + 1]);
			}
			uint8_t temp_str[] = "OK\n";
			HAL_UART_Transmit(&huart1, temp_str, 3, 100);
		}

		cl_pointer = 0;
		cl_flag = 0;
	}
}

void COMMAND_LINE_Receive(uint8_t rx_data)
{
	if(rx_data != '\n')
	{
		cl_buf[cl_pointer++] = rx_data;
	}
	else
	{
		cl_buf[cl_pointer] = '\0';
		cl_flag = 1;
	}
}
