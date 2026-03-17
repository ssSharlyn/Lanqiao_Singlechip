#include "system.h"

void sys_init(void)
{ 
	unsigned char temp;
	
	P0 = 0xff;
	temp = P2 & 0x1f;
	temp = temp | 0xe0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;	
	
	P0 = 0x00;
	temp = P2 & 0x1f;
	temp = temp | 0xa0;
	P2 = temp;
	temp = P2 & 0x1f;
	P2 = temp;		
}