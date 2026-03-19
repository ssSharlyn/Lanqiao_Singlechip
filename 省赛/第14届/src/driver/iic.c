/*	#   I2C代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/

#include "iic.h"

sbit scl = P2^0; // iic串行时钟线
sbit sda = P2^1; // iic串行数据线

#define DELAY_TIME 5 //*** 注：将原代码中的"10"替换为"5" 

//*** 注：将原代码中的"_nop()"删至1个
//
static void I2C_Delay(unsigned char n)
{
    do
    {
      _nop_();	
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

/**
 * @brief  PCF8591指定通道的AD转换结果读取函数
 * @param  addr 控制字节(0x01-光敏电阻, 0x03-电位器Rb2)
 * @return 8位转换结果(0~255)
 */
unsigned char ad_read(unsigned char addr)
{
	unsigned char temp;
	
	// --- 写入控制字节 ---
	I2CStart();
	I2CSendByte(0x90); // 发送器件写地址
	I2CWaitAck();
	I2CSendByte(addr); // 发送控制字节，选择具体的模拟输入通道
	I2CWaitAck();
	
	// --- 读取转换数据 ---
	I2CStart();
	I2CSendByte(0x91); // 发送器件读地址
	I2CWaitAck();
	temp = I2CReceiveByte(); // 读取1字节数据(当前通道的AD值)
	I2CSendAck(1); // 发送非应答信号(NACK)，告诉从机读取结束
	I2CStop();
	
	return temp;
}

/**
 * @brief  PCF8591 DA转换输出函数
 * @param  dat 待输出的模拟量数值(0~255，对应 0~5V)
 */
void da_write(unsigned char dat)
{
	I2CStart();
	I2CSendByte(0x90); // 发送器件写地址(0x90)
	I2CWaitAck();
	I2CSendByte(0x40); // 发送控制字节(0x40表示使能DAC输出)
	I2CWaitAck();
	I2CSendByte(dat); // 发送8位数字量，转换为模拟电压输出
	I2CWaitAck();
}
