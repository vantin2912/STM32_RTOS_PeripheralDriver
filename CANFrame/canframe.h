#include "main.h"
#include "CANHandler/CANHandler.h"

#ifndef CAN_MUTIFRAME_H
#define CAN_MUTIFRAME_H

#define CANFRAME_ENABLE_COUNTER
//#define CANFRAME_ENABE_CRC

#define CANFRAME_MAX_NODE							13
#define CANFRAME_MAX_DATA_LENGTH 					(0x08)
#define CANFRAME_FILL_VALUE 						(0x55)
#define CANFRAME_MAX_BUFFER_SIZE 					55

#define CANFRAME_RCVCPLT_CB_ID						0x01

#define CANFRAME_MSGTYPE_BROADCAST 						0b0000
#define CANFRAME_MSGTYPE_COMMAND_FRAME 					0b0001
#define CANFRAME_MSGTYPE_ACK_COMMAND_FRAME 				0b0010
#define CANFRAME_MSGTYPE_REMOTE_NOTICE_FRAME 			0b0011
#define CANFRAME_MSGTYPE_DATA_FRAME						0b0100

#define CANFRAME_MSGTYPE_TEST_BROADCAST 						0b1000
#define CANFRAME_MSGTYPE_TEST_COMMAND_FRAME 					0b1001
#define CANFRAME_MSGTYPE_TEST_ACK_COMMAND_FRAME 				0b1010
#define CANFRAME_MSGTYPE_TEST_REMOTE_NOTICE_FRAME 				0b1011
#define CANFRAME_MSGTYPE_TEST_DATA_FRAME							0b1100

#define CANFRAME_ALL_NODE 						0b0000
#define CANFRAME_ENGINE_CONTROL_ID					0b0001
#define CANFRAME_LIGHT_GPS_ID						0b0010
#define CANFRAME_MASTER_ID							0b0011
#define CANFRAME_STEERING_ID						0b0100
#define CANFRAME_DISTANCE_ID						0b0101
#define CANFRAME_POWER_ID							0b0110
#define CANFRAME_OBSTALCE1_ID						0b1000
#define CANFRAME_OBSTALCE2_ID						0b1001
#define CANFRAME_OBSTALCE3_ID						0b1010
#define CANFRAME_OBSTALCE4_ID						0b1011
#define CANFRAME_OBSTALCE5_ID						0b1100
#define CANFRAME_OBSTALCE6_ID						0b1101
#define CANFRAME_OBSTALCE7_ID						0b1110
#define CANFRAME_OBSTALCE8_ID						0b1111



#define CANFRAME_FRAMETYPE_END 						0b000
#define CANFRAME_FRAMETYPE_FIRST					0b001
#define CANFRAME_FRAMETYPE_SECOND 					0b010
#define CANFRAME_FRAMETYPE_THIRD 					0b011
#define CANFRAME_FRAMETYPE_FOURTH 					0b100
#define CANFRAME_FRAMETYPE_FIFTH					0b101
#define CANFRAME_FRAMETYPE_SIX						0b110
#define CANFRAME_FRAMETYPE_SEVEN 					0b111

typedef struct {
	uint8_t ExpectedLen;
	uint8_t Data[CANFRAME_MAX_BUFFER_SIZE];
	uint8_t	ReceivedLen;
	uint8_t CurrentFrameType;
	uint8_t MsgType;
} CANFrame_RcvInfoTypedef;



typedef struct {
	uint16_t MessageType;
	uint16_t senderID;
	uint8_t DataLen;
}CANFrame_RxHeaderTypedef;

typedef struct {
	uint16_t MessageType;
	uint16_t TargetNode;
	uint16_t DataLen;
}CANFrame_TxHeaderTypedef;


typedef struct CANFrame_HandlerStruct{
	CAN_OS_HandlerStruct * CAN;
	uint16_t nodeID;
	uint16_t usedFilterBank;
	uint32_t RxFifo;

	osSemaphoreId_t TxSem;
	osThreadId_t rcvHandler_Th;
	void (*ReceiveDataCB) (CANFrame_RxHeaderTypedef*, uint8_t* );
	CANFrame_RcvInfoTypedef _RxRcvInfo[CANFRAME_MAX_NODE];
#ifdef CANFRAME_ENABLE_COUNTER
	uint32_t SendSuccessCounter;
	uint32_t SendFailedCounter;
	uint32_t RcvSucessCounter;
	uint32_t RcvFailedCounter;
#endif

}CANFrame_HandlerStruct;


int CANFrame_Init(CANFrame_HandlerStruct* canhandler, CAN_OS_HandlerStruct* CAN, uint16_t nodeID, uint32_t CAN_RxFifo);
int CANFrame_FilterConfig(CANFrame_HandlerStruct *Canhandle, uint16_t NodeID, uint32_t RxFifo);


//int CANFrame_WaitMsg(CANFrame_HandlerStruct* Canhandle,CANFrame_RxHeaderTypedef* pIDtype,uint8_t *ReceiveData, uint32_t *ReceiveLength);
int CANFrame_Send(CANFrame_HandlerStruct* Canhandle,CANFrame_TxHeaderTypedef* pIDtype, uint8_t *Data, uint32_t Timeout);
int CANFrame_RegCB(CANFrame_HandlerStruct* CANHandler, uint8_t CallbackID,
					void (*Func)(CANFrame_RxHeaderTypedef*, uint8_t*));

int CANFrame_SendRequest(CANFrame_HandlerStruct* Canhandle,uint8_t Data);
int CANFrame_RECEIVE_ACK(CANFrame_HandlerStruct* Canhandle);

#endif
