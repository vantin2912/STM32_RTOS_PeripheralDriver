#include <CANFrame/CANFrame.h>
#include <string.h>
#include "CRC/CRC.h"

#define CANFRAME_GETMSGTYPE_FROMID(CANID)		((CANID>> 7) 	& 0x0f)
#define CANFRAME_GETTARGETNODE_FROMID(CANID)	((CANID>> 3) 	& 0x0f)
#define CANFRAME_GETFRAMETYPE_FROMID(CANID)		((CANID>> 0) 	& 0x07)
const osThreadAttr_t CANRcvTask_attributes = {
  .name = "CANRcvHandler",
  .stack_size = 200 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

static inline void CANFrame_ClearRcvInfo(CANFrame_RcvInfoTypedef* rcvinfo)
{
	memset(rcvinfo, 0,sizeof( CANFrame_RcvInfoTypedef));
}

static CANFrame_RcvInfoTypedef* CANFrame_ProcessData(CANFrame_HandlerStruct* CANHandler, CAN_RxHeaderTypeDef* RxHeader, uint8_t* RxData)
{
	uint8_t senderID = RxData[0];
	CANFrame_RcvInfoTypedef* RcvInfo = &CANHandler->_RxRcvInfo[senderID];
	uint8_t CpyLen;
	uint8_t RcvFrameType = CANFRAME_GETFRAMETYPE_FROMID(RxHeader->StdId);
	uint8_t MsgType =	CANFRAME_GETMSGTYPE_FROMID(RxHeader->StdId);
	uint8_t CurrentFrameType = RcvInfo->CurrentFrameType;
//	SyncPrintf("Rcv Frame Type %d CurrentFrameType %d \r\n", RcvFrameType, CurrentFrameType);
	uint8_t Receive_CRC;
	uint8_t Calc_CRC;
	if(CurrentFrameType == 0)
	{
		// Empty buffer not receive any frame
		if(RxData[1] > CANFRAME_MAX_BUFFER_SIZE)
		{
			return NULL;
		}
		CANFrame_ClearRcvInfo(RcvInfo);
		RcvInfo->ExpectedLen = RxData[1];
		RcvInfo->MsgType = MsgType;
		RcvInfo->CurrentFrameType = RcvFrameType;
		switch (RcvFrameType) {
			case CANFRAME_FRAMETYPE_FIRST:
				CpyLen = 6;
				memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 2, CpyLen);
				RcvInfo->ReceivedLen += CpyLen;
				return NULL;
			case CANFRAME_FRAMETYPE_END:
				CpyLen = RcvInfo->ExpectedLen;
				memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 2, CpyLen);
				RcvInfo->ReceivedLen += CpyLen;
#ifdef CANFRAME_ENABLE_COUNTER
//				Receive_CRC = RxData[CpyLen + 2];
				RcvInfo->ExpectedLen-=1;
				Receive_CRC = RcvInfo->Data[RcvInfo->ExpectedLen];
				RcvInfo->Data[RcvInfo->ExpectedLen] = 0;
				Calc_CRC = crc_8(RcvInfo->Data, RcvInfo->ExpectedLen);
				if(Receive_CRC == Calc_CRC)
				{
					CANHandler->RcvSucessCounter++;

				}else
				{
					SyncPrintf("Invalid CRC\r\n");
					CANHandler->RcvFailedCounter++;
					return NULL;
				}
#endif
				return RcvInfo;
			default:
				break;
		}
		return NULL;
	} else
	{
		if(RcvInfo->MsgType != MsgType)
		{
			CANFrame_ClearRcvInfo(RcvInfo);
#ifdef CANFRAME_ENABLE_COUNTER
				SyncPrintf("Not same MsgType\r\n");
				CANHandler->RcvFailedCounter++;
#endif
		}
		if(RcvFrameType == CANFRAME_FRAMETYPE_END)
		{
//			SyncPrintf("Frame Type END\r\n");
			RcvInfo->CurrentFrameType = CANFRAME_FRAMETYPE_END;
			uint8_t remainLen = RcvInfo->ExpectedLen - RcvInfo->ReceivedLen;
//			if(remainLen > 7)
//			{
//				CANFrame_ClearRcvInfo(RcvInfo);
//				return NULL;
//			}
			memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 1, remainLen);
			RcvInfo->ReceivedLen += remainLen;

#ifdef CANFRAME_ENABLE_COUNTER
				RcvInfo->ExpectedLen-=1;
				Receive_CRC = RcvInfo->Data[RcvInfo->ExpectedLen];
				RcvInfo->Data[RcvInfo->ExpectedLen] = 0;

				Calc_CRC = crc_8(RcvInfo->Data, RcvInfo->ExpectedLen);
				if(Receive_CRC == Calc_CRC)
				{
					CANHandler->RcvSucessCounter++;
				}else
				{
					SyncPrintf("Invalid CRC Calc 0x%.2x Rcv 0x%2x\r\n", Calc_CRC, Receive_CRC);
					CANHandler->RcvFailedCounter++;
					return NULL;

				}
#endif
//			SyncPrintf("CpyLen %d ReceivedLen %d ExpectedLen %d\r\n", CpyLen, RcvInfo->ReceivedLen, RcvInfo->ExpectedLen);
			return RcvInfo;
		}
		else if (RcvFrameType != CurrentFrameType + 1 )
		{
#ifdef CANFRAME_ENABLE_COUNTER
			SyncPrintf("FrameType not contiguos\r\n");
				CANHandler->RcvFailedCounter++;
#endif
			CANFrame_ClearRcvInfo(RcvInfo);
			return NULL;
		}
		else
		{
//			SyncPrintf("Frame Type %d\r\n", RcvFrameType);
			RcvInfo->CurrentFrameType = RcvFrameType;
			CpyLen = 7;
			memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 1, CpyLen);
			RcvInfo->ReceivedLen += CpyLen;
