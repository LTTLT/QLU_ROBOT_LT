#include "include.h"

void main(void)
{
	P0M1=P0M0=P1M1=P1M0=P2M1=P2M0=P3M1=P3M0=P4M1=P4M0=P5M1=P5M0=P6M1=P6M0=P7M1=P7M0= 0;	
	//全部设置成准双向口
	
	InitPWM();		//里面有初始化Timer0，初始化舵机的PWM，

	InitTimer3();	//初始化Timer3,用于产生100us的定时中断
	InitUart1();	//波特率 用Timer1产生，用于与PC端进行通信
  	InitUart2();
	InitADC();
	InitBusServoCtrl();
	
	P3M0 |= 0x80;	//蜂鸣器控制引脚推挽输出
	EA = 1;			//开总中断
	InitFlash();
	InitMemory();
	LED = LED_ON;
	for(;;)
	{
		TaskRun();
	}
}

