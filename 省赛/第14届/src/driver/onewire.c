/*	# 	单总线代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/

#include "onewire.h"

sbit DQ = P1^4;

//
void Delay_OneWire(unsigned int t)  
{
	unsigned char i;
	while(t--){
		for(i=0;i<12;i++);
	}
}

//
void Write_DS18B20(unsigned char dat)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		DQ = 0;
		DQ = dat&0x01;
		Delay_OneWire(5);
		DQ = 1;
		dat >>= 1;
	}
	Delay_OneWire(5);
}

//
unsigned char Read_DS18B20(void)
{
	unsigned char i;
	unsigned char dat;
  
	for(i=0;i<8;i++)
	{
		DQ = 0;
		dat >>= 1;
		DQ = 1;
		if(DQ)
		{
			dat |= 0x80;
		}	    
		Delay_OneWire(5);
	}
	return dat;
}

//
bit init_ds18b20(void)
{
  	bit initflag = 0;
  	
  	DQ = 1;
  	Delay_OneWire(12);
  	DQ = 0;
  	Delay_OneWire(80);
  	DQ = 1;
  	Delay_OneWire(10); 
    initflag = DQ;     
  	Delay_OneWire(5);
  
  	return initflag;
}

/**
 * @brief  DS18B20温度值读取函数
 * @return 转换后的实际温度
 * @note   DS18B20 默认12位分辨率，步进为0.0625°C
 */
float read_temperature(void) {
    unsigned char low, high; // 存储温度低 8 位和高 8 位

    // --- 启动温度转换 ---
    init_ds18b20(); // 初始化DS18B20
    Write_DS18B20(0xcc); // 发送 Skip ROM 命令 (0xCC)，跳过 64 位序列号匹配（单总线仅一个设备时使用）
    Write_DS18B20(0x44); // 发送 Convert T 命令 (0x44)，启动温度转换

    
    Delay_OneWire(200); // 等待转换完成 

    // --- 读取暂存寄存器 ---
    init_ds18b20(); // 读取前必须再次初始化总线，否则无法通信
    Write_DS18B20(0xcc); // 再次跳过 ROM 匹配
    Write_DS18B20(0xbe); // 发送 Read Scratchpad 命令 (0xBE)，准备读取 9 字节数据

    low = Read_DS18B20(); // 读取温度 LSB (低 8 位)
    high = Read_DS18B20(); // 读取温度 MSB (高 8 位)

    return ((high << 8) | low) * 0.0625; // 返回读取的温度值
}
