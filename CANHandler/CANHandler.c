
#include "CANHandler.h"

int CAN_OS_Init(CAN_OS_HandlerStruct* CANHandler, CAN_HandleTypeDef* hcan)
{
	HAL_CAN_Stop(hcan);
	CANHandler->hcan = hcan;
	CANHandler->EventFlag = osEventFlagsNew(NULL);
	CANHandler->TxSemaphore = osSemaphoreNew(CAN_OS_TxMailBox_Max, CAN_OS_TxMailBox_Max, NULL);
	CANHandler->RxSemaphore = osSemaphoreNew(1, 1, NULL);
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
	CAN_OS_ActivateNotification(CANHandler, CAN_IT_TX_MAILBOX_EMPTY);

	return HAL_CAN_Start(CANHandler->hcan);
}

int CAN_OS_Transmit(CAN_OS_HandlerStruct* CANHandler, const CAN_TxHeaderTypeDef *txHeader, uint8_t* txData, uint32_t* txMailbox, uint32_t timeout)
{
	int Status;
	Status = osSemaphoreAcquire(CANHandler->TxSemaphore, timeout);
	if(Status == osErrorTimeout) return HAL_TIMEOUT;
	Status = HAL_CAN_AddTxMessage(CANHandler->hcan, txHeader, txData, txMailbox);
	if (Status != HAL_OK){
		osSemaphoreRelease(CANHandler->TxSemaphore);
	}
	return Status;
}

int CAN_OS_ListenMsg(CAN_OS_HandlerStruct* CANHandler, uint32_t rxFifo, CAN_RxHeaderTypeDef* rxHeader, uint8_t* rxData, uint32_t timeout)
{
	int Status;
	uint32_t activateITS;
	uint8_t waitEvent;
	if (rxFifo == CAN_FILTER_FIFO0){
		activateITS = CAN_IT_RX_FIFO0_MSG_PENDING;
		waitEvent = CAN_OS_RxFifo0Cplt_Event;
	}
	else if (rxFifo == CAN_FILTER_FIFO1 ) {
		activateITS = CAN_IT_RX_FIFO1_MSG_PENDING;
		waitEvent = CAN_OS_RxFifo0Cplt_Event;

	}
	else return osErrorParameter;

	Status = HAL_CAN_ActivateNotification(CANHandler->hcan, activateITS);

	Status = osSemaphoreAcquire(CANHandler->RxSemaphore, timeout);
	if (Status == osErrorTimeout) return HAL_TIMEOUT;
	Status = osEventFlagsWait(CANHandler->EventFlag, waitEvent, osFlagsWaitAll, timeout);
	if(Status < 0 )
	{
		osSemaphoreRelease(CANHandler->RxSemaphore);
		return Status;
	}
	Status = HAL_CAN_GetRxMessage(CANHandler->hcan, rxFifo, rxHeader, rxData);
	osSemaphoreRelease(CANHandler->RxSemaphore);
	return Status == HAL_OK? osOK: osError;
}

void CAN_OS_RxFifo0Cplt_CB(CAN_OS_HandlerStruct* CANHandler)
{
	__HAL_CAN_DISABLE_IT(CANHandler->hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
	osEventFlagsSet(CANHandler->EventFlag, CAN_OS_RxFifo0Cplt_Event);
}
void CAN_OS_RxFifo1Cplt_CB(CAN_OS_HandlerStruct* CANHandler)
{
	__HAL_CAN_DISABLE_IT(CANHandler->hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
	osEventFlagsSet(CANHandler->EventFlag, CAN_OS_RxFifo1Cplt_Event);
}

void CAN_OS_TxCplt_CB(CAN_OS_HandlerStruct* CANHandler)
{
	osSemaphoreRelease(CANHandler->TxSemaphore);
}
void CAN_OS_RegisterCB(CAN_OS_HandlerStruct *hcan, uint8_t callbackID, void (* pCallback)(CAN_HandleTypeDef *_hcan))
{
	switch (callbackID) {
		case CAN_OS_ActivateTxCB_ID:
			HAL_CAN_RegisterCallback(hcan->hcan, HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID,pCallback);
			HAL_CAN_RegisterCallback(hcan->hcan, HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID,pCallback);
			HAL_CAN_RegisterCallback(hcan->hcan, HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID,pCallback);
			HAL_CAN_RegisterCallback(hcan->hcan, HAL_CAN_TX_MAILBOX0_ABORT_CB_ID,pCallback);
			HAL_CAN_RegisterCallback(hcan->hcan, HAL_CAN_TX_MAILBOX1_ABORT_CB_ID,pCallback);
			HAL_CAN_RegisterCallback(hcan->hcan, HAL_CAN_TX_MAILBOX2_ABORT_CB_ID,pCallback);
			break;
		case CAN_OS_ActivateRxFifo0_ID:
			HAL_CAN_RegisterCallback(hcan->hcan,HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID,pCallback);

			break;
		case CAN_OS_ActivateRxFifo1_ID:
			HAL_CAN_RegisterCallback(hcan->hcan,HAL_CAN_RX_FIFO1_MSG_PENDING_CB_ID,pCallback);

			break;
		default:
			return ;
			break;
	}
}
