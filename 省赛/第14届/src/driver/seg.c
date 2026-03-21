#include "seg.h"

unsigned char seg_dula[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0xBF,0xC6,0x89,0x8E,0x8C,0x86,0x88}; // 数码管段码数组

/**
 * @brief 数码管扫描函数
 * @param wela 位选坐标 
 * @param dula 段选索引
 * @param 小数点控制位 point 1-显示 0-隐藏
 */
void seg_disp(unsigned char wela, unsigned char dula, bit point) {
	unsigned char temp;
	
	// --- 消影 ---
	P0 = 0xff;
	temp = P2 & 0x1f | 0xe0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;	
	
		
	// --- 位选 --- 
	P0 = 0x01 << wela;
	temp = P2 & 0x1f | 0xc0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;	
	
	// --- 段选 ---
	P0 = seg_dula[dula];
	if(point)
		P0 &= 0x7f; // 显示小数点	
	temp = P2 & 0x1f | 0xe0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;		
}