//			SyncPrintf("CpyLen %d ReceivedLen %d ExpectedLen %d\r\n", CpyLen, RcvInfo->ReceivedLen, RcvInfo->ExpectedLen);
			return NULL;
		}
	}
}

void CANFrame_RcvTask(void* arg)
{
	int Status = 0;
	CANFrame_HandlerStruct* CANHandler = (CANFrame_HandlerStruct*) arg;
	CAN_RxHeaderTypeDef CAN_RxHeader;
	CANFrame_RxHeaderTypedef CANFrame_RxHeader;
	uint8_t RxData[8];
	uint8_t senderID;
	uint8_t TargetNode;
	uint8_t FifoFillLevel = 0;
	while(1)
	{
		CAN_OS_GetRxFifoFillLevel(CANHandler->CAN, CANHandler->RxFifo, &FifoFillLevel);
		if(FifoFillLevel == 0)
		{
			Status = CAN_OS_ListenMsg(CANHandler->CAN, CANHandler->RxFifo, osWaitForever);
			if(Status != osOK)
			{
				continue;
			}
		}
		Status = CAN_OS_GetRxMessage(CANHandler->CAN, CANHandler->RxFifo, &CAN_RxHeader, RxData);
		if(Status != osOK)
		{
			continue;
		}
//		SyncPrintf("Power Rcv ID 0x%.2lx len %ld: ", CAN_RxHeader.StdId, CAN_RxHeader.DLC);
//		for(uint8_t i = 0; i<8; i++)
//		{
//			SyncPrintf("0x%.2x ", RxData[i]);
//		}
//		SyncPrintf("\r\n");
		TargetNode = CANFRAME_GETTARGETNODE_FROMID(CAN_RxHeader.StdId);
		if( !((TargetNode != CANHandler->nodeID) || (TargetNode != CANFRAME_ALL_NODE)))
		{
			continue;
		}
		senderID = RxData[0];
		CANFrame_RcvInfoTypedef* rcvInfo = CANFrame_ProcessData(CANHandler, &CAN_RxHeader, RxData);
		if(rcvInfo != NULL)
		{
			CANFrame_RxHeader.DataLen = rcvInfo->ExpectedLen;
			CANFrame_RxHeader.MessageType = rcvInfo->MsgType;
			CANFrame_RxHeader.senderID = senderID;
//			SyncPrintf("LightGPS Rcv from ID 0x%.2x len %d: \r\n", senderID, CANFrame_RxHeader.DataLen);
//			for(uint8_t i = 0 ; i< CANFrame_RxHeader.DataLen; i++)
//			{
//				SyncPrintf("%d ", rcvInfo->Data[i]);
//			}
//			SyncPrintf("\r\n");
			rcvInfo->Data[CANFrame_RxHeader.DataLen] = 0;
//			SyncPrintf("%s \r\n", rcvInfo->Data);
			if(CANHandler->ReceiveDataCB != NULL)
			{
				CANHandler->ReceiveDataCB(&CANFrame_RxHeader, rcvInfo->Data);
			}
			CANFrame_ClearRcvInfo(rcvInfo);
		}
	}
}

