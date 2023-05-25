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
#define I2C_OS_TX_CPLT_FLAG		0b0001
#define I2C_OS_RX_CPLT_FLAG		0b0010

typedef struct I2C_OS_HandlerStruct{
	I2C_HandleTypeDef* hi2c;
	osSemaphoreId_t Semaphore;
	osEventFlagsId_t EventFlag;
}I2C_OS_HandlerStruct;

int I2C_OS_Init(I2C_OS_HandlerStruct* i2c);

int I2C_Master_Transmit(I2C_OS_HandlerStruct* i2c);
int I2C_Master_Receive(I2C_OS_HandlerStruct* i2c);

int I2C_Slave_Transmit(I2C_OS_HandlerStruct* i2c);
int I2C_Slave_Receive(I2C_OS_HandlerStruct* i2c);

int I2C_OS_MEM_Write(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,uint16_t MemAddress,
		uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout);
int I2C_OS_MEM_Read(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
		uint16_t MemAddress, uint16_t MemAddSize, uint8_t * pData, uint16_t Size, uint32_t timeout);

int I2C_OS_IsDeviceReady(I2C_OS_HandlerStruct* i2c, uint16_t DevAddress,
									uint32_t Trials, uint32_t Timeout);

#endif /* I2CHANDLER_I2C_HANDLER_H_ */
