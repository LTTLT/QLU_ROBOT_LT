#include "include.h"

void main(void)
{
	P0M1=P0M0=P1M1=P1M0=P2M1=P2M0=P3M1=P3M0=P4M1=P4M0=P5M1=P5M0=P6M1=P6M0=P7M1=P7M0= 0;	
	//ȫ�����ó�׼˫���
	
	InitPWM();		//�����г�ʼ��Timer0����ʼ�������PWM��

	InitTimer3();	//��ʼ��Timer3,���ڲ���100us�Ķ�ʱ�ж�
	InitUart1();	//������ ��Timer1������������PC�˽���ͨ��
  	InitUart2();
	InitADC();
	InitBusServoCtrl();
	
	P3M0 |= 0x80;	//���������������������
	EA = 1;			//�����ж�
	InitFlash();
	InitMemory();
	LED = LED_ON;
	for(;;)
	{
		TaskRun();
	}
}

