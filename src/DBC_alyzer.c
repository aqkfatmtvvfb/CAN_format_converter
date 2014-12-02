/********************************************************************
created:	2013/02/24
created:	24:2:2013   22:56
rev:	    2014/05/20  00:25
filename: 	c:\Users\WangYu\Desktop\CAN\DBC_alyzer\DBC_alyzer\DBC_alyzer.c
file path:	c:\Users\WangYu\Desktop\CAN\DBC_alyzer\DBC_alyzer
file base:	DBC_alyzer
file ext:	c
author:		WangYu(wangyu_1988@126.com)
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "DBC_alyzer.h"

static int calculate_start_bit(int LSB,int byte_order,int signal_size);
static int calculate_MSB_motorola(int LSB,int signal_size);

static int coordinateChange(int org_M, int org_N, int length);

static int matchSignal(Record *pRecord, Message *pMessage, int nMaxMessage);
static int autoConifgure(Message *pMessage, int nMaxMessage);

int main()
{
	FILE *csv;
	FILE *dbc;
	string string_temp,FileName,FileName1,FileName2;
	char char_temp;
	int row,column,k,i,j;
	int iMessage,iSignal,iRecord;
	uint messageIDTemp;
	int nMessage,nRecord;
	Record *record;
	Message *message;
	ValueDescription *valueDescription;
	C_identifier nodeName[20],nodeNameTemp;
	int nNode,iNode;
	int iReceiverNode,iDescription,nDescription,iDescriptionElement;
	int tempQuotationCount;
	//****************************************************
	// 打开要读和要写的文件
	printf("****************************************************\n");
	printf("Version:\t\t20140520\n");
	printf("Author:\t\t\tYu Wang, Fei Han\n");
	printf("Contact Information:\twangyu_1988@126.com\n");
	printf("****************************************************\n");
	printf("请输入csv文件名，不带后缀\n");
	scanf("%s",FileName);
	strcpy(FileName1,FileName);
	strcpy(FileName2,FileName);
	if((csv=fopen(strcat(FileName1,".csv"),"r"))==NULL)
	{
		printf("Can not open csv file\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	};
	if((dbc=fopen(strcat(FileName2,"_c.dbc"),"w"))==NULL)
	{
		printf("Can not open dbc file\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	};
	//****************************************************
	// 计算总的record数
	do
	{
		char_temp=fgetc(csv);
	}
	while(char_temp!='\n'); //去掉第一行的字符

	nRecord=0;
	char_temp=fgetc(csv);

	tempQuotationCount = 0;
	while (char_temp!=EOF)
	{
		if(char_temp == '\"')
		{
			tempQuotationCount++;
		}

		if (char_temp=='\n')
		{
			if(tempQuotationCount%2==0)
			nRecord++;
		}
		char_temp=fgetc(csv);
	}

	//****************************************************
	// 动态分配record
	record=(Record *)malloc(nRecord*sizeof(Record));
	if (record==NULL)
	{	
		printf("record_pr_error!\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	}
	//****************************************************
	// 开始将数据读入到record中
	rewind(csv);
	do
	{
		char_temp=fgetc(csv);
	}
	while(char_temp!='\n'); //去掉第一行的字符

	row=0;
	column=0;
	k=0;
	tempQuotationCount = 0;
	char_temp=fgetc(csv);
	while (char_temp!=EOF)
	{
		if(char_temp == '\"')
			tempQuotationCount++;

		if(char_temp == '\n' && tempQuotationCount%2==1)
			char_temp = ' ';

		if ((','!=char_temp) && ('\n'!=char_temp) && ('\"'!=char_temp))
		{
			string_temp[k]=char_temp;
			k=k+1;
			if(k == 149)
				printf("k = %d\n",k);
		}
		else if(char_temp==',')
		{
			string_temp[k]='\0';
			k=0;
			if (2==row)
			{
				row=2;
			}
			switch(column)
			{
			case 0:	record[row].period_ms	= atoi(string_temp);break;
			case 1:	record[row].rawMessageID	= strtoul(string_temp,NULL,10) ; break;
			case 2: record[row].isExtend=atoi(string_temp);
					record[row].messageID   = record[row].rawMessageID	| (0x80000000*record[row].isExtend);
					break;
			case 3:	strcpy(record[row].messageName,string_temp);break;
			case 4:	record[row].messageSize	=atoi(string_temp);break;
			case 5:	strcpy(record[row].transmitter,string_temp);break;	
			case 6:	strcpy(record[row].receiver,string_temp);break;
			case 7:	strcpy(record[row].signalName,string_temp);break;
			case 8:	strcpy(record[row].units,string_temp);break;
			case 9:
				if(strlen(string_temp) == 0)
					record[row].Min	= NULL_VALUE;
				else
					record[row].Min	= atof(string_temp);
				break;
			case 10:
				if(strlen(string_temp) == 0)
					record[row].Max	= NULL_VALUE;
				else
					record[row].Max	= atof(string_temp);
				break;
			case 11:
				if(strlen(string_temp) == 0)
					record[row].factor	= NULL_VALUE;
				else
					record[row].factor	= atof(string_temp);
				break;
			case 12:
				if(strlen(string_temp) == 0)
					record[row].offset	= NULL_VALUE;
				else
					record[row].offset	= atof(string_temp);
				break;
			case 13:memcpy(record[row].valueDescription,string_temp, sizeof(char)*320);break;
			case 14:record[row].startBit	=atoi(string_temp);break;
			case 15:
				if(strlen(string_temp) == 0)
					record[row].signalSize	= NULL_VALUE;
				else
					record[row].signalSize	= atoi(string_temp);
				break;
			case 16:record[row].byte_order	= atoi(string_temp);break;
				// case 17:record[row].value_type	=string_temp[0];break;
			default: printf("Error in %s(%d)", __FUNCTION__, __LINE__);
			}
			column=column+1;
		}
		else if(char_temp=='\n')
		{
			string_temp[k]='\0';
			k=0;

			if (17==column)
			{
				record[row].value_type	=string_temp[0];
			}
			else
			{
				printf("Error in %s(%d)", __FUNCTION__, __LINE__);
				system("pause");
				exit(0);
			}
			column=0;
			row=row+1;
		}
		else if(char_temp=='\"')
		{
			string_temp[k] = ' ';
		}
		else
		{
			printf("Error in %s(%d)", __FUNCTION__, __LINE__);
			system("pause");
			exit(0);
		}
		char_temp=fgetc(csv);
	}
	//****************************************************
	// 统计节点数量，保存节点名

	// 将接收节点分开
	row=0;
	j=0;
	i=0;
	k=0;
	while (row<nRecord)
	{
		char_temp=record[row].receiver[k];
		if ((char_temp!=' ') && (char_temp!='\0'))
		{
			record[row].receiverNode[j][i]=char_temp;
			i=i+1;
			k=k+1;
		}
		else if(char_temp==' ')
		{
			record[row].receiverNode[j][i]='\0';
			i=0;
			j=j+1;
			k=k+1;
		}		
		else if(char_temp=='\0')
		{
			record[row].receiverNode[j][i]='\0';
			i=0;
			record[row].nReceiverNode=j+1;
			j=j+1;
			while(j<10)
			{	
				record[row].receiverNode[j][0]='\0';
				j=j+1;
			}
			j=0;
			i=0;
			k=0;
			row=row+1;
		} 
		else
		{
			printf("Error in %s(%d)", __FUNCTION__, __LINE__);
			system("pause");
			exit(0);
		}
	}
	//计数
	nNode=1;
	strcpy(nodeName[0],record[0].transmitter);
	iNode=0;
	row=0;
	j=0;
	i=0;
	while (row<nRecord)
	{
		strcpy(nodeNameTemp,record[row].transmitter);

		for(iNode=0;iNode<nNode;iNode++)
		{
			if (strcmp(nodeNameTemp,nodeName[iNode])==0)
			{
				break;
			}
			else
			{
				continue;
			}

		}
		if (iNode==nNode)//与当前node序列均不相同
		{
			nNode=nNode+1;
			strcpy(nodeName[iNode],nodeNameTemp);
		}
		iReceiverNode=0;
		while (iReceiverNode<record[row].nReceiverNode)
		{
			strcpy(nodeNameTemp,record[row].receiverNode[iReceiverNode]);
			for(iNode=0;iNode<nNode;iNode++)
			{
				if (strcmp(nodeNameTemp,nodeName[iNode])==0)
				{
					break;
				}
				else
				{
					continue;
				}

			}
			if (iNode==nNode)//与当前node序列均不相同
			{
				nNode=nNode+1;
				strcpy(nodeName[iNode],nodeNameTemp);
			}
			iReceiverNode=iReceiverNode+1;
		}
		row=row+1;
	}
	//****************************************************
	// 计算总的msg数和signal数,并分配内存
	nMessage=1;
	row=0;
	messageIDTemp=record[row].messageID;
	row=row+1;
	while(row<nRecord)
	{
		if(record[row].messageID!=messageIDTemp)
		{
			nMessage++;
			messageIDTemp=record[row].messageID;
		}
		row=row+1;
	}
	message=(Message *)malloc(nMessage*sizeof(Message));
	if (message==NULL)
	{
		printf("message_pr_error!\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	}
	for (i=0;i<nMessage;i++)
	{
		message[i].nSignal=1;
	}

	row=0;
	i=0;
	messageIDTemp=record[row].messageID;
	row=row+1;
	while (row<nRecord)
	{	
		if(record[row].messageID==messageIDTemp)
		{
			message[i].nSignal=message[i].nSignal+1;
		}
		else
		{
			i=i+1;
			messageIDTemp=record[row].messageID;
		}
		row=row+1;
	}
	for (i=0;i<nMessage;i++)
	{
		message[i].prSignal=(Signal *)malloc(message[i].nSignal*sizeof(Signal));
		if (message[i].prSignal==NULL)
		{
			printf("message[i].prSignal_error!\n");
			printf("Error in %s(%d)", __FUNCTION__, __LINE__);
			system("pause");
			exit(0);
		}

	}

	//****************************************************
	// 将record存到各自的数据结构中
	iRecord=0;
	for (iMessage=0;iMessage<nMessage;iMessage++)
	{
		message[iMessage].ID=record[iRecord].messageID;
		strcpy(message[iMessage].name,record[iRecord].messageName);
		message[iMessage].size=record[iRecord].messageSize;
		strcpy(message[iMessage].transmitter,record[iRecord].transmitter);
		for (iSignal=0;iSignal<message[iMessage].nSignal;iSignal++)
		{
			strcpy(message[iMessage].prSignal[iSignal].name,record[iRecord].signalName);
			//message[iMessage].prSignal[iSignal].start_bit=record[iRecord].startBit;
			message[iMessage].prSignal[iSignal].start_bit=calculate_start_bit(record[iRecord].startBit,record[iRecord].byte_order,record[iRecord].signalSize);
			message[iMessage].prSignal[iSignal].signal_size=record[iRecord].signalSize;
			message[iMessage].prSignal[iSignal].byte_order=record[iRecord].byte_order;
			message[iMessage].prSignal[iSignal].value_type=record[iRecord].value_type;
			message[iMessage].prSignal[iSignal].factor=record[iRecord].factor;
			message[iMessage].prSignal[iSignal].offset=record[iRecord].offset;
			message[iMessage].prSignal[iSignal].minimum=record[iRecord].Min;
			message[iMessage].prSignal[iSignal].maximum=record[iRecord].Max;
			strcpy(message[iMessage].prSignal[iSignal].unit,record[iRecord].units);
			message[iMessage].prSignal[iSignal].nReceiverNode=record[iRecord].nReceiverNode;
			for(i=0;i<message[iMessage].prSignal[iSignal].nReceiverNode;i++)
			{
				strcpy(message[iMessage].prSignal[iSignal].receiverNode[i],record[iRecord].receiverNode[i]);
			}
			iRecord=iRecord+1;
		}
	}

	//autoConifgure(message, nMessage);

	//解析value_description
	for (iRecord=0;iRecord<nRecord;iRecord++)
	{
		int errorFlag = 0;
		int nValDesLength	= strlen(record[iRecord].valueDescription);

		if(nValDesLength < 2)
			continue;
		else
		{
			errorFlag	= matchSignal(&(record[iRecord]), message, nMessage,nValDesLength);
			if(errorFlag == 0)
				return 0;
		}
	}

	//****************************************************
	// 写入dbc
	fprintf(dbc,"VERSION \"\"\n");
	fprintf(dbc,"\n\n");
	fprintf(dbc,"NS_ : \n");
	fprintf(dbc,"\tNS_DESC_\n");
	fprintf(dbc,"\tCM_\n");
	fprintf(dbc,"\tBA_DEF_\n");
	fprintf(dbc,"\tBA_\n");
	fprintf(dbc,"\tVAL_\n");
	fprintf(dbc,"\tCAT_DEF_\n");
	fprintf(dbc,"\tCAT_\n");
	fprintf(dbc,"\tFILTER\n");
	fprintf(dbc,"\tBA_DEF_DEF_\n");
	fprintf(dbc,"\tEV_DATA_\n");
	fprintf(dbc,"\tENVVAR_DATA_\n");
	fprintf(dbc,"\tSGTYPE_\n");
	fprintf(dbc,"\tSGTYPE_VAL_\n");
	fprintf(dbc,"\tBA_DEF_SGTYPE_\n");
	fprintf(dbc,"\tBA_SGTYPE_\n");
	fprintf(dbc,"\tSIG_TYPE_REF_\n");
	fprintf(dbc,"\tVAL_TABLE_\n");
	fprintf(dbc,"\tSIG_GROUP_\n");
	fprintf(dbc,"\tSIG_VALTYPE_\n");
	fprintf(dbc,"\tSIGTYPE_VALTYPE_\n");
	fprintf(dbc,"\tBO_TX_BU_\n");
	fprintf(dbc,"\tBA_DEF_REL_\n");
	fprintf(dbc,"\tBA_REL_\n");
	fprintf(dbc,"\tBA_DEF_DEF_REL_\n");
	fprintf(dbc,"\tBU_SG_REL_\n");
	fprintf(dbc,"\tBU_EV_REL_\n");
	fprintf(dbc,"\tBU_BO_REL_\n");
	fprintf(dbc,"\tSG_MUL_VAL_\n");
	fprintf(dbc,"\n");
	fprintf(dbc,"BS_:\n");
	fprintf(dbc,"\n");
	fprintf(dbc,"BU_:");
	for(iNode=0;iNode<nNode;iNode++)
	{
		fprintf(dbc," %s",nodeName[iNode]);
	}
	fprintf(dbc,"\n\n\n");
	for(iMessage=0;iMessage<nMessage;iMessage++)
	{
		fprintf(
			dbc,"BO_ %u %s: %d %s\n",
			message[iMessage].ID,
			message[iMessage].name,
			message[iMessage].size,
			message[iMessage].transmitter
			);

		for (iSignal=0;iSignal<message[iMessage].nSignal;iSignal++)
		{ 
			fprintf(
				dbc," SG_ %s : %d|%d@%d%c (%g,%g) [%g|%g] \"%s\"  ",
				message[iMessage].prSignal[iSignal].name,
				message[iMessage].prSignal[iSignal].start_bit,
				message[iMessage].prSignal[iSignal].signal_size,
				message[iMessage].prSignal[iSignal].byte_order,
				message[iMessage].prSignal[iSignal].value_type,
				message[iMessage].prSignal[iSignal].factor,
				message[iMessage].prSignal[iSignal].offset,
				message[iMessage].prSignal[iSignal].minimum,
				message[iMessage].prSignal[iSignal].maximum,
				message[iMessage].prSignal[iSignal].unit
				);
			for(i=0;i<message[iMessage].prSignal[iSignal].nReceiverNode;i++)
			{
				fprintf(dbc,"%s",message[iMessage].prSignal[iSignal].receiverNode[i]);
				if(i!=(message[iMessage].prSignal[iSignal].nReceiverNode-1))
				{
					fprintf(dbc,",");
				}
				else
				{
					fprintf(dbc,"\n");
				}
			}
		}
		fprintf(dbc,"\n");
	}

	//文本输出描述列表
	fprintf(dbc,"\n\n");
	for (iMessage=0; iMessage<nMessage; iMessage++)
	{
		for (iSignal=0;iSignal<message[iMessage].nSignal;iSignal++)
		{
			if(message[iMessage].prSignal[iSignal].sValDecritp.nDescriptionElement<=0)
				continue;
			else
			{
				fprintf(dbc, "VAL_ %u %s ", message[iMessage].prSignal[iSignal].sValDecritp.message_ID, message[iMessage].prSignal[iSignal].sValDecritp.signal_name);

				for(iDescriptionElement=0; iDescriptionElement<message[iMessage].prSignal[iSignal].sValDecritp.nDescriptionElement; iDescriptionElement++)
				{
					fprintf(dbc, "%g \"%s\" ",message[iMessage].prSignal[iSignal].sValDecritp.value[iDescriptionElement],message[iMessage].prSignal[iSignal].sValDecritp.description[iDescriptionElement]);
				}
				fprintf(dbc,";\n");
			}
		}
	}

	fprintf(dbc,"\n");
	//****************************************************
	// 释放内存，关文件
	//free(valueDescription);
	fclose(csv);
	fclose(dbc);
	for (i=0;i<nMessage;i++)
	{
		free(message[i].prSignal);
	}
	free(message);
	free(record);


	printf("转换完成\n");
	return 1;

}
static int calculate_start_bit(int LSB,int byte_order,int signal_size)
{
	int start_bit;
	if (1==byte_order)  // intel format
	{
		start_bit=LSB;
	} 
	else
	{
		start_bit=calculate_MSB_motorola(LSB,signal_size); 
	}
	return start_bit;
}
//static int calculate_MSB_motorola(int LSB,int length)
//{
//	int mLSB,nLSB,mMSB,nMSB,MSB;
//	mLSB=LSB/8;
//	nLSB=LSB%8;
//	nMSB=(length+nLSB-1)%8;
//	if ((8-nLSB)>=length)
//	{
//		mMSB=mLSB;
//	} 
//	else
//	{
//		mMSB=mLSB-((length-(8-nLSB))/8+1);
//	}
//	MSB=8*mMSB+nMSB;
//	return MSB;
//}


//hanfei changed: 2014-01-02
static int calculate_MSB_motorola(int LSB,int length)
{
	int mLSB,nLSB,MSB;
	mLSB=LSB/8;
	nLSB=LSB%8;

	MSB	= coordinateChange(mLSB, nLSB, length);

	return MSB;
}

static int coordinateChange(int org_M, int org_N, int length)
{
	int new_M, new_N;
	int result_M, result_N;
	int tempNum;

	new_M	= org_M;
	new_N	= 7 - org_N;

	tempNum	= new_M * 8 + new_N;
	tempNum	= tempNum - length + 1;

	result_N	= tempNum%8;
	result_M	= tempNum/8;

	result_N	= 7 - result_N;

	return 8*result_M+result_N;
}


static int extractDescription(char srcValChar[], ValueDescription *dstValDes,int nValDesLength);
//*---------------------
// 将描述注释与信号对应起来
//*---------------------
static int matchSignal(Record *pRecord, Message *pMessage, int nMaxMessage,int nValDesLength)
{
	int i;
	int nMessageIndex, nSignalIndex;
	//ValueDescription tempValDes;

	for(i=0;i<nMaxMessage;i++)
	{
		if(pMessage[i].ID == pRecord->messageID)
		{
			nMessageIndex	= i;
			break;
		}
	}
	if(i==nMaxMessage)
	{
		printf("Doesn't match Message Description---MessageID:%d\n",pRecord->messageID);
		return 0;
	}

	for(i=0;i<pMessage[nMessageIndex].nSignal;i++)
	{
		if(strcmp(pMessage[nMessageIndex].prSignal[i].name, pRecord->signalName) == 0)
		{
			nSignalIndex	= i;
			break;
		}
	}
	if(i==pMessage[nMessageIndex].nSignal)
	{
		printf("Doesn't match Signal Description---Signal Name:%s\n",pRecord->signalName);
		return 0;
	}

	pMessage[nMessageIndex].prSignal[nSignalIndex].sValDecritp.message_ID	= pRecord->messageID;
	strcpy(pMessage[nMessageIndex].prSignal[nSignalIndex].sValDecritp.signal_name, pRecord->signalName);

	if(extractDescription(pRecord->valueDescription, &(pMessage[nMessageIndex].prSignal[nSignalIndex].sValDecritp),nValDesLength) == 0)
		return 0;
	

	//memcpy(&(pMessage[nMessageIndex].prSignal[nSignalIndex].sValDecritp), &tempValDes, sizeof(ValueDescription));

	return 1;
}

//*--------------------
// 将描述注释赋值
//*--------------------
static int extractDescription(char srcValChar[], ValueDescription *dstValDes,int nValDesLength)
{
	int i=0, k=0, j;
	int valueType=0;
	int nValueDes=0;
	char tempChar=0;
	string string_temp;
	int valueCount=0;

	while(i<nValDesLength)//&&(srcValChar[i] != 13)) 
		//第二个条件添加于20140529by王御，在转换mcu.csv的过程中会出现一个value_des为乱码的问题，原因是读入的字符ascii码为13
	{
		tempChar	= srcValChar[i];
		
		k=0;
		valueType =0;
		while(tempChar != ' ')
		{
			string_temp[k++]	= tempChar;
			tempChar			= srcValChar[++i];
			if( tempChar == '\0')
				break;
		}
		string_temp[k] = '\0';

		

		if(strlen(string_temp)<1)
		{
			i++;
			continue;
		}
		else
		{
			valueCount++;
		}
		//for(j=0;j<strlen(string_temp);j++)
		//{
		//	tempChar = string_temp[j];
			//if((tempChar>=65 && tempChar<=90) || (tempChar>=97 && tempChar<=122))
			if(valueCount%2 == 0)
			{
				valueType = 1;
				//break;
			}
		//}

		if(valueType == 0)
		{
			dstValDes->value[nValueDes/2]	= atof(string_temp); 

			nValueDes++;
		}
		else
		{
			strcpy(dstValDes->description[nValueDes/2], string_temp);

			nValueDes++;
		}

		i++;
	}
	
	if(nValueDes%2 == 1)
	{
		printf("Read Value Description Error\t");
		printf("ID:%u\tname=:%s\tn=%d\n",dstValDes->message_ID,dstValDes->signal_name,dstValDes->nDescriptionElement);
		nValueDes = nValueDes - 1;
		//return 0;
	}
	//nValueDes = nValueDes/2;

	dstValDes->nDescriptionElement	= nValueDes/2;

	return 1;
}


static int defaultMin(Signal *pSignal);
static int defaultMax(Signal *pSignal);
static int defaultFactor(Signal *pSignal);
static int defaultSigSize(Signal *pSignal);
//*-----------------------------------------
// 自动设置Signal.Min Max factor SignalSize
//*-----------------------------------------
static int autoConifgure(Message *pMessage, int nMaxMessage)
{
	int i,j;
	enum defaultType {Min, Max, Factor, SigSize, Offset, NoDefault} eDefaultValue;

	for(i=0;i<nMaxMessage;i++)
	{
		for(j=0;j<pMessage[i].nSignal;j++)
		{
			/*------------------*/
			/*	offset默认处理	*/
			/*------------------*/
			if(pMessage[i].prSignal[j].offset == NULL_VALUE)
			{
				if(pMessage[i].prSignal[j].minimum < 0.0)
					pMessage[i].prSignal[j].offset	= pMessage[i].prSignal[j].minimum;
				else
					pMessage[i].prSignal[j].offset	= 0.0;
			}


			if(pMessage[i].prSignal[j].minimum == NULL_VALUE)
				eDefaultValue	= Min;
			else if(pMessage[i].prSignal[j].maximum == NULL_VALUE)
				eDefaultValue	= Max;
			else if(pMessage[i].prSignal[j].factor == NULL_VALUE)
				eDefaultValue	= Factor;
			else if(pMessage[i].prSignal[j].signal_size == NULL_VALUE)
				eDefaultValue	= SigSize;
			else
				eDefaultValue	= NoDefault;


			switch(eDefaultValue)
			{
			case Min:
				defaultMin(&(pMessage[i].prSignal[j]));break;
			case Max:
				defaultMax(&(pMessage[i].prSignal[j]));break;
			case Factor:
				defaultFactor(&(pMessage[i].prSignal[j]));break;
			case SigSize:
				defaultSigSize(&(pMessage[i].prSignal[j]));break;
			default:
				break;
			}
		}

	}
}


