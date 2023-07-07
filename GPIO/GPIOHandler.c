/*
 * GPIOHandler.c
 *
 *  Created on: Mar 30, 2023
 *      Author: vanti
 */


#include "GPIOHandler.h"

GPIO_HandlerStruct PA0 = {.Port = GPIOA, .Pin = GPIO_PIN_0};
GPIO_HandlerStruct PA2 = {.Port = GPIOA, .Pin = GPIO_PIN_2};
GPIO_HandlerStruct PA3 = {.Port = GPIOA, .Pin = GPIO_PIN_3};
GPIO_HandlerStruct PA7 = {.Port = GPIOA, .Pin = GPIO_PIN_7};
GPIO_HandlerStruct PA10 = {.Port = GPIOA, .Pin = GPIO_PIN_10};

GPIO_HandlerStruct PB0 = {.Port = GPIOB, .Pin = GPIO_PIN_0};
GPIO_HandlerStruct PB1 = {.Port = GPIOB, .Pin = GPIO_PIN_1};
GPIO_HandlerStruct PB10 = {.Port = GPIOB, .Pin = GPIO_PIN_10};
GPIO_HandlerStruct PB11 = {.Port = GPIOB, .Pin = GPIO_PIN_11};

GPIO_HandlerStruct PB5 = {.Port = GPIOB, .Pin = GPIO_PIN_5};
GPIO_HandlerStruct PC6 = {.Port = GPIOC, .Pin = GPIO_PIN_6};
GPIO_HandlerStruct PC13 = {.Port = GPIOC, .Pin = GPIO_PIN_13};


void GPIO_TogglePin(GPIO_HandlerStruct GPIO)
{
	return HAL_GPIO_TogglePin(GPIO.Port, GPIO.Pin);
}

void GPIO_WritePin(GPIO_HandlerStruct GPIO, uint32_t PinState)
{
	return HAL_GPIO_WritePin(GPIO.Port, GPIO.Pin, PinState);
}

int GPIO_ReadPin(GPIO_HandlerStruct GPIO)
{
	return HAL_GPIO_ReadPin(GPIO.Port, GPIO.Pin);
}

