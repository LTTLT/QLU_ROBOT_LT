#include "include.h"





static bool fUartRxComplete = FALSE;
static uint8 xdata UartRxBuffer[260];
uint8 xdata Uart1RxBuffer[260];
unsigned char CMD_1[8]={0x50,0x03,0x00,0x34,0x00,0x01,0xc8,0x45};
static bool UartBusy = FALSE;

uint8 xdata frameIndexSumSum[256];
uint16 UART1ModbusCRC(uint8 *ptr,uint16 ucLen);//CRC校验

void InitUart1(void)
{

	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0xC7;		//设定定时初值//9600bps@12.000MHz
	TH1 = 0xFE;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
    ES = 1;			//使能串口中断
}

void Uart1SendData(BYTE dat)
{
    SBUF = dat;             			//Send data to UART buffer
	while (!TI);
}

void UART1SendDataPacket(uint8 dat[],uint8 count)
{
	uint8 i;
	for(i = 0; i < count; i++)
	{
		Uart1SendData(dat[i]);
	}

}

 unsigned char ucRxFinish=0;
 static unsigned char ucCnt=0,ucLen=0;
 static unsigned char ucRxData[100];
void Uart1() interrupt 4 using 1
{
	uint8 i;
	uint8 rxBuf;
	u16 C=0;
	static uint8 startCodeSum = 0;
	static bool fFrameStart = FALSE;
	static uint8 messageLength = 0;
	static uint8 messageLengthSum = 2;
	
    if(RI)
    {
        RI = 0;                 //清除标志位
        rxBuf = SBUF;		
//			if(ucCnt==0)
//			ucRxData[ucCnt++]=rxBuf ;//ucRxData[0]
//		  		else	if((ucCnt==1)&(rxBuf==0x03))//
//					ucRxData[ucCnt++]=rxBuf;//ucRxData[1]
//				else if(ucCnt==2)
//					{ucRxData[ucCnt++]=rxBuf; ucLen=ucRxData[2];}//ucRxData[2]
//				else if((ucCnt>2)&(ucCnt<=(ucLen+4)))
//					ucRxData[ucCnt++]=rxBuf;//ucRxData[3]		
//				if(ucCnt==(ucLen+5))
//			{ 
//				C=UART1ModbusCRC(ucRxData,ucLen+3);
//				
//				if(C==((ucRxData[ucLen+3]<<8)|ucRxData[ucLen+4]))//
//					{ucRxFinish=1;	ucCnt=0;ucLen=0;}			
//				else 
//					{ucCnt=0;ucLen=0;}				
//			}						 
		if(!fFrameStart)
		{
			if(rxBuf == 0x55)
			{

				startCodeSum++;
				if(startCodeSum == 2)
				{
					startCodeSum = 0;
					fFrameStart = TRUE;
					messageLength = 1;
				}
			}
			else
			{

				fFrameStart = FALSE;
				messageLength = 0;
	
				startCodeSum = 0;
			}
			
		}
		if(fFrameStart)
		{
			Uart1RxBuffer[messageLength] = rxBuf;
			if(messageLength == 2)
			{
				messageLengthSum = Uart1RxBuffer[messageLength];
				if(messageLengthSum < 2)// || messageLengthSum > 30
				{
					messageLengthSum = 2;
					fFrameStart = FALSE;
					
				}
					
			}
			messageLength++;
	
			if(messageLength == messageLengthSum + 2) 
			{
				if(fUartRxComplete == FALSE)
				{
					fUartRxComplete = TRUE;
					for(i = 0;i < messageLength;i++)
					{
						UartRxBuffer[i] = Uart1RxBuffer[i];
					}
				}
				

				fFrameStart = FALSE;
			}
		}
    }
    if (TI)
    {
        TI = 0;                 //清除标志位
    }
}

