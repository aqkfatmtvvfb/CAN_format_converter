/********************************************************************
	created:	2013/02/24
	created:	24:2:2013   22:56
	filename: 	c:\Users\WangYu\Desktop\CAN\DBC_alyzer\DBC_alyzer\DBC_alyzer.h
	file path:	c:\Users\WangYu\Desktop\CAN\DBC_alyzer\DBC_alyzer
	file base:	DBC_alyzer
	file ext:	h
	author:		WangYu(wangyu_1988@126.com)
*********************************************************************/
typedef unsigned int uint;
typedef char string[64];
typedef char C_identifier[32];
typedef struct RecordTag
{
	int				period_ms;
	uint			messageID;
	C_identifier	messageName;
	int				messageSize;
	C_identifier	transmitter;
	char			receiver[320];
	C_identifier	signalName;
	char			signalDescription[128];
	string			units;
	double 			Min;
	double			Max;
	double			factor;
	double			offset;
	char			valueDescription[320];
	int				startBit;
	int				signalSize;
	int				byte_order;
	char			value_type;
	C_identifier	receiverNode[10];
	int				nReceiverNode;
}Record;
typedef struct SignalTag
{
	C_identifier			name;
	uint					start_bit;
	uint					signal_size;
	int						byte_order;
	char					value_type;
	double					factor,offset;
	double					minimum,maximum;
	string					unit;
	C_identifier			receiverNode[10];
	int						nReceiverNode;

	
}Signal;

typedef struct MessageTag 
{
	uint			ID; 
	C_identifier	name;
	int				size;
	C_identifier	transmitter;
	Signal			*prSignal;
	int				nSignal;
}Message;

typedef struct ValueDescriptionTag
{
	uint			message_ID;
	C_identifier	signal_name;
	double			value[10];
	string			description[10];
	int				nDescriptionElement;
}ValueDescription;