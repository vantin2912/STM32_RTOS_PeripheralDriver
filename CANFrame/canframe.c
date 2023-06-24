#include <CANFrame/CANFrame.h>
#include <string.h>

int CANFrame_Init(CANFrame_HandlerStruct* canhandler, CAN_OS_HandlerStruct* CAN, uint16_t senderID)
{
	canhandler->CAN = CAN;
	canhandler->SenderID = senderID;
	canhandler->usedFilterBank = 0;
	return osOK;
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
	/*Add SenderID vs Data length at first frame -----------------------------------*/
	for (int i = 0; i < DataLength; i++)
	{
		uint8_t byte = Data[i];
		if (isFirstFrame)
		{
			TxFrame[FrameIndex] = DataLength;
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
			/*send data--------------------------------------------------------------------*/
			SyncPrintf("Transmit ID 0x%.2x: ", CAN_TxHeader.StdId);
			for(uint8_t i = 0; i<8; i++)
			{
				SyncPrintf("%d ", TxFrame[i]);
			}

			waitTime = timeout - (osKernelGetTickCount() - startTime);
			Status = CAN_OS_Transmit(canhandler->CAN, &CAN_TxHeader, TxFrame, &Txmailbox, waitTime);
			SyncPrintf("Mail box %d\r\n", (uint16_t)Txmailbox);
			if(Status != osOK)
			{
				return Status;
			}
			memset(TxFrame, 0, CANFRAME_MAX_DATA_LENGTH);
			FrameIndex = 0;
			/*add SenderID for every 1st next frame---------------------------------------*/
			TxFrame[0] = canhandler -> SenderID;
			FrameIndex++;
			Frame_type++;
		}
	}
	return osOK;
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


int CANFrame_FilterConfig(CANFrame_HandlerStruct *Can, uint16_t NodeID)
{
	CAN_FilterTypeDef Can_filter_init;
	Can_filter_init.FilterActivation=ENABLE;
	Can_filter_init.FilterBank= Can->usedFilterBank;
	if(Can->usedFilterBank > 14){
		Error_Handler();
	}
	Can_filter_init.FilterFIFOAssignment=CAN_RX_FIFO0;
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
	return HAL_OK;
}
