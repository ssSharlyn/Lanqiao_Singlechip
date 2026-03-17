#include <OneWire.h>

sbit DQ = P1^4;

void Delay_OneWire(unsigned int t)  
{
	unsigned char i;
	while(t--){
		for(i=0;i<12;i++);
	}
}

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

unsigned char Read_DS18B20()
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

bit Init_DS18B20()
{
  	bit Initflag = 0;
  	
  	DQ = 1;
  	Delay_OneWire(12);
  	DQ = 0;
  	Delay_OneWire(80);
  	DQ = 1;
  	Delay_OneWire(10); 
    Initflag = DQ;     
  	Delay_OneWire(5);
  
  	return Initflag;
}

void Convert_T()
{
	Init_DS18B20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
}

float Read_T()
{
	unsigned char Low, High;
	
	Init_DS18B20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	Low = Read_DS18B20();
	High = Read_DS18B20();
	
	return ((High << 8) | Low) / 16.0;
}