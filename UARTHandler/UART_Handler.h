/*
 * UART_Handler.h
 *
 *  Created on: Mar 31, 2023
 *      Author: vanti
 */

#ifndef UARTHANDLER_UART_HANDLER_H_
#define UARTHANDLER_UART_HANDLER_H_

#include "main.h"
#include "cmsis_os.h"


#define UART_OS_RcvCpl_Event 		0b0001
#define UART_OS_RcvToIdleCpl_Event 0b0010
#define UART_OS_TxCpl_Event		0b0100

typedef struct UART_OS_HandlerStruct
{
	UART_HandleTypeDef* huart;
	osMutexId_t RXMutex, TXMutex;
	osEventFlagsId_t EventFlags;

	volatile uint16_t RxLen;
} UART_OS_HandlerStruct;

void UART_OS_Init(UART_OS_HandlerStruct* UART, UART_HandleTypeDef* huart);

int UART_OS_Transmit(UART_OS_HandlerStruct* UART, uint8_t* Buffer, uint16_t size);
int UART_OS_printf(UART_OS_HandlerStruct* UART, char* fmt,...);

int UART_OS_Receive_ToIdle(UART_OS_HandlerStruct* UART, uint8_t* RcvBuffer, uint16_t* RcvLen, uint16_t MaxRcv, uint32_t timeout);
int UART_OS_Receive(UART_OS_HandlerStruct* UART,  uint8_t* RcvBuffer, uint8_t RcvLen, uint32_t timeout);

void UART_OS_RxCplt_CB(UART_OS_HandlerStruct* UART);
void UART_OS_TxCplt_CB(UART_OS_HandlerStruct* UART);

void UART_OS_RcvToIdle_CB(UART_OS_HandlerStruct* UART, uint16_t RcvLen);

#endif /* UARTHANDLER_UART_HANDLER_H_ */
