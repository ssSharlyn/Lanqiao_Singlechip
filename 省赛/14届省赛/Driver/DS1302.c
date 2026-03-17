#include <DS1302.h>
#include <intrins.h>

sbit RST = P1^3;
sbit SCK = P1^7;
sbit SDA = P2^3;

void Write_DS1302(unsigned char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp & 0x01;
		temp >>= 1; 
		SCK = 1;
	}
}   

void Write_DS1302_Byte( unsigned char address, dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_DS1302(address);	
 	Write_DS1302(dat);		
 	RST=0; 
}

unsigned char Read_DS1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_DS1302(address);
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

void Write_RTC(unsigned char* ucRTC)
{
	Write_DS1302_Byte(0x8e,0x00);
	Write_DS1302_Byte(0x84,ucRTC[0] / 10 * 16 + ucRTC[0] % 10);
	Write_DS1302_Byte(0x82,ucRTC[1] / 10 * 16 + ucRTC[1] % 10);
	Write_DS1302_Byte(0x80,ucRTC[2] / 10 * 16 + ucRTC[2] % 10);
	Write_DS1302_Byte(0x8e,0x80);
}

void Read_RTC(unsigned char* ucRTC)
{
	ucRTC[0] = Read_DS1302_Byte(0x85) / 16 * 10 + Read_DS1302_Byte(0x85) % 16;
	ucRTC[1] = Read_DS1302_Byte(0x83) / 16 * 10 + Read_DS1302_Byte(0x83) % 16;
	ucRTC[2] = Read_DS1302_Byte(0x81) / 16 * 10 + Read_DS1302_Byte(0x81) % 16;
}