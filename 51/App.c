#include "include.h"


#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x10            //ADC完成标志
#define ADC_START   0x08            //ADC起始控制位
#define ADC_SPEEDLL 0x00            //540个时钟
#define ADC_SPEEDL  0x20            //360个时钟
#define ADC_SPEEDH  0x40            //180个时钟
#define ADC_SPEEDHH 0x60            //90个时钟

#define ADC_BAT		6		//电池电压的AD检测通道
static bool UartBusy = FALSE;

u32 gSystemTickCount = 0;	//系统从启动到现在的毫秒数

uint8 BuzzerState = 0;
uint8 Mode = 0;
uint16 xdata Ps2TimeCount = 0;

uint16 xdata BatteryVoltage;

void DelayMs(uint16 ms)
{
	uint16 i,j;
	for(i=0;i<800;i++)
		for(j=0;j<ms;j++);
}


void InitADC(void)
{
    P1ASF = 0x40;                   //
    ADC_RES = 0;                    //清除结果寄存器
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
    //Delay(2);                       //ADC上电并延时
}


uint16 GetADCResult(BYTE ch)
{
	uint16 ad;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
    _nop_();                        //等待4个NOP
    _nop_();
    _nop_();
    _nop_();
    while (!(ADC_CONTR & ADC_FLAG));//等待ADC转换完成
    ADC_CONTR &= ~ADC_FLAG;         //Close ADC
    
    ad = ADC_RES;
	ad = (ad << 2) + (ADC_RESL & 0x03);
	return ad;
}



void InitTimer3(void)		//100us@12.000MHz
{
	T4T3M |= 0x02;
	T3L = 0x50;
	T3H = 0xFB;
	T4T3M |= 0x08;
	IE2 |= 0x20;
}




void CheckBatteryVoltage(void)
{
	uint8 i;
	uint32 v = 0;
	for(i = 0;i < 8;i++)
	{
		v += GetADCResult(ADC_BAT);
	}
	v >>= 3;
	
	v = v * 2475 / 256;//adc / 1024 * 3300 * 3(3代表放大3倍，因为采集电压时电阻分压了)
	BatteryVoltage = v;
}

uint16 GetBatteryVoltage(void)
{//电压毫伏
	return BatteryVoltage;
}

void Buzzer(void)
{//放到100us的定时中断里面
	static bool fBuzzer = FALSE;
	static uint32 t1 = 0;
	static uint32 t2 = 0;
	if(fBuzzer)
	{
		if(++t1 >= 2)
		{
			t1 = 0;
			BUZZER = !BUZZER;//2.5KHz
		}
	}
	else
	{
		BUZZER = 0;
	}

	
	if(BuzzerState == 0)
	{
		fBuzzer = FALSE;
		t2 = 0;
	}
	else if(BuzzerState == 1)
	{
		t2++;
		if(t2 < 5000)
		{
			fBuzzer = TRUE;
		}
		else if(t2 < 10000)
		{
			fBuzzer = FALSE;
		}
		else
		{
			t2 = 0;
		}
	}
}

bool  manual = FALSE;
void t3int() interrupt 19
{//定时器3中断  100us
	static uint16 time = 0;
	static uint16 timeBattery = 0;

	Buzzer();
	if(++time >= 10)
	{
		time = 0;
		gSystemTickCount++;
		Ps2TimeCount++;
		if(GetBatteryVoltage() < 6400)//小于6.4V报警
		{
			timeBattery++;
			if(timeBattery > 5000)//持续5秒
			{
				//BuzzerState = 1;
			}
		}
		else
		{
			timeBattery = 0;
			if ( manual == FALSE)
			{
			//	BuzzerState = 0;
			}
		}
	}
}

void TaskTimeHandle(void)
{
	static uint32 time = 10;
	static uint32 times = 0;
	if(gSystemTickCount > time)
	{
		time += 10;
		times++;
		if(times % 2 == 0)//20ms
		{
			ServoPwmDutyCompare();
		}
	}
	
}

void TaskRun(void)
{

	static uint8 keycount = 0;
	int16 distance1;
	int16 distance2;
	TaskTimeHandle();
	CheckBatteryVoltage();
	TaskPCMsgHandle();
	TaskBLEMsgHandle();
	TaskRobotRun();

	if(KEY == 0)
	{
		DelayMs(60);
		{
			if(KEY == 0)
			{
				keycount++;
			}
			else
			{
				if (keycount > 20)
				{
//					keycount = 0;
//					DelayMs(5000);
////					clear();
					FullActRun(1,1);
					DelayMs(5000);
					FullActRun(0,1);
					McuToPCSendData(CMD_FULL_ACTION_ERASE,0,0);
//					distance1 = getdata();
//				   	distance2 = getdata1();
//					if(distance2<200&&distance1<200) LED = ~LED;			
					return;
				}
				else
				{
					keycount = 0;
					LED = ~LED;
					FullActRun(1,1);
					DelayMs(5000);
					FullActRun(0,1);
					McuToPCSendData(CMD_FULL_ACTION_ERASE,0,0);
//					distance1 =getdata();
//					distance2 = getdata1();
//					if(distance2<200&&distance1<200) LED = ~LED;
				}
			}
		}
	}
}
