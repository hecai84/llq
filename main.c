//==============================================================================
//项目							     XC8P9520	DOME
//MCU				  				 XC8P9520
//编程							     Louie
//时钟							     8M 4Clock
//WDT				  				 Disable = 1 & Enable = 0
//RESET				  				 P63,开上拉
//完成日期		      				 2021.04.21
//XC8P9520有1组双向I/O端口:		     共6个输入，6个输出，大部分I/O可以复用为其它功能。
//6个可编程上拉I/O引脚:				 P6.0～P6.5；
//5个可编程下拉I/O引脚:				 P6.0～P6.2,P6.4～P6.5;
//6个可编程驱动增强I/O引脚:		     P6.0～P6.5;
//可设置独立唤醒P6口
//1路外部中断：P60
//3路共周期8Bit=PWM
//宽电压复位：1.2V  1.6V  1.8V   2.4V   2.7V   3.1V   3.3V   3.6V
//中断源：TCC溢出中断	/外部中断	/输入状态改变中断	/T1、PWM周期溢出中断
//端口              SMT                 HSMT               EMT             INV
//    P6.3          0.5*VDD         0.2*VDD/0.8*VDD   0.2*VDD/0.35*VDD   0.5*VDD
// P6.0～P6.2    0.2*VDD/0.6*VDD    0.2*VDD/0.8*VDD   0.2*VDD/0.35*VDD   0.5*VDD
// P6.4～P6.5    0.2*VDD/0.6*VDD    0.2*VDD/0.8*VDD   0.3*VDD      		 0.5*VDD
//==============================================================================

//==============================================================================
//头文件	运用头文件	通用寄存器定义文件
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
//子程序声明
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
		for (j = 80; j > 0; j--)
			for (k = 50; k > 0; k--)
				;
}
void io_init()
{
	P6CR=0X70;					//P65输入 ,其他输出
	PORT6=0X20;					//P65设为高
	PHCR=0XDF;					//P65上拉,其他禁止上拉
	PDCR=0xff;					//禁止下拉	
}

void tcc_init()
{
	_asm
	{
		mov a,@0x02
		contw //tcc预分频1:8
	}
	WDTCR = 0x00;
	WDTC();		//清狗
	IMR = 0x01; //tcc中断
	ENI();
	ISR = 0x00; //清中断标志位
	SLEEP = 1;
	ICIECR = 0x20; //设置P65唤醒
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
		{	mov a,@0x0F //分频
				contw
		}
		WDTCR = 0x00; //关狗
		TCC = 0x00;
		ISR = 0x00;
		IMR = 0x02; //开启P6口输入电平变化中断
		PORT6 = PORT6;
		SLEP();
		NOP();
		NOP();
		NOP();
		NOP();
		WDTCR = 0x00;
		_asm
		{	
				mov a,@0x01 //分频		1:8
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
	PWMCON = 0xdd; //pwm计数器，pwm1/3使能,t1 1:256
	PRD = pwmDt*2-1;	   //	PWM = (1/8M) * 256 * (34+1) = 1120us
	PDC1 = pwmDt;
	//PDC2 = 20;
	PDC3 = pwmDt;
}

void main()
{
	DISI();
	_asm
	{
		mov a,@0x00
		contw
	}
	io_init();
	tcc_init();
	while (1)
	{
		if (P65 == 0)
		{
			Delay();
			Delay();
			if(P65 == 0)
			{
				pwm_init();
				while (1)
				{
					PRD = pwmDt*2-1;	   //	PWM = (1/8M) * 256 * (34+1) = 1120us
					PDC1 = pwmDt;
					PDC3 = pwmDt;
					
					if (pwmDt >= 31)
						pwmDtAdd = 0;
					if (pwmDt <= 5)
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
