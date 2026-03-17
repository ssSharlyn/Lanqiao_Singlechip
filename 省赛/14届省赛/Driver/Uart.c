#include <Uart.h>
#include <stdio.h>

bit Uart_Busy;

void Uart_Init()	//9600bps@12.000MHz
{
	SCON = 0x50;		
	AUXR |= 0x01;		
	AUXR |= 0x04;		
	T2L = 0xC7;			
	T2H = 0xFE;			
	AUXR |= 0x10;		
	ES = 1;
	EA = 1;
}

//void Uart_SendByte(unsigned char dat)
//{
//	SBUF = dat;
//	while(TI == 0);
//	TI = 0;
//}

//void Uart_SendString(unsigned char *dat)
//{
//	while(*dat != '\0')
//	  Uart_SendByte(*dat++);
//}

extern char putchar(char dat)
{
	while(Uart_Busy);
	Uart_Busy = 1;
	SBUF = dat;
	return dat;
}

void UART_Interrupt() interrupt 4 
{
  if(TI) 
   {
     TI = 0; // 清除发送标志
     Uart_Busy = 0; // 标记发送完成
   }
}