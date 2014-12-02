#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CSV_creater.h"
static int calculate_LSB_bit(int start_bit,int byte_order,int signal_size);
static int calculate_LSB_intel(int start_bit,int signal_size);
int main()
{
	FILE *csv;
	FILE *dbc;
	char* dbc_file;
	string string_temp,FileName,FileName1,FileName2;
	char char_temp;
	int k,i,j,endPositon;
	int iMessage,iSignal,iChar;
	uint uintTemp;
	int nMessage,nChar;
	Message *prMessage;
	int extended_flag;
	//****************************************************
	// 打开要读和要写的文件
	printf("****************************************************\n");
	printf("Version:\t\t20140320\n");
	printf("Author:\t\t\tYu Wang, Fei Han\n");
	printf("Contact Information:\twangyu_1988@126.com\n");
	printf("****************************************************\n");
	printf("请输入dbc文件名，不带后缀\n");
	scanf("%s",FileName);
	strcpy(FileName1,FileName);
	strcpy(FileName2,FileName);
	printf("是否是扩展帧？0-NO 1-YES\n");
	scanf("%d",&extended_flag);

	if((dbc=fopen(strcat(FileName1,".dbc"),"r"))==NULL)
	{
		printf("Can not open dbc file\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	};
	if((csv=fopen(strcat(FileName2,"_c.csv"),"w"))==NULL)
	{
		printf("Can not open csv file\n");
		printf("Error in %s(%d)", __FUNCTION__, __LINE__);
		system("pause");
		exit(0);
	};
	//****************************************************
	// 计算总的字符数,分配内存，将文档保存到内存中
	nChar=0;
	char_temp=fgetc(dbc);
	while (char_temp!=EOF)
	{
		nChar++;
		char_temp=fgetc(dbc);		
	}
	dbc_file=(char *)malloc(nChar*sizeof(char));
	rewind(dbc);
	iChar=0;
	while (iChar<nChar)
	{
		dbc_file[iChar]=fgetc(dbc);
		if (dbc_file[iChar]==-80)
		{
			dbc_file[iChar]=32;
		}
		iChar=iChar+1;		
	}
	//****************************************************
	// 计算总的message和signal,分配内存
	nMessage=0;
	for(i=0;i<(nChar-3);i++)
	{
		if ((dbc_file[i]=='B')&&(dbc_file[i+1]=='O')&&(dbc_file[i+2]=='_')&&(dbc_file[i+3]==' '))
		{
			nMessage=nMessage+1;
		}
	}
	prMessage=(Message *)malloc(nMessage*sizeof(Message));
	iMessage=0;
	for(i=0;i<(nChar-3);i++)
	{
		if ((dbc_file[i]=='B')&&(dbc_file[i+1]=='O')&&(dbc_file[i+2]=='_')&&(dbc_file[i+3]==' '))
		{
			prMessage[iMessage++].startPosition=i;
		}
	}
	iMessage=0;
	while (iMessage<nMessage)
	{
		prMessage[iMessage].nSignal=0;
		if (iMessage==(nMessage-1))
		{
			endPositon=nChar;
		} 
		else
		{
			endPositon=prMessage[iMessage+1].startPosition;
		}
		for (i=prMessage[iMessage].startPosition;i<endPositon;i++)
		{
			if ((dbc_file[i]=='S')&&(dbc_file[i+1]=='G')&&(dbc_file[i+2]=='_')&&(dbc_file[i+3]==' '))
			{
				prMessage[iMessage].nSignal=prMessage[iMessage].nSignal+1;
			}
		}
		iMessage=iMessage+1;
	}
	for (iMessage=0;iMessage<nMessage;iMessage++)
	{
		prMessage[iMessage].prSignal=(Signal *)malloc(prMessage[iMessage].nSignal*sizeof(Signal));
		if (prMessage[iMessage].prSignal==NULL)
		{
			printf("prMessage[iMessage].prSignal error!\n");
			printf("iMessage=%d\n",iMessage);
			printf("Error in %s(%d)", __FUNCTION__, __LINE__);
			system("pause");
			exit(0);
		}
		iSignal=0;
		if (iMessage==(nMessage-1))
		{
			endPositon=nChar;
		} 
		else
		{
			endPositon=prMessage[iMessage+1].startPosition;
		}
		for (i=prMessage[iMessage].startPosition;i<endPositon;i++)
		{
			if ((dbc_file[i]=='S')&&(dbc_file[i+1]=='G')&&(dbc_file[i+2]=='_')&&(dbc_file[i+3]==' '))
			{
				prMessage[iMessage].prSignal[iSignal++].startPosition=i;
			}
		}

	}

	for (iMessage=0;iMessage<nMessage;iMessage++)
	{
		//去掉开始的字符
		i=prMessage[iMessage].startPosition+4; 
		//读ID
		char_temp=dbc_file[i];
		k=0;
		while (char_temp!=' ')
		{
			string_temp[k++]=char_temp;
			char_temp=dbc_file[++i];
		}
		string_temp[k]='\0';

		if (extended_flag==0)//不是扩展帧
		{
			prMessage[iMessage].ID=atoi(string_temp); 
		} 
		else
		{
			unsigned long x;
			x=strtoul(string_temp,NULL,10); 	//惨痛的教训！atol返回值最大是0x7FFFFFFF。 Yu Wang 20140320		
			prMessage[iMessage].ID=x & (0x7FFFFFFF);
		}
		//读name
		char_temp=dbc_file[++i];
		k=0;
		while(char_temp!=':')
		{
			string_temp[k++]=char_temp;
			char_temp=dbc_file[++i];
		}
		string_temp[k]='\0';
		strcpy(prMessage[iMessage].name,string_temp);

		//读入数值
		char_temp=dbc_file[++i];
		char_temp=dbc_file[++i];
		prMessage[iMessage].size=(int)char_temp-48;
		
		//读发送节点
		char_temp=dbc_file[++i];
		char_temp=dbc_file[++i];
		k=0;
		j=0;
		while(1)
		{
			if (char_temp!='\n')
			{
				string_temp[k++]=char_temp;
			} 
			else
			{
				string_temp[k]='\0';
				k=0;
				strcpy(prMessage[iMessage].transmitter,string_temp);
				break;
			}				
			char_temp=dbc_file[++i];
		}
		//处理该报文下的信号
		for (iSignal=0;iSignal<prMessage[iMessage].nSignal;iSignal++)
		{
			//去掉开始的字符
			i=prMessage[iMessage].prSignal[iSignal].startPosition+4; 
			//读name
			char_temp=dbc_file[i];
			k=0;
			while(char_temp!=' ')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			strcpy(prMessage[iMessage].prSignal[iSignal].name,string_temp);
			//读start_bit
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!='|')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			prMessage[iMessage].prSignal[iSignal].start_bit=(unsigned int)(atoi(string_temp));
			//读signal_size
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!='@')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			prMessage[iMessage].prSignal[iSignal].signal_size=(unsigned int)(atoi(string_temp));
			//读byte_order
			char_temp=dbc_file[++i];
			prMessage[iMessage].prSignal[iSignal].byte_order=(int)char_temp-48;
			//读value_type
			char_temp=dbc_file[++i];
			prMessage[iMessage].prSignal[iSignal].value_type=char_temp;
			//读factor
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!=',')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			prMessage[iMessage].prSignal[iSignal].factor=atof(string_temp);
			//读offset
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!=')')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			prMessage[iMessage].prSignal[iSignal].offset=atof(string_temp);
			//读minimum
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!='|')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			prMessage[iMessage].prSignal[iSignal].minimum=atof(string_temp);
			//读minimum
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!=']')
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
			}
			string_temp[k]='\0';
			prMessage[iMessage].prSignal[iSignal].maximum=atof(string_temp);
			//处理MOTORORA startbit
			uintTemp=(uint)(calculate_LSB_bit(
				prMessage[iMessage].prSignal[iSignal].start_bit,
				prMessage[iMessage].prSignal[iSignal].byte_order,
				prMessage[iMessage].prSignal[iSignal].signal_size
				));
			prMessage[iMessage].prSignal[iSignal].start_bit=uintTemp;
			//读单位
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			char_temp=dbc_file[++i];
			k=0;
			while(char_temp!='\"')//是不是“
			{
				string_temp[k++]=char_temp;
				char_temp=dbc_file[++i];
				
			}
			string_temp[k]='\0';
			strcpy(prMessage[iMessage].prSignal[iSignal].unit,string_temp);
			//读接收结点
			do 
			{
				char_temp=dbc_file[++i];
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
					strcpy(prMessage[iMessage].prSignal[iSignal].receiverNode[j++],string_temp);
				}else if (char_temp=='\n')
				{
					string_temp[k]='\0';
					k=0;
					strcpy(prMessage[iMessage].prSignal[iSignal].receiverNode[j++],string_temp);
					prMessage[iMessage].prSignal[iSignal].nReceiverNode=j;
					break;
				}				
				char_temp=dbc_file[++i];
			}		
		}		
	}
	//输出到csv中
	fprintf(csv,"period ms,Message ID,Message Name,Message size,transmitter,receiver,Signal Name,Signal Description,Units,Min,Max,factor,offset,value description,start_bit,Signal size,byte_order,value_type\n");
	for (iMessage=0;iMessage<nMessage;iMessage++)
	{
		for (iSignal=0;iSignal<prMessage[iMessage].nSignal;iSignal++)
		{
			fprintf(csv,"50,%u,%s,%d,%s,",
					prMessage[iMessage].ID,
					prMessage[iMessage].name,
					prMessage[iMessage].size,
					prMessage[iMessage].transmitter);
			for (i=0;i<(prMessage[iMessage].prSignal[iSignal].nReceiverNode-1);i++)
			{
				fprintf(csv,"%s ",prMessage[iMessage].prSignal[iSignal].receiverNode[i]);
			}
			fprintf(csv,"%s,",prMessage[iMessage].prSignal[iSignal].receiverNode[i]);
			fprintf(csv,"%s,,%s,%g,%g,%g,%g,,%u,%u,%d,%c\n",
				prMessage[iMessage].prSignal[iSignal].name,
				prMessage[iMessage].prSignal[iSignal].unit,
				prMessage[iMessage].prSignal[iSignal].minimum,
				prMessage[iMessage].prSignal[iSignal].maximum,
				prMessage[iMessage].prSignal[iSignal].factor,
				prMessage[iMessage].prSignal[iSignal].offset,
				prMessage[iMessage].prSignal[iSignal].start_bit,
				prMessage[iMessage].prSignal[iSignal].signal_size,
				prMessage[iMessage].prSignal[iSignal].byte_order,
				prMessage[iMessage].prSignal[iSignal].value_type);
		}
	}
	//****************************************************
	// 释放内存，关文件
	for (i=0;i<nMessage;i++)
	{
		free(prMessage[i].prSignal);
	}
	free(prMessage);
	fclose(csv);
	fclose(dbc);
	printf("转换完成\n");
	return 1;

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
//hanfei changed: 2014-01-02
static int coordinateChange(int org_M, int org_N, int length);

static int calculate_LSB_intel(int MSB,int length)
{
	int mMSB,nMSB,LSB;
	mMSB=MSB/8;
	nMSB=MSB%8;

	LSB	= coordinateChange(mMSB, nMSB, length);

	return LSB;
}

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