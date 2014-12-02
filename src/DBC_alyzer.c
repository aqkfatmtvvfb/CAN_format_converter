/********************************************************************
created:	2013/02/24
created:	24:2:2013   22:56
filename: 	c:\Users\WangYu\Desktop\CAN\DBC_alyzer\DBC_alyzer\DBC_alyzer.c
file path:	c:\Users\WangYu\Desktop\CAN\DBC_alyzer\DBC_alyzer
file base:	DBC_alyzer
file ext:	c
author:		WangYu(wangyu_1988@126.com)
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DBC_alyzer.h"
static int calculate_start_bit(int LSB,int byte_order,int signal_size);
static int calculate_MSB_motorola(int LSB,int signal_size);

static int coordinateChange(int org_M, int org_N, int length);

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
	int extended_flag;
	//****************************************************
	// ��Ҫ����Ҫд���ļ�
	printf("****************************************************\n");
	printf("Version:\t\t20140320\n");
	printf("Author:\t\t\tYu Wang, Fei Han\n");
	printf("Contact Information:\twangyu_1988@126.com\n");
	printf("****************************************************\n");
	printf("������csv�ļ�����������׺\n");
	scanf("%s",FileName);
	strcpy(FileName1,FileName);
	strcpy(FileName2,FileName);
	printf("�Ƿ�����չ֡��0-NO 1-YES\n");
	scanf("%d",&extended_flag);

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
	// �����ܵ�record��
	do
	{
		char_temp=fgetc(csv);
	}
	while(char_temp!='\n'); //ȥ����һ�е��ַ�
	nRecord=0;
	char_temp=fgetc(csv);
	while (char_temp!=EOF)
	{
		if (char_temp=='\n')
		{
			nRecord++;
		}
		char_temp=fgetc(csv);
	}

	//****************************************************
	// ��̬����record
	record=(Record *)malloc(nRecord*sizeof(Record));
	if (record==NULL)
	{	
		printf("record_pr_error!\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	}
	//****************************************************
	// ��ʼ�����ݶ��뵽record��
	rewind(csv);
	do
	{
		char_temp=fgetc(csv);
	}
	while(char_temp!='\n'); //ȥ����һ�е��ַ�

	row=0;
	column=0;
	k=0;
	char_temp=fgetc(csv);
	while (char_temp!=EOF)
	{
		if ((','!=char_temp) && ('\n'!=char_temp))
		{
			string_temp[k]=char_temp;
			k=k+1;
		}
		else if(char_temp==',')
		{
			string_temp[k]='\0';
			k=0;
			switch(column)
			{
			case 0:	record[row].period_ms	=atoi(string_temp);break;
			case 1:	record[row].messageID	=atoi(string_temp) | (0x80000000*extended_flag);break;
			case 2:	strcpy(record[row].messageName,string_temp);	;break;
			case 3:	record[row].messageSize	=atoi(string_temp);break;
			case 4:	strcpy(record[row].transmitter,string_temp);break;	
			case 5:	strcpy(record[row].receiver,string_temp);break;
			case 6:	strcpy(record[row].signalName,string_temp);break;
			case 7:	strcpy(record[row].signalDescription,string_temp);break;
			case 8:	strcpy(record[row].units,string_temp);break;
			case 9:	record[row].Min	=atof(string_temp);break;
			case 10:record[row].Max	=atof(string_temp);break;
			case 11:record[row].factor	=atof(string_temp);break;
			case 12:record[row].offset	=atof(string_temp);break;
			case 13:strcpy(record[row].valueDescription,string_temp);break;
			case 14:record[row].startBit	=atoi(string_temp);break;
			case 15:record[row].signalSize	=atoi(string_temp);break;
			case 16:record[row].byte_order	=atoi(string_temp);break;
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
		else
		{
			printf("Error in %s(%d)", __FUNCTION__, __LINE__);
			system("pause");
			exit(0);
		}
		char_temp=fgetc(csv);
	}
	//****************************************************
	// ͳ�ƽڵ�����������ڵ���

	// �����սڵ�ֿ�
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
	//����
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
		if (iNode==nNode)//�뵱ǰnode���о�����ͬ
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
			if (iNode==nNode)//�뵱ǰnode���о�����ͬ
			{
				nNode=nNode+1;
				strcpy(nodeName[iNode],nodeNameTemp);
			}
			iReceiverNode=iReceiverNode+1;
		}
		row=row+1;
	}
	//****************************************************
	// �����ܵ�msg����signal��,�������ڴ�
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
	// ��record�浽���Ե����ݽṹ��
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
	//����value_description

	//ֵ����������
	row=0;
	nDescription=0;
	while (row<nRecord)
	{
		if (strcmp("\0",record[row].valueDescription)==0)//�յ�
		{
			row=row+1;
			continue;
		}
		else
		{
			nDescription=nDescription+1;
			row=row+1;
		}
	}
	valueDescription=(ValueDescription *)malloc(nDescription*sizeof(ValueDescription));
	if (valueDescription==NULL)
	{
		printf("valueDescription_error!\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	}
	iDescription=0;
	row=0;
	i=0;
	k=0;
	iDescriptionElement=0;
	while (iDescription<nDescription)
	{

		if (strcmp("\0",record[row].valueDescription)==0)//�յ�
		{
			row=row+1;
			continue;
		}
		else
		{
			char_temp=record[row].valueDescription[k];
			if ((char_temp!=' ') && (char_temp!='\0'))
			{
				string_temp[i]=char_temp;
				i=i+1;
				k=k+1;
			}
			else if(char_temp==' ')
			{

				string_temp[i]='\0';
				if(iDescriptionElement%2==0)
				{
					valueDescription[iDescription].value[iDescriptionElement/2]=atof(string_temp);
				}
				else
				{
					strcpy(valueDescription[iDescription].description[iDescriptionElement/2],string_temp);
				}
				i=0;
				k=k+1;
				iDescriptionElement=iDescriptionElement+1;
			}
			else if (char_temp=='\0')
			{
				string_temp[i]='\0';
				if(iDescriptionElement%2==1)
				{
					strcpy(valueDescription[iDescription].description[iDescriptionElement/2],string_temp);
					valueDescription[iDescription].nDescriptionElement=iDescriptionElement/2+1;
					valueDescription[iDescription].message_ID=record[row].messageID;
					strcpy(valueDescription[iDescription].signal_name,record[row].signalName);
					iDescription=iDescription+1;
				}
				else
				{
					printf("Error in Description, number of element in Row %d can not be exact divided by 2,signal name is %s \n",row,record[row].signalName);
					printf("Error in %s(%d)\n", __FUNCTION__, __LINE__);
					nDescription=nDescription-1;
				}
				i=0;
				k=0;
				row=row+1;
				iDescriptionElement=0;	
				
			} 			
			else
			{
				printf("Error in %s(%d)", __FUNCTION__, __LINE__);
				system("pause");
				exit(0);
			}
		}
	}

	//****************************************************
	// д��dbc
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
	for(iDescription=0;iDescription<nDescription;iDescription++)
	{
		fprintf(
			dbc,"VAL_ %u %s",
			valueDescription[iDescription].message_ID,
			valueDescription[iDescription].signal_name
			);
		for (iDescriptionElement=0;iDescriptionElement<valueDescription[iDescription].nDescriptionElement;iDescriptionElement++)
		{
			fprintf(
				dbc," %g \"%s\" ",
				valueDescription[iDescription].value[iDescriptionElement],
				valueDescription[iDescription].description[iDescriptionElement]
			);

		}
		fprintf(dbc,";\n");
	}
	//****************************************************
	// �ͷ��ڴ棬���ļ�
	free(valueDescription);
	for (i=0;i<nMessage;i++)
	{
		free(message[i].prSignal);
	}
	free(message);
	free(record);

	fclose(csv);
	fclose(dbc);
	printf("ת�����\n");
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