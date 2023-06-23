/*
 * CAN.h
 *
 *  Created on: Jun 19, 2023
 *      Author: vanti
 */

#ifndef CANHANDLER_CANHANDLER_H_
#define CANHANDLER_CANHANDLER_H_

#include "main.h"
#include "cmsis_os.h"
#define CAN_OS_TxMailBox_Max		3

#define CAN_OS_ActivateTxCB_ID		1
#define CAN_OS_ActivateRxFifo0_ID	2
#define CAN_OS_ActivateRxFifo1_ID	3

#define CAN_OS_RxFifo0Cplt_Event			0x01
#define CAN_OS_RxFifo1Cplt_Event			0x02

typedef struct CAN_OS_HandlerStruct {
	CAN_HandleTypeDef *hcan;
	osSemaphoreId_t TxSemaphore;
	osSemaphoreId_t RxSemaphore;
	osEventFlagsId_t EventFlag;

}CAN_OS_HandlerStruct;

int CAN_OS_Init(CAN_OS_HandlerStruct* CANHandler, CAN_HandleTypeDef* hcan);
int CAN_OS_ActivateNotification(CAN_OS_HandlerStruct* CANHandler, uint32_t ActiveITs);

int CAN_OS_ConfigFilter(CAN_OS_HandlerStruct* CANHandler,const CAN_FilterTypeDef* Filter);
int CAN_OS_Start(CAN_OS_HandlerStruct* CANHandler);

int CAN_OS_Transmit(CAN_OS_HandlerStruct* CANHandler, const CAN_TxHeaderTypeDef *txHeader, uint8_t* txData, uint32_t* txMailbox, uint32_t timeout);
int CAN_OS_ListenMsg(CAN_OS_HandlerStruct* CANHandler, uint32_t rxFifo, CAN_RxHeaderTypeDef* rxHeader, uint8_t* rxData, uint32_t timeout);

void CAN_OS_TxCplt_CB(CAN_OS_HandlerStruct* CANHandler);
void CAN_OS_RxFifo0Cplt_CB(CAN_OS_HandlerStruct* CANHandler);
void CAN_OS_RxFifo1Cplt_CB(CAN_OS_HandlerStruct* CANHandler);

void CAN_OS_RegisterCB(CAN_OS_HandlerStruct *hcan, uint8_t callbackID, void (* pCallback)(CAN_HandleTypeDef *_hcan));

#endif /* CANHANDLER_CANHANDLER_H_ */
