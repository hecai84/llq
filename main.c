//==============================================================================
//��Ŀ							     XC8P9520	DOME
//MCU				  				 XC8P9520
//���							     Louie
//ʱ��							     8M 4Clock
//WDT				  				 Disable = 1 & Enable = 0
//RESET				  				 P63,������
//�������		      				 2021.04.21
//XC8P9520��1��˫��I/O�˿�:		     ��6�����룬6��������󲿷�I/O���Ը���Ϊ�������ܡ�
//6���ɱ������I/O����:				 P6.0��P6.5��
//5���ɱ������I/O����:				 P6.0��P6.2,P6.4��P6.5;
//6���ɱ��������ǿI/O����:		     P6.0��P6.5;
//�����ö�������P6��
//1·�ⲿ�жϣ�P60
//3·������8Bit=PWM
//���ѹ��λ��1.2V  1.6V  1.8V   2.4V   2.7V   3.1V   3.3V   3.6V
//�ж�Դ��TCC����ж�	/�ⲿ�ж�	/����״̬�ı��ж�	/T1��PWM��������ж�
//�˿�              SMT                 HSMT               EMT             INV
//    P6.3          0.5*VDD         0.2*VDD/0.8*VDD   0.2*VDD/0.35*VDD   0.5*VDD
// P6.0��P6.2    0.2*VDD/0.6*VDD    0.2*VDD/0.8*VDD   0.2*VDD/0.35*VDD   0.5*VDD
// P6.4��P6.5    0.2*VDD/0.6*VDD    0.2*VDD/0.8*VDD   0.3*VDD      		 0.5*VDD
//==============================================================================

//==============================================================================
//ͷ�ļ�	����ͷ�ļ�	ͨ�üĴ��������ļ�
//==============================================================================
#include "XC8P9520.h"

#define uint16_t unsigned int
#define uint8_t unsigned char

#define DISI() _asm {disi}
#define ENI() _asm {eni}
#define WDTC() _asm {wdtc}
#define NOP() _asm {nop}
#define SLEP() _asm {slep}

//===============================================================================
//�ӳ�������
//===============================================================================
void io_init();
void tcc_init();
void File_Sleep();
void io_key();
void Delay();
unsigned char pwmDtAdd = 1;
unsigned char pwmDt = 18;


void Delay()
{
	unsigned char i, j, k;
	for (i = 15; i > 0; i--)
		for (j = 6; j > 0; j--)
			for (k = 50; k > 0; k--)
				;
}
void io_init()
{
	P6CR=0XE0;					//P65���� ,�������
	PHCR=0XDF;					//P65����,������ֹ����
	PDCR=0xEF;					//P60����,������ֹ����	
	PORT6=0X21;					//P65��Ϊ��,P60���ø�
}

void tcc_init()
{
	_asm
	{
		mov a,@0x02
		contw //tccԤ��Ƶ1:8
	}
	WDTCR = 0x00;
	WDTC();		//�幷
	IMR = 0x01; //tcc�ж�
	ENI();
	ISR = 0x00; //���жϱ�־λ
	SLEEP = 1;
	ICIECR = 0x20; //����P65����
}

void File_Sleep()
{
	if (SLEEP == 1)
	{
		//if(++Reg_Sleep>100)
		//{
		//	Reg_Sleep=0;
		DISI();
		_asm
		{	mov a,@0x0F //��Ƶ
				contw
		}
		WDTCR = 0x00; //�ع�
		TCC = 0x00;
		ISR = 0x00;
		IMR = 0x02; //����P6�������ƽ�仯�ж�
		PORT6 = PORT6;
		SLEP();
		NOP();
		NOP();
		NOP();
		NOP();
		WDTCR = 0x00;
		_asm
		{	
				mov a,@0x01 //��Ƶ		1:8
				contw
		}
		ISR = 0x00;
		IMR = 0x01;
		TCC = 156;
		ENI();
		//P63=1;
		NOP();
	}
	//	}
	//else Reg_Sleep=0;
}

void pwm_init()
{
	PWMCON = 0x9d; //pwm��������pwm1/3ʹ��,t1 1:256
	PRD = (pwmDt<<1)-1;	   //	PWM = (1/8M) * 256 * (34+1) = 1120us
	PDC1 = pwmDt;
	//PDC2 = 20;
	//PDC3 = pwmDt;
}

void main()
{
	unsigned char i;
	DISI();
	_asm
	{
		mov a,@0x00
		contw
	}
	io_init();
	tcc_init();
	P60=1;
	for(i=0;i<100;i++)
	{
		Delay();
	}
	while (1)
	{
		if (P65 == 0)
		{
			Delay();
			Delay();
			Delay();
			Delay();
			Delay();
			if(P65 == 0)
			{
				pwm_init();
				P60=0;
				while (1)
				{
					PRD = (pwmDt<<1)-1;	   //	PWM = (1/8M) * 256 * (34+1) = 1120us
					PDC1 = pwmDt;
					//PDC3 = pwmDt;
					
					if (pwmDt >= 30)
						pwmDtAdd = 0;
					if (pwmDt <= 10)
						pwmDtAdd = 1;
	
					if (pwmDtAdd == 1)
						pwmDt++;
					else
						pwmDt--;
					Delay();
				}
			}
		}
	}
}
void _intcall interrupt(void) @ int
{
	ISR = 0x00;
	TCC = +6;
	//	if(++TCC>6)
	//	{
	TCCEnable = 1;
	//}
	_asm
	{
     	reti //Tell Compiler to recover common registers here.
		   //Don't remove or duplicate this instruction in this function
	}
}
void _intcall interrupt_l(void) @0x08 : low_int 0
{
	_asm {
		//backup acc, don't remove
			mov 0x1F , a
	}
}
