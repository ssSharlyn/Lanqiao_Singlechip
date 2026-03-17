#include <STC15F2K60S2.H>

unsigned char ad_read(unsigned char addr);
void da_write(unsigned char dat);
void eeprom_read(unsigned char *str, addr, num);
void eeprom_write(unsigned char *str, addr, num);
