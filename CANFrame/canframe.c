#include <CANFrame/CANFrame.h>
#include <string.h>


#define CANFRAME_GETMSGTYPE_FROMID(CANID)			((CANID>> 7) 	& 0x0f)
#define CANFRAME_GETTARGETNODE_FROMID(CANID)	((CANID>> 3) 	& 0x0f)
#define CANFRAME_GETFRAMETYPE_FROMID(CANID)		((CANID)		& 0x07)


const osThreadAttr_t CANRcvTask_attributes = {
  .name = "CANRcvHandler",
  .stack_size = 200 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
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
	SyncPrintf("Rcv Frame Type %d CurrentFrameType %d \r\n", RcvFrameType, CurrentFrameType);

	if(CurrentFrameType == 0)
	{
		// Empty buffer not receive any frame
		if(RxData[1] > CANFRAME_MAX_BUFFER_SIZE)
		{
			return NULL;
		}
		RcvInfo->ExpectedLen = RxData[1];
		RcvInfo->MsgType = MsgType;
		RcvInfo->CurrentFrameType = RcvFrameType;
		switch (RcvFrameType) {
			case CANFRAME_FRAMETYPE_FIRST:
				SyncPrintf("\r\n________________________\r\n");

				SyncPrintf("Frame Type First \r\n");
				CpyLen = 6;
				memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 2, CpyLen);
				RcvInfo->ReceivedLen += CpyLen;
				SyncPrintf("CpyLen %d ReceivedLen %d ExpectedLen %d\r\n", CpyLen, RcvInfo->ReceivedLen, RcvInfo->ExpectedLen);
				return NULL;
			case CANFRAME_FRAMETYPE_END:
				CpyLen = RcvInfo->ExpectedLen;
				memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 2, CpyLen);
				RcvInfo->ReceivedLen += CpyLen;
				SyncPrintf("CpyLen %d ReceivedLen %d ExpectedLen %d\r\n", CpyLen, RcvInfo->ReceivedLen, RcvInfo->ExpectedLen);
				return RcvInfo;
			default:
				CpyLen = 0;
				break;
		}

		return NULL;

	} else
	{
		if(RcvInfo->MsgType != MsgType)
		{
			CANFrame_ClearRcvInfo(RcvInfo);
		}
		if(RcvFrameType == CANFRAME_FRAMETYPE_END)
		{
			SyncPrintf("Frame Type END\r\n");
			RcvInfo->CurrentFrameType = CANFRAME_FRAMETYPE_END;
			CpyLen = RcvInfo->ExpectedLen - RcvInfo->ReceivedLen;
			memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 1, CpyLen);
			RcvInfo->ReceivedLen += CpyLen;
			SyncPrintf("CpyLen %d ReceivedLen %d ExpectedLen %d\r\n", CpyLen, RcvInfo->ReceivedLen, RcvInfo->ExpectedLen);
			return RcvInfo;
		}
		else if (RcvFrameType != CurrentFrameType + 1 )
		{
			CANFrame_ClearRcvInfo(RcvInfo);
			return NULL;
		}
		else
		{
			SyncPrintf("Frame Type %d\r\n", RcvFrameType);

			RcvInfo->CurrentFrameType = RcvFrameType;
			CpyLen = 7;
			memcpy(RcvInfo->Data + RcvInfo->ReceivedLen, RxData + 1, CpyLen);
			RcvInfo->ReceivedLen += CpyLen;
			SyncPrintf("CpyLen %d ReceivedLen %d ExpectedLen %d\r\n", CpyLen, RcvInfo->ReceivedLen, RcvInfo->ExpectedLen);

			return NULL;
		}
	}



}

