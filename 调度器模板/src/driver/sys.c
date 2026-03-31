#include "sys.h"

/**
 * @brief 系统硬件初始化函数
 * @note  上电时强制关闭数码管、蜂鸣器与继电器等外设
 */	
void sys_init(void) {
	unsigned char temp;
	
	// --- 熄灭数码管显示 ---
	P0 = 0xff; // 准备数据(高电平)
	temp = P2 & 0x1f | 0xe0; // 选通锁存器Y7通道
	P2 = temp; // 打开锁存器，传送数据，熄灭数码管
	temp = P2 & 0x1f; 
	P2 = temp; // 锁定锁存器
	
	// --- 关闭继电器与蜂鸣器 ---
	P0 = 0x00; // 准备数据(低电平)
	temp = P2 & 0x1f | 0xa0; // 选通锁存器Y5通道
	P2 = temp; // 打开锁存器，传送数据，关闭蜂鸣器等外设
	temp = P2 & 0x1f; 
	P2 = temp; // 锁定锁存器
}