void McuToPCSendData(uint8 cmd,uint8 prm1,uint8 prm2)
{
	uint8 dat[8];
	uint8 datlLen = 2;
	switch(cmd)
	{

//		case CMD_ACTION_DOWNLOAD:
//			datlLen = 2;
//			break;

		default:
			datlLen = 2;
			break;
	}

	dat[0] = 0x55;
	dat[1] = 0x55;
	dat[2] = datlLen;
	dat[3] = cmd;
	dat[4] = prm1;
	dat[5] = prm2;
	UART1SendDataPacket(dat,datlLen + 2);
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
void FlashEraseAll();
void SaveAct(uint8 fullActNum,uint8 frameIndexSum,uint8 frameIndex,uint8* pBuffer);
void TaskPCMsgHandle(void)
{

	uint16 i;
	uint8 cmd;
	uint8 id;
	uint8 servoCount;
	uint16 time;
	uint16 pos;
	uint16 times;
	uint8 fullActNum;
	if(UartRxOK())
	{
		LED = !LED;
		cmd = UartRxBuffer[3];
 		switch(cmd)
 		{
 			case CMD_MULT_SERVO_MOVE:
				servoCount = UartRxBuffer[4];
				time = UartRxBuffer[5] + (UartRxBuffer[6]<<8);
				for(i = 0; i < servoCount; i++)
				{
					id =  UartRxBuffer[7 + i * 3];
					pos = UartRxBuffer[8 + i * 3] + (UartRxBuffer[9 + i * 3]<<8);
	
					ServoSetPluseAndTime(id,pos,time);
					BusServoCtrl(id,SERVO_MOVE_TIME_WRITE,pos,time);
				}
 				break;
			
			case CMD_FULL_ACTION_RUN:
				fullActNum = UartRxBuffer[4];//动作组编号
				times = UartRxBuffer[5] + (UartRxBuffer[6]<<8);//运行次数
				FullActRun(fullActNum,times);
				break;
				
			case CMD_FULL_ACTION_STOP:
				FullActStop();
				break;
				
			case CMD_FULL_ACTION_ERASE:
				FlashEraseAll();
				McuToPCSendData(CMD_FULL_ACTION_ERASE,0,0);
				break;

			case CMD_ACTION_DOWNLOAD:
				SaveAct(UartRxBuffer[4],UartRxBuffer[5],UartRxBuffer[6],UartRxBuffer + 7);
				McuToPCSendData(CMD_ACTION_DOWNLOAD,0,0);
				break;

			case openmv_1:	
				FullActRun(1,1);
				break;

			case openmv_2:
				FullActRun(2,1);
				break;

			case openmv_3:
				FullActRun(3,1);
				break;

			case openmv_4:
				FullActRun(4,1);
				break;

			case openmv_5:
				FullActRun(5,1);
				break;

			case openmv_6:
				FullActRun(6,1);
				break;

 		}
	}
}

void SaveAct(uint8 fullActNum,uint8 frameIndexSum,uint8 frameIndex,uint8* pBuffer)
{
	uint8 i;
	
	if(frameIndex == 0)//下载之前先把这个动作组擦除
	{//一个动作组占16k大小，擦除一个扇区是4k，所以要擦4次
		for(i = 0;i < 4;i++)//ACT_SUB_FRAME_SIZE/4096 = 4
		{
			FlashEraseSector((MEM_ACT_FULL_BASE) + (fullActNum * ACT_FULL_SIZE) + (i * 4096));
		}
	}

	FlashWrite((MEM_ACT_FULL_BASE) + (fullActNum * ACT_FULL_SIZE) + (frameIndex * ACT_SUB_FRAME_SIZE)
		,ACT_SUB_FRAME_SIZE,pBuffer);
	
	if((frameIndex + 1) ==  frameIndexSum)
	{
		FlashRead(MEM_FRAME_INDEX_SUM_BASE,256,frameIndexSumSum);
		frameIndexSumSum[fullActNum] = frameIndexSum;
		FlashEraseSector(MEM_FRAME_INDEX_SUM_BASE);
		FlashWrite(MEM_FRAME_INDEX_SUM_BASE,256,frameIndexSumSum);
	}
}


void FlashEraseAll()
{//将所有255个动作组的动作数设置为0，即代表将所有动作组擦除
	uint16 i;
	
	for(i = 0;i <= 255;i++)
	{
		frameIndexSumSum[i] = 0;
	}
	FlashEraseSector(MEM_FRAME_INDEX_SUM_BASE);
	FlashWrite(MEM_FRAME_INDEX_SUM_BASE,256,frameIndexSumSum);
}

void InitMemory(void)
{
	uint8 i;
	uint8 code logo[] = "LOBOT";
	uint8 datatemp[8];

	FlashRead(MEM_LOBOT_LOGO_BASE,5,datatemp);
	for(i = 0; i < 5; i++)
	{
		if(logo[i] != datatemp[i])
		{
			//如果发现不相等的，则说明是新FLASH，需要初始化
			FlashEraseSector(MEM_LOBOT_LOGO_BASE);
			FlashWrite(MEM_LOBOT_LOGO_BASE,5,logo);
			FlashEraseAll();
			break;
		}
	}
	
}
uint16 UART1ModbusCRC(uint8 *ptr,uint16 ucLen)//CRC校验
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
int16 getdata1(void)
{
	//uint8 dat[7];
 	unsigned int uiData;
	UART1SendDataPacket(CMD_1,8);
	DelayMs(100);
	uiData=ucRxData[3]<<8|ucRxData[4];		
	ucRxFinish=0;
    return uiData;	
}

