/*	#   I2C代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/

#include "iic.h"
#include <intrins.h>

sbit scl = P2^0;
sbit sda = P2^1;

/*<< 注: 电压显示异常可将DELAY_TIME改为5 >>*/
#define DELAY_TIME	5

//
static void I2C_Delay(unsigned char n)
{
    do
    {
      _nop_();_nop_();_nop_();_nop_();_nop_();
      _nop_();_nop_();_nop_();_nop_();_nop_();
      _nop_();_nop_();_nop_();_nop_();_nop_();		
    }
    while(n--);      	
}

//
void I2CStart(void)
{
    sda = 1;
    scl = 1;
	I2C_Delay(DELAY_TIME);
    sda = 0;
	I2C_Delay(DELAY_TIME);
    scl = 0;    
}

//
void I2CStop(void)
{
    sda = 0;
    scl = 1;
	I2C_Delay(DELAY_TIME);
    sda = 1;
	I2C_Delay(DELAY_TIME);
}

//
void I2CSendByte(unsigned char byt)
{
    unsigned char i;
	
    for(i=0; i<8; i++){
        scl = 0;
		I2C_Delay(DELAY_TIME);
        if(byt & 0x80){
            sda = 1;
        }
        else{
            sda = 0;
        }
		I2C_Delay(DELAY_TIME);
        scl = 1;
        byt <<= 1;
		I2C_Delay(DELAY_TIME);
    }
	
    scl = 0;  
}

//
unsigned char I2CReceiveByte(void)
{
	unsigned char da;
	unsigned char i;
	for(i=0;i<8;i++){   
		scl = 1;
		I2C_Delay(DELAY_TIME);
		da <<= 1;
		if(sda) 
			da |= 0x01;
		scl = 0;
		I2C_Delay(DELAY_TIME);
	}
	return da;    
}

//
unsigned char I2CWaitAck(void)
{
	unsigned char ackbit;
	
    scl = 1;
	I2C_Delay(DELAY_TIME);
    ackbit = sda; 
    scl = 0;
	I2C_Delay(DELAY_TIME);
	
	return ackbit;
}

//
void I2CSendAck(unsigned char ackbit)
{
    scl = 0;
    sda = ackbit; 
	I2C_Delay(DELAY_TIME);
    scl = 1;
	I2C_Delay(DELAY_TIME);
    scl = 0; 
	sda = 1;
	I2C_Delay(DELAY_TIME);
}


unsigned char ad_read(unsigned char addr)
{
	unsigned char temp; 
	
	I2CStart();
	I2CSendByte(0x90); //选择PCF8591在IIC的地址，解除写保护
	I2CWaitAck(); //等待接收信号
	I2CSendByte(addr); //发送需要转换的地址
	I2CWaitAck(); //等待接收信号
	
	I2CStart();
	I2CSendByte(0x91); //选择PCF8591在IIC的地址，进行读取
	I2CWaitAck(); //等待接收信号
	temp = I2CReceiveByte(); //读取转换完成的数据
	I2CSendAck(1); //发送读取完毕信号
	I2CStop();
	
	return temp;
}

void da_write(unsigned char dat)
{
	I2CStart();
	I2CSendByte(0x90); //选择PCF8591在IIC的地址，解除写保护
	I2CWaitAck(); //等待接收信号
	I2CSendByte(0x41); //使能da转换
	I2CWaitAck(); //等待接收信号
	I2CSendByte(dat); //发送需要转换的数据
	I2CWaitAck(); //等待接收信号
}

void eeprom_read(unsigned char *str, addr, num)
{
	I2CStart();
	I2CSendByte(0xa0); //选择AT24C02在IIC的地址，解除写保护
	I2CWaitAck(); //等待接收信号
	I2CSendByte(addr); //发送数据存储的地址
	I2CWaitAck(); //等待接收信号
	
	I2CStart();
	I2CSendByte(0xa1); //选择AT24C02在IIC的地址，进行读取
	I2CWaitAck(); //等待接收信号
	while(num--)
	{
		*str++ = I2CReceiveByte(); //读取数据
		if(num)
			I2CSendAck(0); //发送继续读取信号	
		else	
		  I2CSendAck(1); //发送读取完毕信号	
	}	
  I2CStop();
}

void eeprom_write(unsigned char *str, addr, num)
{
	I2CStart();
	I2CSendByte(0xa0); //选择AT24C02在IIC的地址，解除写保护
	I2CWaitAck(); //等待接收信号
	I2CSendByte(addr); //发送数据存储的地址
	I2CWaitAck(); //等待接收信号
	
	while(num--)
	{
		I2CSendByte(*str++); //发送需要写入的数据
	  I2CWaitAck(); //等待接收信号
		I2C_Delay(200);
	}
	I2CStop();
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
	I2C_Delay(255);
}