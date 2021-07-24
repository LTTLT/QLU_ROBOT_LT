#include "include.h"


static bool Uart2Busy = FALSE;
unsigned char CMD_0[8]={0x50,0x03,0x00,0x34,0x00,0x01,0xc8,0x45};
static bool fUartRxComplete = FALSE;
static uint8 xdata UartRxBuffer[260];

static bool UartBusy = FALSE;
uint8 distance1 = 0;
uint8 distance2 = 0;

uint16 ModbusCRC(uint8 *ptr,uint16 ucLen) ;

void InitUart2(void)
{
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0xC7;		//设定定时初值	//9600bps@12.000MHz
	T2H = 0xFE;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
	IE2 |= 0x01;                 //使能串口2中断
}



/***********************************************************
* 名    称： bool Uart2SendData(UINT8 dat)
* 功    能： 
* 入口参数：  
* 出口参数：
* 说    明： 					 
/**********************************************************/ 
void Uart2SendData(UINT8 dat)
{
    S2BUF = dat;             			//Send data to UART buffer
	Uart2Busy = TRUE;
	while (Uart2Busy);
}

void Uart2SendDataPacket(UINT8 dat[],uint8 len)
{
	uint8 i;
	for(i = 0;i < len;i++)
	{
		Uart2SendData(dat[i]);
	}
}



 unsigned char ucRxFinish1=0;
 static unsigned char ucCnt=0,ucLen=0;
 static unsigned char ucRxData[100];
/***********************************************************
* 名    称： void Uart4() interrupt 18 using 1
* 功    能： 
* 入口参数：  
* 出口参数：
* 说    明： 					 
/**********************************************************/ 
void Uart2() interrupt 8 using 1
{
	uint8 temp;
	 u16 C=0;
//	static uint8 messageLengthSum = 2;
	
	if (S2CON & S2RI)		//串口2收到数据请求中断
    {
        S2CON &= ~S2RI;     //Clear receive interrupt flag
		temp = S2BUF; 		//赋值串口2数据
				if(ucCnt==0)
					ucRxData[ucCnt++]=temp ;//ucRxData[0]
		  else	if((ucCnt==1)&(temp==0x03))//
					ucRxData[ucCnt++]=temp;//ucRxData[1]
			else if(ucCnt==2)
					{ucRxData[ucCnt++]=temp; ucLen=ucRxData[2];}//ucRxData[2]
			else if((ucCnt>2)&(ucCnt<=(ucLen+4)))
					ucRxData[ucCnt++]=temp;//ucRxData[3]
			if(ucCnt==(ucLen+5))
			{ 
				C=ModbusCRC(ucRxData,ucLen+3);
				
				if(C==((ucRxData[ucLen+3]<<8)|ucRxData[ucLen+4]))//
					{ucRxFinish1=1;	ucCnt=0;ucLen=0;}			
				else 
					{ucCnt=0;ucLen=0;}				
			}
    }

    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;     //Clear transmit interrupt flag
        Uart2Busy = FALSE;           //Clear transmit busy flag
    }
}


static bool UartRxOK(void)
{
	if(fUartRxComplete)
	{
		fUartRxComplete = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
void TaskBLEMsgHandle(void)
{
	uint8 length;
	//McuToPCSendData(CMD_ACTION_DOWNLOAD,data1,data2);
	if(UartRxOK())
	{
		LED = !LED;
		length = ((UartRxBuffer[1]<<8)|UartRxBuffer[2]);
 		}
}

uint16 ModbusCRC(uint8 *ptr,uint16 ucLen)//CRC校验
{
	uint8 i;
	uint16 j,crc=0xffff;
	int32 n;
	int32 k;
	
	i=i;	
	
	for( n=0; n<ucLen;n++)
	{
		crc=ptr[n]^crc;
		for( k=0;k<8;k++)
		if(crc&0x01)
		{
			crc=crc>>1;
			crc=crc^0xa001;
		}
		else
		{
			crc=crc>>1;	
		}		
	}

	j=crc>>8;
	j=j|(crc<<8);
	return j;

}
int16 getdata(void)
{
	uint8 dat[7];
 	unsigned int uiData;
	Uart2SendDataPacket(CMD_0,8);
	DelayMs(100);
	uiData=ucRxData[3]<<8|ucRxData[4];		
	ucRxFinish1=0;
	dat[0]=0x55;
	dat[1]=ucRxData[0];
	dat[2]=ucRxData[1];
	dat[3]=ucRxData[2];
	dat[4]=ucRxData[3];
	dat[5]=ucRxData[4];
	dat[6]=ucRxData[5];
	UART1SendDataPacket(dat,7);
    return uiData;	
}



