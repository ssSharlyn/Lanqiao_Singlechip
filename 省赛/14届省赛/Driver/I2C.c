#include <I2C.h>
#include <intrins.h>

#define DELAY_TIME	10

sbit SCL = P2^0;
sbit SDA = P2^1;

static void I2C_Delay(unsigned char n)
{
    do
    {
        _nop_();_nop_();_nop_();_nop_();_nop_();
        _nop_();_nop_();_nop_();_nop_();_nop_();
        _nop_();_nop_();_nop_();_nop_();_nop_();		
    }
    while(n--);      	
}

void I2C_Start()
{
    SDA = 1;
    SCL = 1;
	I2C_Delay(DELAY_TIME);
    SDA = 0;
	I2C_Delay(DELAY_TIME);
    SCL = 0;    
}

void I2C_Stop()
{
    SDA = 0;
    SCL = 1;
	I2C_Delay(DELAY_TIME);
    SDA = 1;
	I2C_Delay(DELAY_TIME);
}

void I2C_SendByte(unsigned char byt)
{
    unsigned char i;
	
    for(i=0; i<8; i++){
        SCL = 0;
		I2C_Delay(DELAY_TIME);
        if(byt & 0x80){
            SDA = 1;
        }
        else{
            SDA = 0;
        }
		I2C_Delay(DELAY_TIME);
        SCL = 1;
        byt <<= 1;
		I2C_Delay(DELAY_TIME);
    }
	
    SCL = 0;  
}

unsigned char I2C_RecByte()
{
	unsigned char da;
	unsigned char i;
	for(i=0;i<8;i++){   
		SCL = 1;
		I2C_Delay(DELAY_TIME);
		da <<= 1;
		if(SDA) 
			da |= 0x01;
		SCL = 0;
		I2C_Delay(DELAY_TIME);
	}
	return da;    
}

unsigned char I2C_WaitAck()
{
	unsigned char ackbit;
	
    SCL = 1;
	I2C_Delay(DELAY_TIME);
    ackbit = SDA; 
    SCL = 0;
	I2C_Delay(DELAY_TIME);
	
	return ackbit;
}

void I2C_SendAck(unsigned char ackbit)
{
    SCL = 0;
    SDA = ackbit; 
	I2C_Delay(DELAY_TIME);
    SCL = 1;
	I2C_Delay(DELAY_TIME);
    SCL = 0; 
	SDA = 1;
	I2C_Delay(DELAY_TIME);
}

unsigned char AD_Read(unsigned char addr)
{
	unsigned char temp;
	
	I2C_Start();
	I2C_SendByte(0x90);
	I2C_WaitAck();
	I2C_SendByte(addr);
	I2C_WaitAck();
	I2C_Start();
	I2C_SendByte(0x91);
	I2C_WaitAck();
	temp = I2C_RecByte();
	I2C_SendAck(1);
	I2C_Stop();
	
	return temp;
}

void DA_Write(unsigned char addr)
{
	I2C_Start();
	I2C_SendByte(0x90);
	I2C_WaitAck();
	I2C_SendByte(0x41);
	I2C_WaitAck();
	I2C_SendByte(addr);
	I2C_WaitAck();
	I2C_Stop();
}