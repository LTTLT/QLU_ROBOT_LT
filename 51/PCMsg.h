#ifndef _PC_MSG_H_
#define _PC_MSG_H_




//ָ��
#define CMD_MULT_SERVO_MOVE					3	//��������ͬʱ���˶�
#define CMD_FULL_ACTION_RUN					6
#define CMD_FULL_ACTION_STOP				7
#define CMD_FULL_ACTION_ERASE				8
#define CMD_ACTION_DOWNLOAD					25
#define openmv_1							9	 //��ɫ
#define openmv_2							10	  //a
#define openmv_3							11	   //b
#define openmv_4							12	   // ��ɫ c
#define openmv_5							13	//����	  d
#define openmv_6							14	//		  e
//�����ʼ��ַ
#define MEM_LOBOT_LOGO_BASE					0L	//"LOBOT"��Ż���ַ������ʶ���Ƿ�����FLASH
#define MEM_FRAME_INDEX_SUM_BASE			4096L//ÿ���������ж��ٶ������������ַ��ʼ��ţ�����256��������
#define MEM_ACT_FULL_BASE					8192L//�������ļ��������ַ��ʼ���

//��С
#define ACT_SUB_FRAME_SIZE					64L		//һ������֡ռ64�ֽڿռ�
#define ACT_FULL_SIZE						16384L	//16KB,һ������������ռ14kb�ֽ�







void InitUart1(void);

void TaskPCMsgHandle(void);
void InitMemory(void);
extern void McuToPCSendData(uint8 cmd,uint8 prm1,uint8 prm2);
extern void UART1SendDataPacket(uint8 dat[],uint8 count);
int16 getdata1(void);

#endif
