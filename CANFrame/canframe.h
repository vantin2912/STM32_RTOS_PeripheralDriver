#include "main.h"
#include "CANHandler/CANHandler.h"

#ifndef CAN_MUTIFRAME_H
#define CAN_MUTIFRAME_H

#define CANFRAME_MAX_DATA_LENGTH 					(0x08)
#define CANFRAME_FILL_VALUE 						(0x55)
#define CANFRAME_MAX_BUFFER_SIZE 					128

#define CANFRAME_MSGTYPE_ALL_NODE 						0b0000
#define CANFRAME_MSGTYPE_COMMAND_FRAME 					0b0001
#define CANFRAME_MSGTYPE_ACK_COMMAND_FRAME 				0b0010
#define CANFRAME_MSGTYPE_REMOTE_NOTICE_FRAME 			0b0011
#define CANFRAME_MSGTYPE_TEST_FRAME 					0b0100
#define CANFRAME_MSGTYPE_ACK_TEST_FRAME 				0b0101

#define CANFRAME_TARGET_ALL_NODE 						0b0000
#define CANFRAME_TARGET_ENGINE_CONTROL					0b0001
#define CANFRAME_TARGET_LIGHT_GPS						0b0010
#define CANFRAME_TARGET_MASTER							0b0011
#define CANFRAME_TARGET_STEERING						0b0100
#define CANFRAME_TARGET_POWER							0b0110
#define CANFRAME_TARGET_OBSTALCE1						0b1000
#define CANFRAME_TARGET_OBSTALCE2						0b1001
#define CANFRAME_TARGET_OBSTALCE3						0b1010
#define CANFRAME_TARGET_OBSTALCE4						0b1011
#define CANFRAME_TARGET_OBSTALCE5						0b1100
#define CANFRAME_TARGET_OBSTALCE6						0b1101
#define CANFRAME_TARGET_OBSTALCE7						0b1110
#define CANFRAME_TARGET_OBSTALCE8						0b1111

#define CANFRAME_FRAMETYPE_END 						0b000
#define CANFRAME_FRAMETYPE_FIRST					0b001
#define CANFRAME_FRAMETYPE_SECOND 					0b010
#define CANFRAME_FRAMETYPE_THIRD 					0b011
#define CANFRAME_FRAMETYPE_FOURTH 					0b100
#define CANFRAME_FRAMETYPE_FIFTH					0b101
#define CANFRAME_FRAMETYPE_SIX						0b110
#define CANFRAME_FRAMETYPE_SEVEN 					0b111

typedef struct CANFrame_HandlerStruct{
	CAN_OS_HandlerStruct * CAN;
	uint16_t SenderID;
	uint16_t usedFilterBank;
}CANFrame_HandlerStruct;

typedef struct {
	uint16_t MessageType;
	uint16_t TargetNode;
	uint16_t Frametype;
	uint8_t ReceivedBuffer[CANFRAME_MAX_BUFFER_SIZE];
	uint8_t Index;
	uint8_t ExpectedLength;
	uint8_t frameIndex;
	uint8_t isLastFrame;
}CANFrame_RxHeaderTypedef;

typedef struct {
	uint16_t MessageType;
	uint16_t TargetNode;
	uint16_t DataLen;
}CANFrame_TxHeaderTypedef;

int CANFrame_Init(CANFrame_HandlerStruct* canhandler, CAN_OS_HandlerStruct* CAN, uint16_t senderID);
int CANFrame_FilterConfig(CANFrame_HandlerStruct *Canhandle, uint16_t NodeID);


int CANFrame_WaitMsg(CANFrame_HandlerStruct* Canhandle,CANFrame_RxHeaderTypedef* pIDtype,uint8_t *ReceiveData, uint32_t *ReceiveLength);
int CANFrame_Send(CANFrame_HandlerStruct* Canhandle,CANFrame_TxHeaderTypedef* pIDtype, uint8_t *Data, uint32_t Timeout);

int CANFrame_SendRequest(CANFrame_HandlerStruct* Canhandle,uint8_t Data);
int CANFrame_RECEIVE_ACK(CANFrame_HandlerStruct* Canhandle);

#endif