//*-----------
// 信号Min缺省
//*-----------
static int defaultMin(Signal *pSignal)
{
	int SignalSize	= pSignal->signal_size;
	double Factor	= pSignal->factor;
	double MaxValue	= pSignal->maximum;
	double MinValue, MaxSize;

	MaxSize		= pow((double)2, SignalSize);

	MinValue	= MaxValue - MaxSize*Factor;

	/*------------------------------*/
	/*		是不是应该做些处理			*/
	/*	使最小值的精度与MaxValue一致？	*/
	/*------------------------------*/
	

	/*------------------------------*/

	pSignal->minimum	= MinValue;

	return 1;
}

//*-----------
// 信号Max缺省
//*-----------
static int defaultMax(Signal *pSignal)
{
	int SignalSize	= pSignal->signal_size;
	double Factor	= pSignal->factor;
	double MinValue	= pSignal->minimum;
	double MaxValue, MaxSize;

	MaxSize		= pow((double)2, SignalSize);

	MaxValue	= MinValue + MaxSize*Factor;

	/*------------------------------*/
	/*		是不是应该做些处理			*/
	/*	使最大值的精度与MaxValue一致？	*/
	/*------------------------------*/
	

	/*------------------------------*/

	pSignal->maximum	= MaxValue;

	return 1;
}

