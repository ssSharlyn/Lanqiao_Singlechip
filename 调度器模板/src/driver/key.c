#include "key.h"

/** 
 * @brief  键盘扫描函数
 * @note   使用独立按键时注释掉后三行按键代码
 * @return 按键键码值(4-19) 无按键按下时返回0
 */
unsigned char key_read(void) {
  unsigned char temp = 0;

  if(P30 == 0) return;
  P44 = 0; P42 = 1; P35 = 1; P34 = 1; // 选中第一列	
	if(P30 == 0) temp = 7;
	if(P31 == 0) temp = 6;
	if(P32 == 0) temp = 5;
	if(P33 == 0) temp = 4;
	P44 = 1;P42 = 1;P35 = 1;P34 = 1;
	
	if(P30 == 0) return;
  P44 = 1; P42 = 0; P35 = 1; P34 = 1; // 选中第二列		
	if(P30 == 0) temp = 11;
	if(P31 == 0) temp = 10;
	if(P32 == 0) temp = 9;
	if(P33 == 0) temp = 8;
	P44 = 1;P42 = 1;P35 = 1;P34 = 1;
	
	if(P30 == 0) return;
  P44 = 1; P42 = 1; P35 = 0; P34 = 1; // 选中第三列		
	if(P30 == 0) temp = 15;
	if(P31 == 0) temp = 14;
	if(P32 == 0) temp = 13;
	if(P33 == 0) temp = 12;
	P44 = 1;P42 = 1;P35 = 1;P34 = 1;
	
	if(P30 == 0) return;
  P44 = 1; P42 = 1; P35 = 1; P34 = 0; // 选中第四列		
	if(P30 == 0) temp = 19;
	if(P31 == 0) temp = 18;
	if(P32 == 0) temp = 17;
	if(P33 == 0) temp = 16;
	P44 = 1;P42 = 1;P35 = 1;P34 = 1;
	
	P3 |= 0xef; // 恢复 P3 口高电平状态，准备下一次扫描
	
	return temp;
}