int CANFrame_Init(CANFrame_HandlerStruct* canhandler, CAN_OS_HandlerStruct* CAN, uint16_t nodeID, uint32_t CAN_RxFifo)
{
	memset(canhandler, 0, sizeof(CANFrame_HandlerStruct));
	canhandler->CAN = CAN;
	canhandler->nodeID = nodeID;
	canhandler->usedFilterBank = 0;
	canhandler->RxFifo = CAN_RxFifo;

	canhandler->TxSem = osSemaphoreNew(1, 1, NULL);
	canhandler->rcvHandler_Th = osThreadNew(CANFrame_RcvTask, canhandler, &CANRcvTask_attributes);
	CANFrame_FilterConfig(canhandler, CANFRAME_ALL_NODE, CAN_RxFifo);
	return CANFrame_FilterConfig(canhandler, nodeID, CAN_RxFifo);

}

int CANFrame_Send(CANFrame_HandlerStruct* canhandler, CANFrame_TxHeaderTypedef* CANFrame_txHeader,
							uint8_t *Data , uint32_t timeout)
{
	int Status;

	CAN_TxHeaderTypeDef CAN_TxHeader;
	/*Config Frame----------------------------------------------------------------*/
	CAN_TxHeader.DLC=8;
	CAN_TxHeader.RTR=CAN_RTR_DATA;
	CAN_TxHeader.IDE=CAN_ID_STD;
	/*Config ID-------------------------------------------------------------------*/
	uint32_t Txmailbox;
	uint16_t ID_NUM = (CANFrame_txHeader->MessageType <<7) | CANFrame_txHeader->TargetNode << 3;
	uint8_t Frame_type = CANFRAME_FRAMETYPE_FIRST;

	/*Implement send data----------------------------------------------------------*/
	uint8_t TxFrame[CANFRAME_MAX_DATA_LENGTH] = {0};
	uint32_t FrameIndex = 0;
	uint32_t DataLength = CANFrame_txHeader->DataLen;
	uint8_t isFirstFrame = 1;
	uint8_t isLastFrame=0;
	uint32_t startTime = osKernelGetTickCount();
	uint8_t calcCRC = crc_8(Data, DataLength);
	int waitTime;

	if( CANFrame_txHeader->DataLen > CANFRAME_MAX_BUFFER_SIZE)
	{
		return osErrorParameter;
	}
	/*Add nodeID vs Data length at first frame -----------------------------------*/
#ifdef CANFRAME_ENABLE_COUNTER
	Data[DataLength] = calcCRC;
	DataLength++;
#endif

	osSemaphoreAcquire(canhandler->TxSem, timeout);

	for (int i = 0; i < DataLength; i++)
	{
		uint8_t byte = Data[i];
		if (isFirstFrame)
		{
			TxFrame[FrameIndex] = canhandler->nodeID;
			FrameIndex++;
			TxFrame[FrameIndex] = DataLength;
			FrameIndex++;
			isFirstFrame = 0;
		}
		/*Add byte into frame data----------------------------------------------------*/
		TxFrame[FrameIndex] = byte;
		FrameIndex++;
		/*Check if frame data is not fill, add FILL byte until frame full 8bytes------*/
		if (FrameIndex == CANFRAME_MAX_DATA_LENGTH || i == DataLength - 1)
		{
			TxFrame[FrameIndex] = calcCRC;
			FrameIndex++;
			while (FrameIndex < CANFRAME_MAX_DATA_LENGTH)
			{
				TxFrame[FrameIndex] = CANFRAME_FILL_VALUE;
				FrameIndex++;
			}
			/*Check last frame------------------------------------------------------*/
			if(i == DataLength - 1)
			{
				isLastFrame=1;
				CAN_TxHeader.StdId = ID_NUM | CANFRAME_FRAMETYPE_END;

			}
			if(isLastFrame==0){
				CAN_TxHeader.StdId =ID_NUM | Frame_type;
			}
			/*---------send data-----------------------------------------------------------*/

			if(CANFrame_txHeader->DataLen > 20) osDelay(1);
			waitTime = timeout - (osKernelGetTickCount() - startTime);
			if (waitTime < 0)
			{
				osSemaphoreRelease(canhandler->TxSem);
#ifdef CANFRAME_ENABLE_COUNTER
				canhandler->SendFailedCounter++;
#endif
				return osErrorTimeout;
			}
			Status = CAN_OS_Transmit(canhandler->CAN, &CAN_TxHeader, TxFrame, &Txmailbox, waitTime);
			if(Status != osOK)
			{
				osSemaphoreRelease(canhandler->TxSem);
#ifdef CANFRAME_ENABLE_COUNTER
				canhandler->SendFailedCounter++;
#endif
				return Status;
			}
			memset(TxFrame, 0, CANFRAME_MAX_DATA_LENGTH);
			FrameIndex = 0;
			/*add SenderID for every 1st next frame---------------------------------------*/
			TxFrame[0] = canhandler -> nodeID;
			FrameIndex++;
			Frame_type++;
		}
	}
	osSemaphoreRelease(canhandler->TxSem);
#ifdef CANFRAME_ENABLE_COUNTER
	canhandler->SendSuccessCounter++;
#endif
	return osOK;
}