//*-------------
// 信号Factor缺省
//*-------------
static int defaultFactor(Signal *pSignal)
{
	int SignalSize	= pSignal->signal_size;
	double MinValue	= pSignal->minimum;
	double MaxValue	= pSignal->maximum;
	double Factor, MaxSize;

	MaxSize		= pow((double)2, SignalSize);

	Factor		= (MaxValue-MinValue)/MaxSize;

	/*------------------------------*/
	/*		是不是应该做些处理			*/
	/*		确定factor的精度			*/
	/*------------------------------*/
	

	/*------------------------------*/

	pSignal->factor	= Factor;

	return 1;
}

//*-----------------
// 信号signalSize缺省
//*-----------------
static int defaultSigSize(Signal *pSignal)
{
	double MinValue	= pSignal->minimum;
	double MaxValue	= pSignal->maximum;
	double Factor	= pSignal->factor;
	double Offset	= pSignal->offset;
	double MaxSize	= (MaxValue-MinValue)/Factor;

	int SignalSize, powerBit;

	double temp;

	if(MaxSize <= 1)
		pSignal->signal_size	= 1;
	else
	{
		for(powerBit=0;powerBit<10;powerBit++)
		{
			double temp1	= pow(2,pow(2,powerBit));
			double temp2	= pow(2,pow(2,powerBit+1));

			if(temp1<MaxSize && temp2>MaxSize)
				break;
		}

		pSignal->signal_size	= pow(2, (powerBit+1));
	}

	return 1;
}