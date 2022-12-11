#ifndef BUTTON_H
#define BUTTON_H

#include "main.h"

typedef struct{
	uint8_t BTN_Current;
	uint8_t BTN_Last;
	uint8_t BTN_Filter;
	uint8_t is_debouncing;
	uint8_t is_press;
	uint32_t time_debounce;
	uint32_t time_start_press;
	uint8_t releasing_state;
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
} BUTTON_HandleTypedef;

typedef void (*button_callback_function_t)(BUTTON_HandleTypedef *);

void BUTTON_Handle(BUTTON_HandleTypedef *ButtonX);
void BUTTON_Init(BUTTON_HandleTypedef *ButtonX, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t releasing_state);
void BUTTON_Set_Callback_Function(button_callback_function_t p_pressing_callback_function,
								button_callback_function_t p_releasing_callback_function,
								button_callback_function_t p_short_pressing_callback_function,
								button_callback_function_t p_long_pressing_callback_function);

#endif