int CANFrame_RegCB(CANFrame_HandlerStruct* CANHandler, uint8_t CallbackID,
					void (*Func)(CANFrame_RxHeaderTypedef*, uint8_t*))
{
	switch (CallbackID) {
		case CANFRAME_RCVCPLT_CB_ID:
			CANHandler->ReceiveDataCB = Func;
			return osOK;
		default:
			return osErrorParameter;

	}

}

int CANFrame_FilterConfig(CANFrame_HandlerStruct *Can, uint16_t NodeID, uint32_t RxFifo)
{
	CAN_FilterTypeDef Can_filter_init;
	Can_filter_init.FilterActivation=ENABLE;
	Can_filter_init.FilterBank= Can->usedFilterBank++;
	if(Can->usedFilterBank > 14){
		Error_Handler();
	}
	Can_filter_init.FilterFIFOAssignment=RxFifo;
	Can_filter_init.FilterIdHigh=NodeID<<8;
	Can_filter_init.FilterIdLow= 0x0000;
	Can_filter_init.FilterMaskIdHigh= 0x0F00;
	Can_filter_init.FilterMaskIdLow= 0x0000;
	Can_filter_init.FilterMode=CAN_FILTERMODE_IDMASK;
	Can_filter_init.FilterScale=CAN_FILTERSCALE_32BIT;
	if(CAN_OS_ConfigFilter(Can->CAN,&Can_filter_init)!=HAL_OK)
	{
		Error_Handler();
	}
	return osOK;
}
