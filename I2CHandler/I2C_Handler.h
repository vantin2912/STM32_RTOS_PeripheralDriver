/*
 * I2C_Handler.h
 *
 *  Created on: May 25, 2023
 *      Author: vanti
 */

#ifndef I2CHANDLER_I2C_HANDLER_H_
#define I2CHANDLER_I2C_HANDLER_H_
#include "main.h"
#include "cmsis_os.h"
#define I2C_OS_MEM_TX_CPLT_FLAG			0x01
#define I2C_OS_MEM_RX_CPLT_FLAG			0x02
#define I2C_OS_MASTER_TX_CPLT_FLAG		0x04
#define I2C_OS_MASTER_RX_CPLT_FLAG		0x08

typedef struct I2C_OS_HandlerStruct{
	I2C_HandleTypeDef* hi2c;
	osSemaphoreId_t Semaphore;
	osEventFlagsId_t EventFlag;
}I2C_OS_HandlerStruct;

int I2C_OS_Init(I2C_OS_HandlerStruct* i2c, I2C_HandleTypeDef* hi2c);

// Not implemented Yet
int I2C_OS_Master_Transmit_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout);
int I2C_OS_Master_Transmit_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout);
int I2C_OS_Master_Receive_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout);
int I2C_OS_Master_Receive_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t timeout);
//
//int I2C_OS_Slave_Transmit(I2C_OS_HandlerStruct* i2c);
//int I2C_OS_Slave_Receive(I2C_OS_HandlerStruct* i2c);

int I2C_OS_MEM_Write_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,uint16_t MemAddress,
		uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout);
int I2C_OS_MEM_Read_DMA(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
		uint16_t MemAddress, uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout);

int I2C_OS_MEM_Write_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,uint16_t MemAddress,
		uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout);
int I2C_OS_MEM_Read_IT(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
		uint16_t MemAddress, uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout);

int I2C_OS_IsDeviceReady(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
									uint32_t Trials, uint32_t Timeout);

void I2C_OS_MEM_RxCpltCB(I2C_OS_HandlerStruct* i2c);
void I2C_OS_MEM_TxCpltCB(I2C_OS_HandlerStruct* i2c);
void I2C_OS_MasterRxCpltCB(I2C_OS_HandlerStruct* i2c);
void I2C_OS_MasterTxCpltCB(I2C_OS_HandlerStruct* i2c);

#endif /* I2CHANDLER_I2C_HANDLER_H_ */
