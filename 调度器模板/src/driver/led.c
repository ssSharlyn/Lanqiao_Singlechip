#include "led.h"

idata unsigned char temp_1 = 0x00; // 当前LED状态
idata unsigned char temp_1_old = 0xff; // 上一次LED状态

/**
 * @brief LED扫描函数
 * @param ucled 指向包含8个元素的数组首地址，用于记录8个LED的状态
 * @note  仅在检测到LED状态改变时，才更新硬件
 */
void led_disp(unsigned char *ucled) {
	unsigned char temp;
	
	temp_1 = 0x00; // 初始化LED状态
	temp_1 = ucled[0] << 0| ucled[1] << 1| ucled[2] << 2| ucled[3] << 3| 
	         ucled[4] << 4| ucled[5] << 5| ucled[6] << 6| ucled[7] << 7; // 记录当前LED状态
	
	// 当检测到LED状态发生变化时，更改LED状态
	if(temp_1_old != temp_1) {
	  P0 = ~temp_1; // 对状态数据取反，因为是低电平点亮
	  temp = P2 & 0x1f | 0x80; // 选通锁存器Y4通道
	  P2 = temp; // 打开锁存器，传送数据
	  temp = P2 & 0x1f; 
	  P2 = temp; // 锁定锁存器
		temp_1_old = temp_1; // 更新上一次LED状态
	}
}

/**
 * @brief LED熄灭函数
 * @note  一次性熄灭所有LED
 */
void led_off(unsigned char *ucled) {
	unsigned char temp;
	
	P0 = 0xff; // 准备数据
	temp = P2 & 0x1f | 0x80; // 选通锁存器Y4通道
	P2 = temp; // 打开锁存器，传送数据
	temp = P2 & 0x1f; 
	P2 = temp; // 锁定锁存器
	
	temp_1_old = 0x00; // 更新上一次LED状态
}

idata unsigned char temp_2 = 0x00; // 当前外设状态
idata unsigned char temp_2_old = 0xff; // 上一次外设状态

/**
 * @brief 继电器驱动函数
 * @param en 继电器状态控制位 1-吸合 0-断开  
 * @note  控制继电器的状态
 */
void relay(bit en) {
	unsigned char temp;
	
	if(en) {
		temp_2 |= 0x10; // 继电器吸合 
	}
	else {
		temp_2 &= ~0x10; // 继电器断开 
	}
	
	// 当检测到外设状态发生变化时，更改外设状态
	if(temp_2_old != temp_2)
	{
		P0 = temp_2; // 准备数据
	  temp = P2 & 0x1f | 0xa0; // 选通锁存器Y5通道
	  P2 = temp; // 打开锁存器，传送数据
	  temp = P2 & 0x1f; 
	  P2 = temp; // 锁定锁存器
		
		temp_2_old = temp_2; // 更新上一次外设状态
	}
}

/**
 * @brief 电机驱动函数
 * @param en 电机状态控制位 1-开 0-关  
 * @note  控制电机的状态
 */
void motor(bit en) {
	unsigned char temp;
	
	if(en) {
		temp_2 |= 0x20; // 打开电机 
	}
	else {
		temp_2 &= ~0x20; // 关闭电机 
	}
	
	// 当检测到外设状态发生变化时，更改外设状态
	if(temp_2_old != temp_2)
	{
		P0 = temp_2; // 准备数据
	  temp = P2 & 0x1f | 0xa0; // 选通锁存器Y5通道
	  P2 = temp; // 打开锁存器，传送数据
	  temp = P2 & 0x1f; 
	  P2 = temp; // 锁定锁存器
		
		temp_2_old = temp_2; // 更新上一次外设状态
	}
}

/**
 * @brief 蜂鸣器驱动函数
 * @param en 蜂鸣器状态控制位 1-开 0-关  
 * @note  控制蜂鸣器的状态
 */
void buzzer(bit en) {
	unsigned char temp;
	
	if(en) {
		temp_2 |= 0x40; // 打开蜂鸣器
	}
	else {
		temp_2 &= ~0x40; // 关闭蜂鸣器 
	}
	
	// 当检测到外设状态发生变化时，更改外设状态
	if(temp_2_old != temp_2)
	{
		P0 = temp_2; // 准备数据
	  temp = P2 & 0x1f | 0xa0; // 选通锁存器Y5通道
	  P2 = temp; // 打开锁存器，传送数据
	  temp = P2 & 0x1f; 
	  P2 = temp; // 锁定锁存器
		
		temp_2_old = temp_2; // 更新上一次外设状态
	}
}