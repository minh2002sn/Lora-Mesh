/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "Command_Line.h"
#include "Frame_Sync.h"
#include "button.h"
#include <string.h>
#include <stdio.h>
#include "SX1278.h"
#include "ring_buffer.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

PACKET_STRUCTURE command_buffer[10];
RING_BUFFER_HandleTypeDef ring_buffer;

//SX1278_hw_t hlora_hw = {{RST_Pin, RST_GPIO_Port}, {D0_Pin, D0_GPIO_Port}, {NSS_Pin, NSS_GPIO_Port}, &hspi1};
SX1278_hw_t hlora_hw;
SX1278_t hlora;
uint8_t LoRa_Rx_Buffer[255];
uint8_t num_rx_byte = 0;

uint8_t uart_rx_buf = 0;

BUTTON_HandleTypedef btn;
uint8_t tx_frame_data[255];
uint8_t tx_frame_len = 0;

void Push_Command_To_Ring_Buffer(uint8_t final_des_id, uint8_t temp_des_id, uint8_t time_to_live, uint8_t is_requiring_reply, uint8_t length, uint8_t buffer[])
{
	PACKET_STRUCTURE temp_data = {final_des_id, temp_des_id, FS_Data.my_id, time_to_live, length};
	for(int i = 0; i < length; i++)
	{
		temp_data.buffer[i] = buffer[i];
	}
	temp_data.is_requiring_reply = is_requiring_reply;
	RING_BUFFER_Push(&ring_buffer, temp_data);
}

void Command_Sending_Handle()
{
	if(RING_BUFFER_Available(&ring_buffer) && FRAME_SYNC_Is_Ready_Transmit())
	{
		PACKET_STRUCTURE temp_data;
		RING_BUFFER_Pop(&ring_buffer, &temp_data);
		FRAME_SYNC_Send_Frame(temp_data.final_des_id, temp_data.temp_des_id, temp_data.time_to_live, temp_data.buffer, temp_data.length, temp_data.is_requiring_reply);
	}
}

void FRAME_SYNC_RxCpltCallback(PACKET_STRUCTURE rx_packet)
{
#if MY_ID != 0
	static uint32_t counter = 0;
	if(rx_packet.buffer[0] == 0x02)
	{
		counter++;
		uint8_t temp_cmd[] = {rx_packet.buffer[0], FS_Data.my_id, counter};
		Push_Command_To_Ring_Buffer(0, FS_Data.my_id - 1, 7, 0, sizeof(temp_cmd), temp_cmd);
	}
#endif

	char tx_str[100] = {};
	sprintf(tx_str, "\nSource ID: %02X\nData: ", rx_packet.src_id);
	for(int i = 0; i < rx_packet.length; i++)
	{
		char temp_str[4];
		sprintf(temp_str, "%02X ", rx_packet.buffer[i]);
		strcat(tx_str, temp_str);
	}
	strcat(tx_str, "\nCRC Correct\n");
	HAL_UART_Transmit(&huart1, (uint8_t *)tx_str, strlen(tx_str), 1000);

}

void FRAME_SYNC_RxFailCallback(uint8_t *p_rx_data, uint8_t data_size)
{
	char tx_str[100] = "\nCRC Fail\n";
	HAL_UART_Transmit(&huart1, (uint8_t *)tx_str, strlen(tx_str), 1000);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == huart1.Instance)
	{
		COMMAND_LINE_Receive(uart_rx_buf);
		HAL_UART_Receive_IT(&huart1, &uart_rx_buf, 1);
	}
}

void FRAME_SYNC_Packet_Transmit(uint8_t* tx_buffer, uint8_t tx_buffer_length)
{
	SX1278_transmit(&hlora, tx_buffer, tx_buffer_length, 2000);
	SX1278_LoRaEntryRx(&hlora, 0, 2000);
}

void FRAME_SYNC_Packet_Receive()
{
  	if(hlora.status != RX)
  	{
  		SX1278_LoRaEntryRx(&hlora, 0, 2000);
  	}

	num_rx_byte = SX1278_LoRaRxPacket(&hlora);
	if(num_rx_byte > 0)
	{
		SX1278_read(&hlora, LoRa_Rx_Buffer, num_rx_byte);
		for(int i = 0; i < num_rx_byte; i++)
		{
			FRAME_SYNC_Receive(LoRa_Rx_Buffer[i]);
		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  hlora_hw.dio0.port = D0_GPIO_Port;
  hlora_hw.dio0.pin = D0_Pin;
  hlora_hw.nss.port = NSS_GPIO_Port;
  hlora_hw.nss.pin = NSS_Pin;
  hlora_hw.reset.port = RST_GPIO_Port;
  hlora_hw.reset.pin = RST_Pin;
  hlora_hw.spi = &hspi1;

  hlora.hw = &hlora_hw;
  SX1278_init(&hlora, 434000000, SX1278_POWER_20DBM, SX1278_LORA_SF_12,
		  SX1278_LORA_BW_500KHZ, SX1278_LORA_CR_4_8, SX1278_LORA_CRC_EN, 10);

  HAL_UART_Receive_IT(&huart1, &uart_rx_buf, 1);

  SX1278_LoRaEntryRx(&hlora, 0, 2000);

  RING_BUFFER_Init(&ring_buffer, command_buffer, 10);

//#if MY_ID == 0
////	  static uint32_t timer = 0;
////	  if(HAL_GetTick() - timer > 5000)
////	  {
//		  uint8_t temp_cmd[] = {0x02};
//		  Push_Command_To_Ring_Buffer(1, 1, 7, 1, sizeof(temp_cmd), temp_cmd);
////		  Push_Command_To_Ring_Buffer(2, 1, 7, 1, sizeof(temp_cmd), temp_cmd);
//		  Push_Command_To_Ring_Buffer(4, 1, 7, 0, sizeof(temp_cmd), temp_cmd);
////		  timer = HAL_GetTick();
////	  }
//#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#if MY_ID == 0
	  static uint32_t timer = 0;
	  if(HAL_GetTick() - timer > 30000)
	  {
		  uint8_t temp_cmd[] = {0x02};
		  Push_Command_To_Ring_Buffer(1, 1, 7, 1, sizeof(temp_cmd), temp_cmd);
		  Push_Command_To_Ring_Buffer(2, 1, 7, 1, sizeof(temp_cmd), temp_cmd);
		  Push_Command_To_Ring_Buffer(3, 1, 7, 1, sizeof(temp_cmd), temp_cmd);
		  Push_Command_To_Ring_Buffer(4, 1, 7, 0, sizeof(temp_cmd), temp_cmd);
		  timer = HAL_GetTick();
	  }
#endif

//	  BUTTON_Handle(&btn);

//	  COMMAND_LINE_Handle();

	  FRAME_SYNC_Handle();

	  Command_Sending_Handle();


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RST_Pin|NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : D0_Pin */
  GPIO_InitStruct.Pin = D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(D0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RST_Pin NSS_Pin */
  GPIO_InitStruct.Pin = RST_Pin|NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
