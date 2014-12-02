#define MAX_MESSAGE		100
#define MAX_SIGNAL		64
//#define MAX_SIG_DESCRIP 100
#define MAX_COMMENT		100
#define MAX_ValueDescription		20
typedef unsigned int uint;
typedef char string[128];
typedef char C_identifier[32];

typedef struct ValueDescriptionTag
{
	uint			message_ID;
	C_identifier	signal_name;
	double			value[MAX_ValueDescription];
	string			description[MAX_ValueDescription];
	int				nDescriptionElement;
}ValueDescription;

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
	int						startPosition;
	ValueDescription		sValDecritp;
}Signal;

typedef struct MessageTag 
{
	uint			ID; 
	C_identifier	name;
	int				size;
	C_identifier	transmitter;
	Signal			*prSignal[MAX_SIGNAL];
	int				nSignal;
	int				startPosition;
	uint				isExtend;
}Message;

typedef struct Config_Tag
{
	string dbcFileName;
	string csvFileName;
}Config;

typedef struct DBCInMemory_Tag
{
	char* prDBC;
	int nChar;
}DbcInMemory;
