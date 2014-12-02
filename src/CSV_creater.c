#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "CSV_creater.h"

int readConfig(Config* prConfig);
int initialize(Config const * const prConfig,DbcInMemory* prDbcInMemory);
int isMessage(int const iChar,char const * const prDBC);
int addMessage(Message *ptrMessage[], int curMessage, char *dbcFile, int charIndex);
	static Message* initMessageMemory();
	static int getMessageID(Message *pMessage, char* dbcFILE, int charIndex);
	static int getMessageName(Message *pMessage, char* dbcFILE, int charIndex);
	static int getMessageSize(Message *pMessage, char* dbcFILE, int charIndex);
	static int getMessageTransmitter(Message *pMessage, char* dbcFILE, int charIndex);
int isSignal(int const iChar,char const * const prDBC);
int readSignal(Message *ptrMessage[], int curMessage, char *dbcFile, int charIndex);
	static Signal*	initSignalMemory();
	static int	getSignalNmae(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalStartBit(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalSize(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalByteOrder(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalValueType(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalFactor(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalOffset(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalMin(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalMax(Signal *pSignal, char* dbcFILE, int charIndex);
	static int calculate_LSB_bit(int start_bit,int byte_order,int signal_size);
		static int calculate_LSB_intel(int MSB,int length);
			static int coordinateChange(int org_M, int org_N, int length);		
	static int	getSignalUnit(Signal *pSignal, char* dbcFILE, int charIndex);
	static int	getSignalReceiver(Signal *pSignal, char* dbcFILE, int charIndex);
int isValueDescription(int const iChar,char const * const prDBC);
	int readValDescript(Message *ptrMessage[], int curMessage, char *dbcFile, int charIndex);
	static int	getValDesMessageID(ValueDescription *pVal, char* dbcFILE, int charIndex);
	static int	getValDesSignalName(ValueDescription *pVal, char* dbcFILE, int charIndex);
	static int	getValDesList(ValueDescription *pVal, char* dbcFILE, int charIndex);
	static int	matchSignal(Message *ptrMessage[], ValueDescription *tempValDescript, int nMessage);
int isComment(int const iChar,char const * const prDBC);
int writeCSV(Config* prConfig, Message *ptrMessage[], int nMessage);
int freeMessage(Message *ptrMessage[], int nMaxDimension);
int freeComment();
int freeMem(char* dbc_file);

int main()
{
	DbcInMemory dbcMemory;
	Config config;
	Message *prMessage[MAX_MESSAGE];
	int iMessage= 0;
	int iChar;
	int nMessage;
	readConfig(&config);
	initialize(&config,&dbcMemory);
	iChar=0;
	while(iChar<dbcMemory.nChar-4)
	{
		if (1==isMessage(iChar,dbcMemory.prDBC))
		{
			iChar	= addMessage(prMessage, iMessage, dbcMemory.prDBC, iChar);
			iMessage	= iMessage + 1;			
		}
		else if (1==isSignal(iChar,dbcMemory.prDBC))
		{
			iChar	= readSignal(prMessage, iMessage-1, dbcMemory.prDBC, iChar);
		}
		else if (1==isValueDescription(iChar,dbcMemory.prDBC))
		{
			iChar	= readValDescript(prMessage, iMessage, dbcMemory.prDBC, iChar);
		}
		else if (1==isComment(iChar,dbcMemory.prDBC))
		{
			iChar++;
		}
		else if ((dbcMemory.prDBC[iChar]=='B')&&(dbcMemory.prDBC[iChar+1]=='A')
			&&(dbcMemory.prDBC[iChar+2]=='_'))
		{
			do 
			{
				iChar++;
			} while (dbcMemory.prDBC[iChar]!='\n');
		}
		else
		{
			iChar++;
		}
	}
	nMessage=iMessage;
	writeCSV(&config,prMessage, nMessage);

	freeMessage(prMessage, nMessage);
	freeMem(dbcMemory.prDBC);

	return 1;
}
int readConfig(Config* prConfig)
{
	string FileName;
	printf("****************************************************\n");
	printf("Version:\t\t20140320\n");
	printf("Author:\t\t\tYu Wang, Fei Han\n");
	printf("Contact Information:\twangyu_1988@126.com\n");
	printf("****************************************************\n");
	printf("请输入dbc文件名，不带后缀\n");
	scanf("%s",FileName);
	strcpy(prConfig->dbcFileName,FileName);
	strcat(prConfig->dbcFileName,".dbc");
	strcpy(prConfig->csvFileName,FileName);
	strcat(prConfig->csvFileName,"_c.csv");
	return 1;
}
int initialize(Config const * const prConfig,DbcInMemory* prDbcInMemory)
{
	FILE *dbcfile;
	char char_temp;
	int iChar;
	//****************************************************
	// 打开dbc文件
	if((dbcfile=fopen(prConfig->dbcFileName,"r"))==NULL)
	{
		printf("Can not open dbc file\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	}
	//****************************************************
	// 计算总的字符数
	prDbcInMemory->nChar=0;
	char_temp=fgetc(dbcfile);
	while(!feof(dbcfile))
	{
		prDbcInMemory->nChar++;
		char_temp=fgetc(dbcfile);		
	}
	//****************************************************
	// 分配内存
	prDbcInMemory->prDBC=(char *)malloc(prDbcInMemory->nChar*sizeof(char));
	memset(prDbcInMemory->prDBC,0,prDbcInMemory->nChar*sizeof(char));
	//****************************************************
	// 将文档保存到内存中
	rewind(dbcfile);
	iChar=0;
	while (iChar<prDbcInMemory->nChar)
	{
		prDbcInMemory->prDBC[iChar]=fgetc(dbcfile);
		if (prDbcInMemory->prDBC[iChar]==-80)
		{
			prDbcInMemory->prDBC[iChar]=32;
		}
		//if (prDbcInMemory->prDBC[iChar]==0x10)
		//{
		//	prDbcInMemory->prDBC[iChar]='a';
		//}
		iChar=iChar+1;		
	}
	//****************************************************
	// 关文件
	fclose(dbcfile);
	dbcfile=NULL;
	return 1;
}
int isMessage(int const iChar,char const * const prDBC)
{
	int flag;
	if ((prDBC[iChar]=='B')&&(prDBC[iChar+1]=='O')&&(prDBC[iChar+2]=='_')&&(prDBC[iChar+3]==' '))
	{
		flag=1;
	}
	else
	{
		flag=0;
	}
	return flag;
}
int addMessage(Message *ptrMessage[], int curMessage, char *dbcFile, int charIndex)
{
	int i	= charIndex;
	ptrMessage[curMessage]	= initMessageMemory();
	ptrMessage[curMessage]->startPosition	= i;
	i	= i+4;

	//读ID
	i	= getMessageID(ptrMessage[curMessage], dbcFile, i);

	//读Message Name
	i	= getMessageName(ptrMessage[curMessage], dbcFile, i);

	//读Message Size
	i	= getMessageSize(ptrMessage[curMessage], dbcFile, i);

	//读Message发送节点
	i	= getMessageTransmitter(ptrMessage[curMessage], dbcFile, i);

	return i;
}
static Message* initMessageMemory()
{
	Message *pMessage	= NULL;

	pMessage	= (Message*) malloc(sizeof(Message));

	memset(pMessage,0,sizeof(Message));

	return	pMessage;
}


static int getMessageID(Message *pMessage, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i = charIndex;
	int k;
	unsigned long rawID;

	char_temp = dbcFILE[i];

	k=0;
	while (char_temp!=' ')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';

	rawID=strtoul(string_temp,NULL,10); 	//惨痛的教训！atol返回值最大是0x7FFFFFFF。 Yu Wang 20140320		
	if (1==(rawID>>31))
	{
		pMessage->isExtend=1;
	} 
	else
	{
		pMessage->isExtend=0;
	}
	pMessage->ID=rawID & (0x7FFFFFFF);
	
	return i;
}


static int getMessageName(Message *pMessage, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i = charIndex;
	int k;

	char_temp = dbcFILE[++i];
	k=0;
	while(char_temp!=':')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';

	strcpy(pMessage->name,string_temp);

	return i;
}
static int getMessageSize(Message *pMessage, char* dbcFILE, int charIndex)
{
	char char_temp;
	int i	= charIndex;

	char_temp = dbcFILE[++i];
	char_temp = dbcFILE[++i];
	pMessage->size = (int)char_temp - 48;

	return i;
}
static int getMessageTransmitter(Message *pMessage, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];

	k=0;
	while(1)
	{
		if (char_temp!='\n')
		{
			string_temp[k++]	= char_temp;
		} 
		else
		{
			string_temp[k]='\0';
			k=0;
			strcpy(pMessage->transmitter,string_temp);
			break;
		}				
		char_temp	= dbcFILE[++i];//读完Message，再读了一个\n
	}

	return i;
}
int isSignal(int const iChar,char const * const prDBC)
{
	int flag;
	if ((prDBC[iChar]=='S')&&(prDBC[iChar+1]=='G')&&(prDBC[iChar+2]=='_')&&(prDBC[iChar+3]==' '))
	{
		flag=1;
	}
	else
	{
		flag=0;
	}
	return flag;
}
int readSignal(Message *ptrMessage[], int curMessage, char *dbcFile, int charIndex)
{
	int i	= charIndex;
	int curSignalIndex	= ptrMessage[curMessage]->nSignal;
	Signal *ptrSignal = NULL;

	ptrMessage[curMessage]->nSignal++;

	ptrMessage[curMessage]->prSignal[curSignalIndex]	= initSignalMemory();

	ptrMessage[curMessage]->prSignal[curSignalIndex]->startPosition = i;
	i = i+4;

	ptrSignal	= ptrMessage[curMessage]->prSignal[curSignalIndex];

	//读Signal Name
	i	= getSignalNmae(ptrSignal, dbcFile, i);

	//读Signal Start Bit
	i	= getSignalStartBit(ptrSignal, dbcFile, i);

	//读Signal Size
	i	= getSignalSize(ptrSignal, dbcFile, i);

	//读Signal ByteOrder
	i	= getSignalByteOrder(ptrSignal, dbcFile, i);

	//读Signal ValueType
	i	= getSignalValueType(ptrSignal, dbcFile, i);

	//读Signal Factor
	i	= getSignalFactor(ptrSignal, dbcFile, i);

	//读Signal Offset
	i	= getSignalOffset(ptrSignal, dbcFile, i);

	//读Signal Minimum
	i	= getSignalMin(ptrSignal, dbcFile, i);

	//读Signal Maximum
	i	= getSignalMax(ptrSignal, dbcFile, i);

	//整理Signal Motorora StartBit
	ptrSignal->start_bit	= (uint)(calculate_LSB_bit(
		ptrSignal->start_bit,
		ptrSignal->byte_order,
		ptrSignal->signal_size
		));

	//读Signal Unit
	i	= getSignalUnit(ptrSignal, dbcFile, i);

	//读接收节点
	i	= getSignalReceiver(ptrSignal, dbcFile, i);


	return i;
}


static Signal*	initSignalMemory()
{
	Signal *pSignal	= NULL;

	pSignal	= (Signal*) malloc(sizeof(Signal));

	memset(pSignal,0,sizeof(Signal));

	return	pSignal;
}


static int	getSignalNmae(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char	char_temp;
	string	string_temp;
	int i	= charIndex;
	int k;

	char_temp=dbcFILE[i];
	k=0;
	while(char_temp!=' ')
	{
		string_temp[k++]=char_temp;
		char_temp=dbcFILE[++i];
	}
	string_temp[k]='\0';

	strcpy(pSignal->name,string_temp);

	return i;
}


static int	getSignalStartBit(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	k=0;

	while(char_temp!='|')
	{
		string_temp[k++]	= char_temp;
		char_temp=dbcFILE[++i];
	}
	string_temp[k]='\0';

	pSignal->start_bit	= (unsigned int)(atoi(string_temp));

	return i;
}


static int	getSignalSize(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp=dbcFILE[++i];
	k=0;

	while(char_temp!='@')
	{
		string_temp[k++]=char_temp;
		char_temp=dbcFILE[++i];
	}
	string_temp[k]='\0';

	pSignal->signal_size	= (unsigned int)(atoi(string_temp));

	return i;
}


static int	getSignalByteOrder(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	int i	= charIndex;

	char_temp	= dbcFILE[++i];
	pSignal->byte_order	= (int)char_temp - 48;

	return i;
}


static int	getSignalValueType(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	int i	= charIndex;

	char_temp	= dbcFILE[++i];

	pSignal->value_type	= char_temp;

	return i;
}


static int	getSignalFactor(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];

	k=0;

	while(char_temp!=',')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';

	pSignal->factor	= atof(string_temp);

	return i;
}


static int	getSignalOffset(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	k=0;

	while(char_temp!=')')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';

	pSignal->offset=atof(string_temp);

	return i;
}


static int	getSignalMin(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	k=0;

	while(char_temp!='|')
	{
		string_temp[k++]=char_temp;
		char_temp=dbcFILE[++i];
	}
	string_temp[k]='\0';

	pSignal->minimum	= atof(string_temp);

	return i;
}


static int	getSignalMax(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	k=0;

	while(char_temp!=']')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';

	pSignal->maximum=atof(string_temp);

	return i;
}
//hanfei changed: 2014-01-02
static int coordinateChange(int org_M, int org_N, int length)
{
	int new_M, new_N;
	int result_M, result_N;
	int tempNum;

	new_M	= org_M;
	new_N	= 7 - org_N;

	tempNum	= new_M * 8 + new_N;

	tempNum	= length -1 + tempNum;

	result_N	= tempNum%8;
	result_M	= tempNum/8;

	result_N	= 7 - result_N;

	return 8*result_M+result_N;
}


static int calculate_LSB_intel(int MSB,int length)
{
	int mMSB,nMSB,LSB;
	mMSB=MSB/8;
	nMSB=MSB%8;

	LSB	= coordinateChange(mMSB, nMSB, length);

	return LSB;
}


static int calculate_LSB_bit(int start_bit,int byte_order,int signal_size)
{
	int LSB;
	if (1==byte_order)  // intel format
	{
		LSB=start_bit;
	} 
	else
	{
		LSB=calculate_LSB_intel(start_bit,signal_size); 
	}
	return LSB;
}
static int	getSignalUnit(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	char_temp	= dbcFILE[++i];
	k=0;

	while(char_temp!='\"')//是不是“
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];

	}
	string_temp[k]='\0';

	strcpy(pSignal->unit,string_temp);

	return i;
}


static int	getSignalReceiver(Signal *pSignal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int	i = charIndex;
	int k,j;

	do 
	{
		char_temp	= dbcFILE[++i];
	} while (char_temp==' ');

	k=0;
	j=0;

	while(1)
	{
		if ((char_temp!=',')&&(char_temp!='\n'))
		{
			string_temp[k++]=char_temp;
		} 
		else if (char_temp==',')
		{
			string_temp[k]='\0';
			k=0;
			strcpy(pSignal->receiverNode[j++],string_temp);
		}
		else if (char_temp=='\n')
		{
			string_temp[k]='\0';
			k=0;
			strcpy(pSignal->receiverNode[j++],string_temp);
			pSignal->nReceiverNode	= j;
			break;
		}				
		char_temp	= dbcFILE[++i];
	}

	return i;
}
int isValueDescription(int const iChar,char const * const prDBC)
{
	int flag;
	if ((prDBC[iChar]=='V')&&(prDBC[iChar+1]=='A')&&(prDBC[iChar+2]=='L')&&(prDBC[iChar+3]=='_')&&(prDBC[iChar+4]==' '))
	{
		flag=1;
	}
	else
	{
		flag=0;
	}
	return flag;
}
int readValDescript(Message *ptrMessage[], int curMessage, char *dbcFile, int charIndex)
{
	int i	= charIndex +5;
	//int errorFlag;
	//int nMessageIndex, nSignalIndex;
	ValueDescription tempValDescript;

	//读Value Description MessageID
	i	= getValDesMessageID(&tempValDescript, dbcFile, i);

	//读Value Description SignalName
	i	= getValDesSignalName(&tempValDescript, dbcFile, i);

	//读Value Description List
	i	= getValDesList(&tempValDescript, dbcFile, i);

	//整理tempValue到Message
	matchSignal(ptrMessage, &tempValDescript, curMessage);

	return i;
}


static int	getValDesMessageID(ValueDescription *pVal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k=0;

	char_temp	= dbcFILE[i];
	while (char_temp!=' ')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';
	pVal->message_ID	= strtoul(string_temp,NULL,10) & (0x7FFFFFFF);
	return i;
}


static int	getValDesSignalName(ValueDescription *pVal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int k;

	char_temp	= dbcFILE[++i];
	k=0;

	while(char_temp!=' ')
	{
		string_temp[k++]	= char_temp;
		char_temp			= dbcFILE[++i];
	}
	string_temp[k]='\0';

	strcpy(pVal->signal_name,string_temp);

	return i;
}


static int	getValDesList(ValueDescription *pVal, char* dbcFILE, int charIndex)
{
	char char_temp;
	string string_temp;
	int i	= charIndex;
	int nValDesIndex, k;

	k	= nValDesIndex	= 0;

	char_temp		=dbcFILE[i];
	nValDesIndex	= 0;
	pVal->nDescriptionElement = 0;

	while(char_temp != ';')
	{
		char_temp = dbcFILE[++i];
		k = 0;
		if(char_temp>=48 && char_temp<=57)
		{
			while(char_temp != ' ')
			{
				string_temp[k++]	= char_temp;
				char_temp			= dbcFILE[++i];
			}
			string_temp[k] = '\0';

			k=0;
			pVal->value[nValDesIndex]	= strtoul(string_temp,NULL,10) & (0x7FFFFFFF);
		}

		if(char_temp == '\"')
		{
			char_temp			= dbcFILE[++i];

			while(char_temp != '\"')
			{
				string_temp[k++]	= char_temp;
				char_temp			= dbcFILE[++i];
			}
			string_temp[k]	= '\0';

			strcpy(pVal->description[nValDesIndex],string_temp);

			nValDesIndex++;
		}
	}
	pVal->nDescriptionElement	= nValDesIndex;

	return i;
}


static int	matchSignal(Message *ptrMessage[], ValueDescription *tempValDescript, int nMessage)
{
	int i=0;
	int messageIndex	= 0;
	int signalIndex	= 0;

	while(1)
	{
		if(i>nMessage)
		{
			printf("Doesn't match value description to message\n");
			system("pause");
			exit(0);
		}

		if(ptrMessage[i]->ID == tempValDescript->message_ID)
		{
			messageIndex	= i;
			break;
		}
		else
			i++;
	}

	i=0;
	while(1)
	{
		if(i>ptrMessage[messageIndex]->nSignal)
		{
			printf("Doesn't match value description to signal\n");
			system("pause");
			exit(0);
		}

		if(strcmp(tempValDescript->signal_name, ptrMessage[messageIndex]->prSignal[i]->name) == 0)
		{
			signalIndex	= i;
			break;
		}
		else
			i++;
	}

	memcpy(&(ptrMessage[messageIndex]->prSignal[signalIndex]->sValDecritp),tempValDescript,sizeof(ValueDescription));

	return 1;
}
int isComment(int const iChar,char const * const prDBC)
{
	int flag;
	if ((prDBC[iChar]=='C')&&(prDBC[iChar+1]=='M')&&(prDBC[iChar+2]=='_')&&(prDBC[iChar+3]==' '))
	{
		flag=1;
	}
	else
	{
		flag=0;
	}
	return flag;
}
int writeCSV(Config* prConfig, Message *ptrMessage[], int nMessage)
{
	FILE *csv;
	int iMessage, iSignal;
	int i;
	if((csv=fopen(prConfig->csvFileName,"w"))==NULL)
	{
		printf("Can not open csv file\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	};

	//输出到csv中
	fprintf(csv,"cycle,ID,isExt,Message,size,Tx,Rx,Signal,Units,Min,Max,factor,offset,description,start,length,IorM,sign\n");
	for (iMessage=0;iMessage<nMessage;iMessage++)
	{
		for (iSignal=0;iSignal<ptrMessage[iMessage]->nSignal;iSignal++)
		{
			fprintf(csv,"50,%u,%u,%s,%d,%s,",
					ptrMessage[iMessage]->ID,
					ptrMessage[iMessage]->isExtend,
					ptrMessage[iMessage]->name,
					ptrMessage[iMessage]->size,
					ptrMessage[iMessage]->transmitter);
			for (i=0;i<((ptrMessage[iMessage]->prSignal[iSignal])->nReceiverNode-1);i++)
			{
				fprintf(csv,"%s ",(ptrMessage[iMessage]->prSignal[iSignal])->receiverNode[i]);
			}
			fprintf(csv,"%s,",ptrMessage[iMessage]->prSignal[iSignal]->receiverNode[i]);

			fprintf(csv,"%s,%s,%g,%g,%g,%g,",
				(ptrMessage[iMessage]->prSignal[iSignal])->name,
				(ptrMessage[iMessage]->prSignal[iSignal])->unit,
				(ptrMessage[iMessage]->prSignal[iSignal])->minimum,
				(ptrMessage[iMessage]->prSignal[iSignal])->maximum,
				(ptrMessage[iMessage]->prSignal[iSignal])->factor,
				(ptrMessage[iMessage]->prSignal[iSignal])->offset);

			fprintf(csv,"\"");
			for (i=0;i<((ptrMessage[iMessage]->prSignal[iSignal]->sValDecritp).nDescriptionElement);i++)
			{
				fprintf(csv,"%g %s ",(ptrMessage[iMessage]->prSignal[iSignal]->sValDecritp.value[i]), (ptrMessage[iMessage]->prSignal[iSignal]->sValDecritp).description[i]);
			//	if(i<((ptrMessage[iMessage]->prSignal[iSignal]->sValDecritp).nDescriptionElement-1))
			//	{
			//		fprintf(csv,"\n");
			//	}
			}
			fprintf(csv,"\"");


			fprintf(csv,",%u,%u,%d,%c\n",
				ptrMessage[iMessage]->prSignal[iSignal]->start_bit,
				ptrMessage[iMessage]->prSignal[iSignal]->signal_size,
				ptrMessage[iMessage]->prSignal[iSignal]->byte_order,
				ptrMessage[iMessage]->prSignal[iSignal]->value_type);
		}
	}
	fclose(csv);
	csv=NULL;
	return 1;
}
int freeMessage(Message *ptrMessage[], int nMaxDimension)
{
	int i,j;

	for(i=0;i<nMaxDimension;i++)
	{
		for(j=0;j<ptrMessage[i]->nSignal;j++)
		{
			free(ptrMessage[i]->prSignal[j]);
			ptrMessage[i]->prSignal[j]	= NULL;
		}

		free(ptrMessage[i]);
		ptrMessage[i]	= NULL;
	}

	return 1;
}
int freeComment()
{
	return 1;
}
int freeMem(char* dbc_file)
{
	free(dbc_file);	dbc_file= NULL;
	return 1;
}