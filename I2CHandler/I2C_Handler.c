/*
 * I2C_Handler.c
 *
 *  Created on: May 25, 2023
 *      Author: vanti
 */


#include "I2C_Handler.h"
int I2C_OS_Init(I2C_OS_HandlerStruct* i2c, I2C_HandleTypeDef* hi2c)
{
	i2c->hi2c = hi2c;
//	osSemaphoreNew(1, 0, NULL);
	i2c->Semaphore = osSemaphoreNew(1, 1, NULL);
	i2c->EventFlag = osEventFlagsNew(NULL);
	return 0;
}
int I2C_OS_MEM_Write_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,uint16_t MemAddress,
		uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout);
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MEM_TX_CPLT_FLAG);
	Status = HAL_I2C_Mem_Write_DMA(i2c->hi2c, DevAddress, MemAddress, MemAddSize, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MEM_TX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status;
}
int I2C_OS_MEM_Read_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
		uint16_t MemAddress, uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout );
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MEM_RX_CPLT_FLAG);
	Status = HAL_I2C_Mem_Read_DMA(i2c->hi2c, DevAddress, MemAddress, MemAddSize, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MEM_RX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status;
}

int I2C_OS_MEM_Write_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,uint16_t MemAddress,
		uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout);
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MEM_TX_CPLT_FLAG);
	Status = HAL_I2C_Mem_Write_IT(i2c->hi2c, DevAddress, MemAddress, MemAddSize, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MEM_TX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status > 0 ? osOK: Status ;
}

int I2C_OS_MEM_Read_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
		uint16_t MemAddress, uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout );
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MEM_RX_CPLT_FLAG);
	Status = HAL_I2C_Mem_Read_IT(i2c->hi2c, DevAddress, MemAddress, MemAddSize, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MEM_RX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status > 0 ? osOK: Status;
}

int I2C_OS_IsDeviceReady(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint32_t
		Trials, uint32_t Timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, Timeout);
	if (Status != osOK) return Status;
	Status = HAL_I2C_IsDeviceReady(i2c->hi2c, DevAddress, Trials, Timeout);

	if (Status != 0)
	{
		Status = osError;
	} else
	{
		Status = osOK;
	}
	osSemaphoreRelease(i2c->Semaphore);
	return Status;

}

int I2C_OS_Master_Transmit_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout );
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MASTER_TX_CPLT_FLAG);
	Status = HAL_I2C_Master_Transmit_IT(i2c->hi2c, DevAddress, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MASTER_TX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status > 0 ? osOK: Status;

}
int I2C_OS_Master_Transmit_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout );
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MASTER_TX_CPLT_FLAG);
	Status = HAL_I2C_Master_Transmit_DMA(i2c->hi2c, DevAddress, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MASTER_TX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status > 0 ? osOK: Status;

}

int I2C_OS_Master_Receive_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout );
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MASTER_RX_CPLT_FLAG);
	Status = HAL_I2C_Master_Receive_IT(i2c->hi2c, DevAddress, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MASTER_RX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status > 0 ? osOK: Status;

}

int I2C_OS_Master_Receive_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout)
{
	int Status = osOK;
	Status = osSemaphoreAcquire(i2c->Semaphore, timeout );
	if (Status != osOK) return Status;
	osEventFlagsClear(i2c->EventFlag, I2C_OS_MASTER_RX_CPLT_FLAG);
	Status = HAL_I2C_Master_Receive_DMA(i2c->hi2c, DevAddress, pData, Size);
	if (Status != 0)
	{
		Status = osError;
		osSemaphoreRelease(i2c->Semaphore);
		return Status;
	}
	Status = osEventFlagsWait(i2c->EventFlag, I2C_OS_MASTER_RX_CPLT_FLAG, osFlagsWaitAll, timeout);
	osSemaphoreRelease(i2c->Semaphore);
	return Status > 0 ? osOK: Status;
}

void I2C_OS_MEM_RxCpltCB(I2C_OS_HandlerStruct* i2c)
{
	osEventFlagsSet(i2c->EventFlag, I2C_OS_MEM_RX_CPLT_FLAG);
}
void I2C_OS_MEM_TxCpltCB(I2C_OS_HandlerStruct* i2c)
{
	osEventFlagsSet(i2c->EventFlag, I2C_OS_MEM_TX_CPLT_FLAG);
}

void I2C_OS_MasterRxCpltCB(I2C_OS_HandlerStruct* i2c)
{
	osEventFlagsSet(i2c->EventFlag, I2C_OS_MASTER_RX_CPLT_FLAG);
}
void I2C_OS_MasterTxCpltCB(I2C_OS_HandlerStruct* i2c)
{
	osEventFlagsSet(i2c->EventFlag, I2C_OS_MASTER_TX_CPLT_FLAG);
}

