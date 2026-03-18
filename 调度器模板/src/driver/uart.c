#include "uart.h"

/**
 * @brief 串口1初始化函数
 * @note  使用定时器2作为波特率发生器，配置为12T模式
 */
void uart1_init(void)	{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0xE6;			//设置定时初始值
	T2H = 0xFF;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	ES = 1;         // 使能串口中断(若只需发送，可关闭此项)
	EA = 1;         // 使能全局中断
}

/**
 * @brief  重写标准库输出函数 putchar
 * @param  ch 待发送字符
 * @return 发送成功的字符
 */
extern char putchar(char ch) {
	SBUF = ch;
	while(TI == 0);
	TI = 0;
	return ch;
}