void CANFrame_RcvTask(void* arg)
{
	int Status;
	CANFrame_HandlerStruct* CANHandler = (CANFrame_HandlerStruct*) arg;
	CAN_RxHeaderTypeDef CAN_RxHeader;
	CANFrame_RxHeaderTypedef CANFrame_RxHeader;
	uint8_t RxData[8];
	uint8_t senderID;
	uint8_t TargetNode;
	while(1)
	{
		Status = CAN_OS_ListenMsg(CANHandler->CAN, CANHandler->RxFifo, &CAN_RxHeader, RxData, osWaitForever);
//		if(CAN_RxHeader->)
		if(Status != osOK)
		{
			SyncPrintf("Listen Failed %d \r\n", Status);
			continue;
		}
		TargetNode = CANFRAME_GETTARGETNODE_FROMID(CAN_RxHeader.StdId);
		if( !((TargetNode != CANHandler->nodeID) || (TargetNode != CANFRAME_ALL_NODE)))
		{
			SyncPrintf("Receive Wrong Target = %d \r\n", TargetNode);
			continue;
		}
		senderID = RxData[0];
		CANFrame_RcvInfoTypedef* rcvInfo = CANFrame_ProcessData(CANHandler, &CAN_RxHeader, RxData);
		if(rcvInfo != NULL)
		{
			CANFrame_RxHeader.DataLen = rcvInfo->ExpectedLen;
			CANFrame_RxHeader.MessageType = rcvInfo->MsgType;
			CANFrame_RxHeader.senderID = senderID;
			SyncPrintf("LightGPS Rcv from ID 0x%.2x len %d: ", senderID, CANFrame_RxHeader.DataLen);
			for(uint8_t i = 0; i< CANFrame_RxHeader.DataLen ; i++)
			{
				SyncPrintf("%d ", rcvInfo->Data[i]);
			}
			SyncPrintf("\r\n");
//			CANHandler->ReceiveDataCB(CANHandler->ReceiveDataCB_arg, &CANFrame_RxHeader, rcvInfo->Data);
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
	canhandler->rcvHandler_Th = osThreadNew(CANFrame_RcvTask, canhandler, &CANRcvTask_attributes);
	CANFrame_FilterConfig(canhandler, CANFRAME_MSGTYPE_BROADCAST, CAN_RxFifo);
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
	uint32_t waitTime;

	if( CANFrame_txHeader->DataLen > CANFRAME_MAX_BUFFER_SIZE)
	{
		return osErrorParameter;
	}
	/*Add nodeID vs Data length at first frame -----------------------------------*/
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
//			SyncPrintf("Transmit ID 0x%.2x: ", CAN_TxHeader.StdId);
//			for(uint8_t i = 0; i<8; i++)
//			{
//				SyncPrintf("%d ", TxFrame[i]);
//			}
//			osDelay(1);
			waitTime = timeout - (osKernelGetTickCount() - startTime);

			Status = CAN_OS_Transmit(canhandler->CAN, &CAN_TxHeader, TxFrame, &Txmailbox, waitTime);
//			SyncPrintf("Mail box %d\r\n", (uint16_t)Txmailbox);
			if(Status != osOK)
			{
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
	return osOK;
}
int CANFrame_RegCB(CANFrame_HandlerStruct* CANHandler, uint8_t CallbackID,
					void (*Func)(void*, CANFrame_RxHeaderTypedef*, uint8_t*), void* arg)
{
	switch (CallbackID) {
		case CANFRAME_RCVCPLT_CB_ID:
			CANHandler->ReceiveDataCB = Func;
			CANHandler->ReceiveDataCB_arg = arg;
			return HAL_OK;
		default:
			return osErrorParameter;

	}

}

//int CANFrame_WaitMsg(CANFrame_HandlerStruct* canhandler, CANFrame_RxHeaderTypedef* pIDtype,uint8_t *ReceiveData, uint32_t *ReceiveLength)
//{
//	CAN_RxHeaderTypeDef RxHeader;
//	uint8_t frame[CAN_MAX_DATA_LENGTH] = {0};
//	uint8_t isLastFrame = 0;
//	uint8_t SenderId=0;
//	uint8_t LengthRecData=0;
//	CANConfigIDRxtypedef *Rec= (CANConfigIDRxtypedef*) malloc(CAN_MAX_DATA_LENGTH);
//	for (int i=0; i< 13; i++)
//	{
//		Rec[i].Index=0;
//		Rec[i].ExpectedLength=0;
//		Rec[i].frameIndex=0;
//	}
//		while(!isLastFrame){
//			while (HAL_CAN_GetRxFifoFillLevel(canhandler->hcan, CAN_RX_FIFO0) == 0);
//			if (HAL_CAN_GetRxMessage(canhandler->hcan, CAN_RX_FIFO0, &RxHeader, frame) != HAL_OK)
//			{
//				Error_Handler();
//			}
//			SenderId=frame[0];
//			uint16_t ID_NUM = RxHeader.StdId;
//			pIDtype -> Frametype = ID_NUM & 0x07;
//			pIDtype -> TargetNode = (ID_NUM >> 3) & 0x0F;
//			pIDtype ->  MessageType= (ID_NUM >> 7) & 0x0F;
//			switch (pIDtype->Frametype) {
//			  case FIRST_FRAME:
//			    Rec[SenderId].ExpectedLength = frame[1];
//			    Rec[SenderId].frameIndex = 2;
//			    break;
//			  case END_FRAME:
//			    Rec[SenderId].frameIndex = 1;
//			    isLastFrame = 1;
//			    break;
//			  default:
//			    Rec[SenderId].frameIndex = 1;
//			    break;
//			}
//			for(;Rec[SenderId].frameIndex<CAN_MAX_DATA_LENGTH;Rec[SenderId].frameIndex++){
//				Rec[SenderId].ReceivedBuffer[Rec[SenderId].Index]=frame[Rec[SenderId].frameIndex];
//				Rec[SenderId].Index++;
//			}
//		}
//	*ReceiveLength=Rec[SenderId].ExpectedLength;
//	memcpy(ReceiveData,Rec[SenderId].ReceivedBuffer,Rec[SenderId].ExpectedLength);
//	memset(Rec[SenderId].ReceivedBuffer,0,Rec[SenderId].ExpectedLength);
//	Rec[SenderId].Index=0;
//	free(Rec);
//	return HAL_OK;
//}

int CAN_Send_Request(CANFrame_HandlerStruct* canhandler,uint8_t* Data)
{
	uint32_t Txmailbox;
	CAN_TxHeaderTypeDef Txheader;
	Txheader.DLC=1;
	Txheader.RTR=CAN_RTR_DATA;
	Txheader.IDE=CAN_ID_STD;
	Txheader.StdId=CANFRAME_MSGTYPE_TEST_FRAME;
	return CAN_OS_Transmit(canhandler->CAN, &Txheader, Data, &Txmailbox, 20);

}

//uint16_t CAN_RECEIVE_ACK(CANFrame_HandlerStruct* canhandler)
//{
//	CAN_RxHeaderTypeDef Rxheader;
//	uint8_t frame=0;
//	uint8_t ack=0;
//	while (HAL_CAN_GetRxFifoFillLevel(canhandler->hcan, CAN_RX_FIFO0) == 0)
//	if (HAL_CAN_GetRxMessage(canhandler->hcan, CAN_RX_FIFO0, &Rxheader, frame) != HAL_OK)
//	{
//		Error_Handler();
//	}
//	if(Rxheader.StdId==TEST_FRAME&&frame==1){
//		return HAL_ERROR;
//	}else if(Rxheader.StdId==TEST_FRAME&&frame==0)
//	{
//		return HAL_OK;
//	}
//}


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
