/*	# 	DS1302代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/								

#include "ds1302.h"

sbit RST = P1^3; // 复位/使能端 
sbit SCK = P1^7; // 串行时钟线
sbit SDA = P2^3; // 串行数据线 

//
void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

//
void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

//
unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}

/**
 * @brief  DS1302写入时间函数
 * @param  ucRTC 指向待写入时间数组的指针(格式: [0]-时, [1]-分, [2]-秒)
 * @note   DS1302内部使用 BCD 码存储，写入前需将十进制转换为BCD码
 */
void write_rtc(unsigned char *ucRTC) {
	unsigned char i;
	
	Write_Ds1302_Byte(0x8e,0x00); // 解除写保护
	Write_Ds1302_Byte(0x80,0x80); // 停止时钟
	for(i = 0; i < 3; i++) {
	  Write_Ds1302_Byte(0x84 - i*2, ucRTC[i] / 10 * 16 + ucRTC[i] % 10); 
	}
	Write_Ds1302_Byte(0x8e,0x80); // 开启写保护
}

/**
 * @brief  DS1302读取时间函数
 * @param  ucRTC 指向存储时间结果数组的指针
 * @note   读取出的数据为BCD码，需转换为十进制以便数码管显示
 */
void read_rtc(unsigned char *ucRTC) {
	unsigned char temp;
	unsigned char i;
	
	EA = 0; // 屏蔽中断，防止 SPI 模拟时序被中断打断导致读取错误
	for(i = 0; i < 3; i++) {
		temp = Read_Ds1302_Byte(0x85 - 2*i);
	  ucRTC[i] = temp / 16 * 10 + temp % 16; 
	}
	EA = 1; // 恢复中断
}