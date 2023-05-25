/*
 * UART_Handler.c
 *
 *  Created on: Mar 31, 2023
 *      Author: vanti
 */
#include "UART_Handler.h"
#include "stdlib.h"
#include <stdarg.h>
#include "string.h"

#define UART_TxBufferSize 200
uint32_t timeStart;
void UART_Start(UARTHandler_Struct* UART)
{
	UART->RXMutex = osMutexNew(NULL);
	UART->TXMutex = osMutexNew(NULL);
	UART->EventFlags =  osEventFlagsNew(NULL);
	UART->RxBuffer = malloc(UART->RxBufferSize);
	memset(UART->RxBuffer, 0, UART->RxBufferSize);

}


int UART_printf(UARTHandler_Struct* UART, char* format, ...)
{

	va_list arg;
	char Buffer[UART_TxBufferSize]={0};
	size_t size=sizeof(Buffer);
	va_start(arg,format);
	char* Output  = vasnprintf(Buffer,&size,format,arg);
	va_end(arg);
	if(Output != NULL)
	{
//		while(HAL_UART_GetState(UART->huart) == HAL_UART_STATE_BUSY_RX);

		UART_Transmit(UART,(uint8_t*) Output, size);
		if(Output != Buffer)
		{
			vPortFree(Output);
		}
	}

	return size;
}

int UART_Transmit(UARTHandler_Struct* UART, uint8_t* Buffer, uint16_t size)
{
	osMutexAcquire(UART->TXMutex, 20);
	osEventFlagsClear(UART->EventFlags, UART_TxCpl_Event);

	HAL_UART_Transmit_DMA(UART->huart, Buffer, size);
	__HAL_DMA_DISABLE_IT(UART->huart->hdmatx, DMA_IT_HT);

	osEventFlagsWait(UART->EventFlags, UART_TxCpl_Event, osFlagsWaitAll, 50);
	osMutexRelease(UART->TXMutex);
	return 0;
}

int UART_Receive_ToIdle(UARTHandler_Struct* UART, uint8_t* RcvBuffer, uint16_t* RcvLen, uint16_t MaxRcv, uint32_t timeout)
{
	int Status = 0;
	Status = osMutexAcquire(UART->RXMutex, timeout);
	if(Status == osErrorTimeout) return -HAL_TIMEOUT;
	else if (Status < 0 ) return -HAL_ERROR;

	osEventFlagsClear(UART->EventFlags, UART_RcvToIdleCpl_Event);
	Status = HAL_UARTEx_ReceiveToIdle_DMA(UART->huart, UART->RxBuffer, MaxRcv);
	__HAL_DMA_DISABLE_IT(UART->huart->hdmarx, DMA_IT_HT);

	if (Status !=HAL_OK)
	{
		HAL_UART_AbortReceive(UART->huart);
		osMutexRelease(UART->RXMutex);
		return -Status;
	}

	Status = osEventFlagsWait(UART->EventFlags, UART_RcvToIdleCpl_Event, osFlagsWaitAll, timeout);

	if(Status >0)
	{
		*RcvLen = UART->RxLen;
		memcpy(RcvBuffer, UART -> RxBuffer, UART->RxLen);
		memset(UART->RxBuffer, 0, UART->RxLen);
		UART->RxLen = 0;
	}
	HAL_UART_AbortReceive(UART->huart);
	osEventFlagsClear(UART->EventFlags, UART_RcvToIdleCpl_Event);
	osMutexRelease(UART->RXMutex);
//	SyncPrintf("Proc Time %ld \r\n", osKernelGetTickCount()-timeStart);

	return HAL_OK;
}

int UART_Receive(UARTHandler_Struct* UART,  uint8_t* RcvBuffer, uint8_t RcvLen, uint32_t timeout)
{
	int Status;
	Status = osMutexAcquire(UART->RXMutex, timeout);
	if(Status == osErrorTimeout) return -HAL_TIMEOUT;
	else if (Status < 0 ) return -HAL_ERROR;

	osEventFlagsClear(UART->EventFlags, UART_RcvToIdleCpl_Event);
	Status = HAL_UART_Receive_DMA(UART->huart, UART->RxBuffer, RcvLen);
	__HAL_DMA_DISABLE_IT(UART->huart->hdmarx, DMA_IT_HT);

	if (Status !=HAL_OK)
	{
		osMutexRelease(UART->RXMutex);
		return -Status;
	}

	Status = osEventFlagsWait(UART->EventFlags, UART_RcvCpl_Event, osFlagsWaitAll, timeout);

	if(Status >0)
	{
		memcpy(RcvBuffer, UART -> RxBuffer, RcvLen);
		memset(UART->RxBuffer, 0, RcvLen);
		UART->RxLen = 0;
	}
	else
	{
		HAL_UART_AbortReceive(UART->huart);
		osEventFlagsClear(UART->EventFlags, UART_RcvToIdleCpl_Event);
	}
	osMutexRelease(UART->RXMutex);
	return HAL_OK;
}

void UART_RcvToIdle_CB(UARTHandler_Struct* UART, uint16_t RcvLen)
{
	UART->RxLen = RcvLen;
//	HAL_UART_Transmit_IT(UART->huart, UART->, Size)
	osEventFlagsSet(UART->EventFlags, UART_RcvToIdleCpl_Event);
}

void UART_RxCplt_CB(UARTHandler_Struct* UART)
{
	osEventFlagsSet(UART->EventFlags, UART_RcvCpl_Event);
}

void UART_TxCplt_CB(UARTHandler_Struct* UART)
{
	osEventFlagsSet(UART->EventFlags, UART_TxCpl_Event);
}
