/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "uart.h"
#include <string.h>
#include <stdio.h>
#include "SX1278.h"

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

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//SX1278_hw_t hlora_hw = {{RST_Pin, RST_GPIO_Port}, {D0_Pin, D0_GPIO_Port}, {NSS_Pin, NSS_GPIO_Port}, &hspi1};
SX1278_hw_t hlora_hw;
SX1278_t hlora;
uint8_t LoRa_Rx_Buffer[255];
uint8_t num_rx_byte = 0;

uint8_t uart_rx_buf = 0;

BUTTON_HandleTypedef btn;

/*
 * CRC 8	10 02 00 11 22 33 D4 10 03
 * CRC 16	10 02 00 11 22 33 B0 08 10 03
 * CRC 32	10 02 00 11 22 33 24 C2 31 6D 10 03
 */
//uint8_t tx_frame_data[] = {0x00, 0x11, 0x22, 0x33};
uint8_t tx_frame_data[255];
uint8_t tx_frame_len = 0;
void BUTTON_Press_Short_Callback(BUTTON_HandleTypedef *ButtonX)
{
	if(ButtonX == &btn)
	{
		uint8_t str[] = "Sended\n";
		HAL_UART_Transmit(&huart6, str, strlen((char *)str), 1000);
		FRAME_SYNC_Transmit(2, 1, 5, tx_frame_data, tx_frame_len, 0);
	}
}

void FRAME_SYNC_RxCpltCallback(uint8_t *p_rx_data, uint8_t data_size)
{
	char tx_str[100] = "\nData: ";
	for(int i = 0; i < data_size; i++)
	{
		char temp_str[4];
		sprintf(temp_str, "%02X ", p_rx_data[i]);
		strcat(tx_str, temp_str);
	}
	strcat(tx_str, "\nCRC Correct\n");
	HAL_UART_Transmit(&huart6, (uint8_t *)tx_str, strlen(tx_str), 1000);

}

void FRAME_SYNC_RxFailCallback(uint8_t *p_rx_data, uint8_t data_size)
{
	char tx_str[100] = "\nCRC Fail\n";
	HAL_UART_Transmit(&huart6, (uint8_t *)tx_str, strlen(tx_str), 1000);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == huart6.Instance)
	{
		COMMAND_LINE_Receive(uart_rx_buf);
		HAL_UART_Receive_IT(&huart6, &uart_rx_buf, 1);
	}
	else if(huart->Instance == huart2.Instance)
	{
//		UART_Receive(uart_rx_buf);
		HAL_UART_Receive_IT(&huart2, &uart_rx_buf, 1);
	}
}

void FRAME_SYNC_Packet_Transmit(uint8_t* tx_buffer, uint8_t tx_buffer_length)
{
	SX1278_transmit(&hlora, tx_buffer, tx_buffer_length, 2000);
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
			HAL_UART_Transmit(&huart6, LoRa_Rx_Buffer + i, 1, 1000);
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
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_SPI1_Init();
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

  BUTTON_Init(&btn, GPIOA, GPIO_PIN_0, 0);
  BUTTON_Set_Callback_Function(NULL, NULL, BUTTON_Press_Short_Callback, NULL);

  HAL_UART_Receive_IT(&huart6, &uart_rx_buf, 1);
  HAL_UART_Receive_IT(&huart2, &uart_rx_buf, 1);
  UART_Init();

  SX1278_LoRaEntryRx(&hlora, 0, 2000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  BUTTON_Handle(&btn);

//	  UART_Handle();

	  COMMAND_LINE_Handle();

	  FRAME_SYNC_Handle();

//	  uint8_t str[] = "Minh";
//	  SX1278_transmit(&hlora, str, sizeof((char *)str), 1000);

//	  uint8_t num = SX1278_LoRaRxPacket(&hlora);
//	  if(num)
//	  {
//		  SX1278_read(&hlora, LoRa_Rx_Buffer, num);
//		  HAL_UART_Transmit(&huart6, LoRa_Rx_Buffer, num, 1000);
//	  }

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RST_Pin|NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 D0_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : RST_Pin NSS_Pin */
  GPIO_InitStruct.Pin = RST_Pin|NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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
