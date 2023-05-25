/*
 * GPIOHandler.h
 *
 *  Created on: Mar 30, 2023
 *      Author: vanti
 */

#ifndef GPIO_GPIOHANDLER_H_
#define GPIO_GPIOHANDLER_H_
#include "main.h"
typedef struct GPIO_HandlerStruct
{
	GPIO_TypeDef* Port;
	uint32_t	Pin;
} GPIO_HandlerStruct;

extern GPIO_HandlerStruct PA10;
extern GPIO_HandlerStruct PB5;
extern GPIO_HandlerStruct PC6;

void GPIO_TogglePin(GPIO_HandlerStruct GPIO);
void GPIO_WritePin(GPIO_HandlerStruct GPIO, uint32_t PinState);
#endif /* GPIO_GPIOHANDLER_H_ */
