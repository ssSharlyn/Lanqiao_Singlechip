#include "ultrasonic.h"
#include "intrins.h"

sbit TX = P1^0;
sbit RX = P1^1;

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	i = 3;
	while (--i);
}

void ultrasonic_init(void)
{
	unsigned char i;
	
	EA = 0;
	for(i = 0; i < 8; i++)
	{
		TX = 1;
		Delay12us();
		TX = 0;
		Delay12us();
	}
	EA = 1;
}

unsigned char ultrasonic_dat(void)
{
	unsigned int time;
	CMOD = 0x00;
	CL = CH = 0;
	ultrasonic_init();
	CR = 1;
	while((RX == 1) && (CF == 0)); //等待接受到返回信号或者定时器溢出
	CR = 0;
	//接受到返回信号
	if(CF == 0)
	{
		time = (CH << 8) | CL;
		return (time * 0.017);
	}
	//定时器溢出
	else
	{
		CF = 0;
		return 0;
	}
}