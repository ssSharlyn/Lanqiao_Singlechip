#include <STC15F2K60S2.H>
#include <intrins.h>

void da_write(unsigned char dat);
unsigned char ad_read(unsigned char addr);
void eeprom_write(unsigned char *str, unsigned char addr, unsigned char num);
void eeprom_read(unsigned char *str, unsigned char addr, unsigned char num);
