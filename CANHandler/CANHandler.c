
#include "CANHandler.h"

int CAN_OS_Init(CAN_OS_HandlerStruct* CANHandler, CAN_HandleTypeDef* hcan)
{
	CANHandler->hcan = hcan;
	CANHandler->EventFlag = osEventFlagsNew(NULL);
	CANHandler->TxSemaphore = osSemaphoreNew(CAN_OS_TxMailBox_Max, CAN_OS_TxMailBox_Max, NULL);
	CANHandler->RxSemaphore = osSemaphoreNew(1, 1, NULL);

	CAN_OS_ActivateNotification(CANHandler, CAN_IT_TX_MAILBOX_EMPTY);
	CAN_OS_ActivateNotification(CANHandler, CAN_IT_RX_FIFO0_MSG_PENDING);
	CAN_OS_ActivateNotification(CANHandler, CAN_IT_RX_FIFO1_MSG_PENDING);

	return HAL_OK;
}

int CAN_OS_ConfigFilter(CAN_OS_HandlerStruct* CANHandler,const CAN_FilterTypeDef* Filter)
{
	return HAL_CAN_ConfigFilter(CANHandler->hcan, Filter);
}

int CAN_OS_ActivateNotification(CAN_OS_HandlerStruct* CANHandler, uint32_t ActiveITs)
{
	 return HAL_CAN_ActivateNotification(CANHandler->hcan,ActiveITs);
}

int CAN_OS_Start(CAN_OS_HandlerStruct* CANHandler)
{
	return HAL_CAN_Start(CANHandler->hcan);
}

int CAN_OS_Transmit(CAN_OS_HandlerStruct* CANHandler, const CAN_TxHeaderTypeDef txHeader, uint8_t* txData, uint32_t* txMailbox, uint32_t timeout)
{
	int Status;
	Status = osSemaphoreAcquire(CANHandler->TxSemaphore, timeout);
	if(Status == osErrorTimeout) return HAL_TIMEOUT;
	Status = HAL_CAN_AddTxMessage(CANHandler->hcan, &txHeader, txData, txMailbox);
	if (Status != HAL_OK){
		osSemaphoreRelease(CANHandler->TxSemaphore);
	}
	return Status;
}

int CAN_OS_Receive(CAN_OS_HandlerStruct* CANHandler, uint32_t rxFifo, CAN_RxHeaderTypeDef* rxHeader, uint8_t* rxData, uint32_t timeout)
{
	int Status;
	Status = osSemaphoreAcquire(CANHandler->RxSemaphore, timeout);
	if (Status == osErrorTimeout) return HAL_TIMEOUT;
	Status = osEventFlagsWait(CANHandler->EventFlag, CAN_OS_RxCplt_Event, osFlagsWaitAll, timeout);
	if(Status < 0 )
	{
		return -Status;
	}
	Status = HAL_CAN_GetRxMessage(CANHandler->hcan, rxFifo, rxHeader, rxData);
	osSemaphoreRelease(CANHandler->RxSemaphore);
	return Status;
}

void CAN_OS_RxCplt_CB(CAN_OS_HandlerStruct* CANHandler)
{
	osEventFlagsSet(CANHandler->EventFlag, CAN_OS_RxCplt_Event);
}

void CAN_OS_TxCplt_CB(CAN_OS_HandlerStruct* CANHandler)
{
	osSemaphoreRelease(CANHandler->TxSemaphore);
}
void CAN_OS_RegisterCB(CAN_HandleTypeDef *hcan, void (* pTxCallback)(CAN_HandleTypeDef *_hcan),
												void (* pRxCallback)(CAN_HandleTypeDef *_hcan))
{
	HAL_CAN_RegisterCallback(hcan, HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID,pTxCallback);
	HAL_CAN_RegisterCallback(hcan, HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID,pTxCallback);
	HAL_CAN_RegisterCallback(hcan, HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID,pTxCallback);
	HAL_CAN_RegisterCallback(hcan, HAL_CAN_TX_MAILBOX0_ABORT_CB_ID,pTxCallback);
	HAL_CAN_RegisterCallback(hcan, HAL_CAN_TX_MAILBOX1_ABORT_CB_ID,pTxCallback);
	HAL_CAN_RegisterCallback(hcan, HAL_CAN_TX_MAILBOX2_ABORT_CB_ID,pTxCallback);

	HAL_CAN_RegisterCallback(hcan,HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID,pRxCallback);
	HAL_CAN_RegisterCallback(hcan,HAL_CAN_RX_FIFO1_MSG_PENDING_CB_ID,pRxCallback);
}
