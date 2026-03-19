#include "ultra.h"

sbit TX = P1^0; // 触发引脚
sbit RX = P1^1; // 回响引脚

/**
 * @brief 12us延时函数
 * @note  用于产生40kHz左右的超声波方波
 */
void delay12us(void) {
	unsigned char data i;

	_nop_();
	i = 3;
	while (--i);
}

/**
 * @brief 超声波初始化函数
 * @note  发送8个超声波脉冲 
 */
void ultrasonic_init(void) {
  unsigned char i;

  EA = 0;
  for(i = 0; i < 8; i++) {
		TX = 1;
		delay12us();
		TX = 0;
		delay12us();
	}
	
 	EA = 1;
}

/**
 * @brief  获取超声波测距函数
 * @return 距离值(单位: cm)
 */
unsigned int ultra_dat(void) {    
	unsigned int time;
	
	// 初始化PCA定时器
	CMOD = 0x00;
	CL = CH = 0;

	ultrasonic_init(); 	// 发送触发信号
	CR = 1; // 启动PCA定时器
	while((RX == 1) && (CF == 0)); //等待接受到返回信号或者定时器溢出
	CR = 0; // 关闭PCA定时器
	
	// 定时器未溢出，成功接收到回响信号
	if(CF == 0) {
	  time = (CH << 8) | CL; // 计算超声波传播的时间
    return (time * 0.017); // 返回测得的距离值		
	}
	// 定时器溢出
	else {
		CF = 0; // 清除溢出标志
		return 0;
	}
}