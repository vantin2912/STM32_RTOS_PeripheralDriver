
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
	HAL_StatusTypeDef Status =  HAL_CAN_ConfigFilter(CANHandler->hcan, Filter);
	return Status == HAL_OK? osOK: osError;
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

static inline void CAN_MailboxSync(CAN_OS_HandlerStruct* CANHandler)
{
	uint32_t FreeMailbox;
	uint32_t semCount;
	while(1)
	{
		FreeMailbox = HAL_CAN_GetTxMailboxesFreeLevel(CANHandler->hcan);
		semCount = osSemaphoreGetCount(CANHandler->TxSemaphore);
		if(semCount == FreeMailbox)
		{
			break;
		}else if( semCount < FreeMailbox)
		{
			osSemaphoreRelease(CANHandler->TxSemaphore);
		}else
		{
			osSemaphoreAcquire(CANHandler->TxSemaphore, 0);
		}
	}
}

int CAN_OS_Transmit(CAN_OS_HandlerStruct* CANHandler, const CAN_TxHeaderTypeDef *txHeader, uint8_t* txData, uint32_t* txMailbox, uint32_t timeout)
{
	int Status;
	CAN_MailboxSync(CANHandler);
	Status = osSemaphoreAcquire(CANHandler->TxSemaphore, timeout);
	if(Status == osErrorTimeout) return HAL_TIMEOUT;
	uint32_t CANError = HAL_CAN_GetError(CANHandler->hcan);
	if(CANError != 0)
	Status = HAL_CAN_AddTxMessage(CANHandler->hcan, txHeader, txData, txMailbox);
//	SyncPrintf("CAN Tx ")
	if (Status != HAL_OK){
		osSemaphoreRelease(CANHandler->TxSemaphore);
		return osError;
	};

	return osOK;
}

//int CAN_OS_WaitMailboxEmpty(CAN_OS_HandlerStruct* CANHandler, uint32_t Mailbox, uint32_t timeout)
//{
//	uint32_t waitEvent;
//	switch (Mailbox) {
//		case 1:
//			waitEvent = CAN_OS_Mailbox0Empty_Event;
//			break;
//		case 2:
//			waitEvent = CAN_OS_Mailbox1Empty_Event;
//			break;
//		case 3:
//			waitEvent = CAN_OS_Mailbox2Empty_Event;
//			break;
//		default:
//			return osErrorParameter;
//			break;
//	}
//	int Status = osEventFlagsWait(CANHandler->EventFlag, waitEvent, osFlagsWaitAll, timeout);
//	return Status > 0 ? osOK : Status;
//}

int CAN_OS_ListenMsg(CAN_OS_HandlerStruct* CANHandler, uint32_t rxFifo, uint32_t timeout)
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
		waitEvent = CAN_OS_RxFifo1Cplt_Event;
	}
	else return osErrorParameter;

	Status = HAL_CAN_ActivateNotification(CANHandler->hcan, activateITS);

	Status = osSemaphoreAcquire(CANHandler->RxSemaphore, timeout);
	if (Status != osOK) return Status;
	Status = osEventFlagsWait(CANHandler->EventFlag, waitEvent, osFlagsWaitAll, timeout);
	osSemaphoreRelease(CANHandler->RxSemaphore);
	return Status > 0 ? osOK : Status;
}

int CAN_OS_GetRxFifoFillLevel(CAN_OS_HandlerStruct* CANHandler, uint32_t rxFifo, uint8_t* FillLevel)
{
	*FillLevel = HAL_CAN_GetRxFifoFillLevel(CANHandler->hcan, rxFifo);
	return osOK;
}

int CAN_OS_GetRxMessage(CAN_OS_HandlerStruct* CANHandler, uint32_t rxFifo, CAN_RxHeaderTypeDef* rxHeader, uint8_t* rxData)
{
	uint8_t Status = HAL_CAN_GetRxMessage(CANHandler->hcan, rxFifo, rxHeader, rxData);
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

int CAN_OS_RegisterCB(CAN_OS_HandlerStruct *hcan, uint8_t callbackID, void (* pCallback)(CAN_HandleTypeDef *_hcan))
{
	switch (callbackID)
	{
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
			return osErrorParameter;
			break;
	}
	return osOK;
}
