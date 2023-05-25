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


#define UART_RcvCpl_Event 		0b0001
#define UART_RcvToIdleCpl_Event 0b0010
#define UART_TxCpl_Event		0b0100

typedef struct UARTHandler_Struct
{
	UART_HandleTypeDef* huart;
	osMutexId_t RXMutex, TXMutex;
	osEventFlagsId_t EventFlags;

	uint8_t* RxBuffer;
	uint16_t RxLen;
	uint16_t RxBufferSize;
} UARTHandler_Struct;

void UART_Start(UARTHandler_Struct* UART);

int UART_Transmit(UARTHandler_Struct* UART, uint8_t* Buffer, uint16_t size);
int UART_printf(UARTHandler_Struct* UART, char* fmt,...);

int UART_Receive_ToIdle(UARTHandler_Struct* UART, uint8_t* RcvBuffer, uint16_t* RcvLen, uint16_t MaxRcv, uint32_t timeout);
int UART_Receive(UARTHandler_Struct* UART,  uint8_t* RcvBuffer, uint8_t RcvLen, uint32_t timeout);

void UART_RxCplt_CB(UARTHandler_Struct* UART);
void UART_TxCplt_CB(UARTHandler_Struct* UART);

void UART_RcvToIdle_CB(UARTHandler_Struct* UART, uint16_t RcvLen);

#endif /* UARTHANDLER_UART_HANDLER_H_ */
