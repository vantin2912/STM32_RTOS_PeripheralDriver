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

#define UART_OS_PrintfBufferSize 100
void UART_OS_Init(UART_OS_HandlerStruct* UART, UART_HandleTypeDef* huart)
{
	UART->RXMutex = osMutexNew(NULL);
	UART->TXMutex = osMutexNew(NULL);
	UART->EventFlags = osEventFlagsNew(NULL);
	UART->huart = huart;
}


int UART_OS_printf(UART_OS_HandlerStruct* UART, char* format, ...)
{

	va_list arg;
	char Buffer[UART_OS_PrintfBufferSize]={0};
	size_t size=UART_OS_PrintfBufferSize;
	va_start(arg,format);
	char* Output  = vasnprintf(Buffer,&size,format,arg);
	va_end(arg);
	if(Output != NULL)
	{
//		while(HAL_UART_GetState(UART->huart) == HAL_UART_STATE_BUSY_RX);

		UART_OS_Transmit(UART,(uint8_t*) Output, size);
		if(Output != Buffer)
		{
			vPortFree(Output);
		}
	}

	return size;
}

int UART_OS_Transmit(UART_OS_HandlerStruct* UART, uint8_t* Buffer, uint16_t size)
{
	int Status;
	Status = osMutexAcquire(UART->TXMutex, 20);
	if (Status != osOK) return Status;
	osEventFlagsClear(UART->EventFlags, UART_OS_TxCpl_Event);

	HAL_UART_Transmit_DMA(UART->huart, Buffer, size);
	__HAL_DMA_DISABLE_IT(UART->huart->hdmatx, DMA_IT_HT);

	Status = osEventFlagsWait(UART->EventFlags, UART_OS_TxCpl_Event, osFlagsWaitAll, 20);

	osMutexRelease(UART->TXMutex);

	return Status>0? osOK: Status;
}

int UART_OS_Receive_ToIdle(UART_OS_HandlerStruct* UART, uint8_t* RcvBuffer, uint16_t* RcvLen, uint16_t MaxRcv, uint32_t timeout)
{
	int Status = 0;
	Status = osMutexAcquire(UART->RXMutex, timeout);
	if(Status == osErrorTimeout) return osErrorTimeout;
	else if (Status < 0 ) return osError;

	osEventFlagsClear(UART->EventFlags, UART_OS_RcvToIdleCpl_Event);
	Status = HAL_UARTEx_ReceiveToIdle_DMA(UART->huart, RcvBuffer, MaxRcv);
	__HAL_DMA_DISABLE_IT(UART->huart->hdmarx, DMA_IT_HT);

	if (Status !=HAL_OK)
	{
		HAL_UART_AbortReceive(UART->huart);
		osMutexRelease(UART->RXMutex);
		return osError;
	}

	Status = osEventFlagsWait(UART->EventFlags, UART_OS_RcvToIdleCpl_Event, osFlagsWaitAll, timeout);

	if(Status >0)
	{
		*RcvLen = UART->RxLen;
		UART->RxLen = 0;
//		Status = osOK
	}
	HAL_UART_AbortReceive(UART->huart);
	osEventFlagsClear(UART->EventFlags, UART_OS_RcvToIdleCpl_Event);
	osMutexRelease(UART->RXMutex);
//	SyncPrintf("Proc Time %ld \r\n", osKernelGetTickCount()-timeStart);

	return Status > 0? osOK: Status;
}

int UART_OS_Receive(UART_OS_HandlerStruct* UART,  uint8_t* RcvBuffer, uint8_t RcvLen, uint32_t timeout)
{
	int Status;
	Status = osMutexAcquire(UART->RXMutex, timeout);
	if(Status == osErrorTimeout) return -HAL_TIMEOUT;
	else if (Status < 0 ) return -HAL_ERROR;

	osEventFlagsClear(UART->EventFlags, UART_OS_RcvToIdleCpl_Event);
	Status = HAL_UART_Receive_DMA(UART->huart, RcvBuffer, RcvLen);
	__HAL_DMA_DISABLE_IT(UART->huart->hdmarx, DMA_IT_HT);

	if (Status !=HAL_OK)
	{
		osMutexRelease(UART->RXMutex);
		return -Status;
	}

	Status = osEventFlagsWait(UART->EventFlags, UART_OS_RcvCpl_Event, osFlagsWaitAll, timeout);

	if(Status >0)
	{
		UART->RxLen = 0;
	}
	else
	{
		HAL_UART_AbortReceive(UART->huart);
		osEventFlagsClear(UART->EventFlags, UART_OS_RcvToIdleCpl_Event);
	}
	osMutexRelease(UART->RXMutex);
	return Status > 0? osOK: Status;
}

void UART_OS_RcvToIdle_CB(UART_OS_HandlerStruct* UART, uint16_t RcvLen)
{
	UART->RxLen = RcvLen;
	osEventFlagsSet(UART->EventFlags, UART_OS_RcvToIdleCpl_Event);
}

void UART_OS_RxCplt_CB(UART_OS_HandlerStruct* UART)
{
	osEventFlagsSet(UART->EventFlags, UART_OS_RcvCpl_Event);
}

void UART_OS_TxCplt_CB(UART_OS_HandlerStruct* UART)
{
//	uint32_t flag = osEventFlagsGet(UART->EventFlags);
	osEventFlagsSet(UART->EventFlags, UART_OS_TxCpl_Event);
}
