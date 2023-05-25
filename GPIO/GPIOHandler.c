/*
 * GPIOHandler.c
 *
 *  Created on: Mar 30, 2023
 *      Author: vanti
 */


#include "GPIOHandler.h"




void GPIO_TogglePin(GPIO_HandlerStruct GPIO)
{
	return HAL_GPIO_TogglePin(GPIO.Port, GPIO.Pin);
}

void GPIO_WritePin(GPIO_HandlerStruct GPIO, uint32_t PinState)
{
	return HAL_GPIO_WritePin(GPIO.Port, GPIO.Pin, PinState);
}

