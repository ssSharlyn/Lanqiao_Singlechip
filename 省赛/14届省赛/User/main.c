#include <STC15F2K60S2.H>
#include <Init.h>
#include <Key.h>
#include <Seg.h>
#include <LED.h>
#include <DS1302.h>
#include <OneWire.h>
#include <I2C.h>
#include <string.h>
#include <Uart.h>
#include <stdio.h>

/* 按键 */
unsigned char Key_Slow;
unsigned char Key_Val, Key_Down, Key_Up, Key_Old;
unsigned int Timer_2000ms; //长按计时器
bit Tim2_Flag; //长按计时标志

/* 数码管 */
unsigned char Seg_Slow;
unsigned char Seg_Pos;
unsigned char xdata Seg_Buf[8] ={10,10,10,10,10,10,10,10};
unsigned char xdata Seg_Point[8] = {0,0,0,0,0,0,0,0};
unsigned char Seg_Mode; //数码管显示模式，0-时间界面，1-回显界面，2-参数界面，3-温湿度界面
unsigned char Retn_Mode; //回显子界面，0-温度回显，1-湿度回显，2-时间回显
unsigned char ucRTC[3] = {13,03,05}; //时间
unsigned char idata t, t_last, humi, humi_last; //温度、上次温度，湿度、上次湿度
unsigned char idata t_max, humi_max; //最大温度、湿度
unsigned int idata t_sum, humi_sum; //温度、湿度总值
float idata t_ave, humi_ave; //平均温度、平均湿度
unsigned char idata Coll_Count; //采集触发次数
float RD1 = 5; //光敏电阻分压
unsigned int Timer_3000ms; //采集间隔计时器
bit Tim1_Flag; //采集间隔计时标志
unsigned char RTC_Recd[2]; //采集时间记录
unsigned char Seg_Mode_Recd; //当前界面记录
unsigned int Timer_1000ms; //频率计时器
unsigned int Freq; //频率
unsigned char t_set = 30; //温度参数

/* LED */
unsigned char xdata ucLED[8] = {0,0,0,0,0,0,0,0};
bit Blink_Flag; //LED闪烁标志
unsigned char Timer_100ms; //LED闪烁计时器

/* 串口 */
unsigned char Uart_Slow;
unsigned char Uart_Rec[10];

void Key_Proc()
{
	if(Key_Slow) return;
	Key_Slow = 1;
	
	Key_Val = Key_Read();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
	switch(Key_Down)
	{
		case 4: //界面切换按键
			if(Seg_Mode != 3) //处于非温湿度界面时
			  if(++Seg_Mode == 3) 
					Seg_Mode = 0;
		break;
		
		case 5: //回显子界面切换按键
			if(Seg_Mode == 1) //处于回显界面时
				if(++Retn_Mode == 3)
					Retn_Mode = 0;
		break;
				
		case 9: //+按键
			if(Seg_Mode == 2) //处于参数界面时
			  if(--t_set == 255) //参数越界处理
					t_set = 99;
		break;
		
		case 8: //-按键
			if(Seg_Mode == 2) //处于参数界面时
				if(++t_set == 100) //参数越界处理
					t_set = 0;
		break;
	}
	
	if(Seg_Mode == 1 && Retn_Mode == 2) //处于时间回显界面时
	{
		if(Key_Down == 9)
			Tim2_Flag = 1; //拉高标志位，开始计时
		if(Key_Up == 9 && Timer_2000ms > 2000) //长按2s后松开，清除所有数据
		{
			Tim2_Flag = 0;
			Coll_Count = 0;
			t_sum = humi_sum = t_max = humi_max = t = t_last = humi = humi_last = t_ave = humi_ave = 0;
		}
	}
}

void Seg_Proc()
{
	if(Seg_Slow) return;
	Seg_Slow = 1;
	
	Read_RTC(ucRTC); //时间读取
	RD1 = AD_Read(0x41) / 51.0; //电压读取
	Convert_T(); //开始温度转换
	
	if(RD1 <= 3 && Tim1_Flag == 0) //暗状态触发采集，且3秒内不可再触发
	{
		Tim1_Flag = 1; //拉高标志位，计时三秒开始
    t = Read_T(); //采集温度
		if(Freq >= 200 && Freq <= 2000) //频率有效时才记录
		{
			memcpy(RTC_Recd,ucRTC,2); //获取最近触发时间
		  Coll_Count++; // 采集次数加一
			humi = (unsigned char)(2.0 / 45 * Freq + 10.0 / 9); //采集湿度
			t_sum += t; //计算温度总值
		  humi_sum += humi; //计算湿度总值
		  t_ave = t_sum / (float)Coll_Count; //计算平均温度
		  humi_ave = humi_sum / (float)Coll_Count; //计算平均湿度
		  t_max = (t > t_last) ? t : t_last; //获取最大温度
	    humi_max = (humi > humi_last) ? humi : humi_last; //获取最大湿度
		}
		else //无效数据
			humi = 0;
		
		if(Seg_Mode != 3) //连续采集时保护界面记录值
			Seg_Mode_Recd = Seg_Mode; //记录当前界面
	    
		Seg_Mode = 3; //切换到温湿度界面
		
		ucLED[5] = (t > t_last && humi > humi_last && Coll_Count > 1); //温湿度高于上次值时，L6点亮
	  t_last = t; //记录上次温度
	  humi_last = humi; //记录上次湿度
	}

	switch(Seg_Mode)
	{
		case 0: //时间界面
		  Seg_Buf[2] = Seg_Buf[5] = 11;
		  Seg_Buf[0] = ucRTC[0] / 10;
		  Seg_Buf[1] = ucRTC[0] % 10;
		  Seg_Buf[3] = ucRTC[1] / 10;
		  Seg_Buf[4] = ucRTC[1] % 10;
		  Seg_Buf[6] = ucRTC[2] / 10;
		  Seg_Buf[7] = ucRTC[2] % 10;
		  Retn_Mode = 0; //重置回显界面
		break;
		
		case 1: //回显界面
			if(Retn_Mode == 0) //温度回显
			{
				Seg_Buf[0] = 12;
				Seg_Buf[1] = 10;
				if(Coll_Count != 0) //触发次数不为0才显示
				{
					Seg_Buf[4] = 11;
					Seg_Buf[2] = t_max / 10;
					Seg_Buf[3] = t_max % 10;
					Seg_Buf[5] = (unsigned char)t_ave / 10;
					Seg_Buf[6] = (unsigned char)t_ave % 10;
					Seg_Buf[7] = (unsigned char)(t_ave * 10) % 10;
					Seg_Point[6] = 1;
				}
				else
				{
					Seg_Buf[4] = 10;
					Seg_Buf[2] = 10;
					Seg_Buf[3] = 10;
					Seg_Buf[5] = 10;
					Seg_Buf[6] = 10;
					Seg_Buf[7] = 10;
					Seg_Point[6] = 0;
				}
			}		
			if(Retn_Mode == 1) //湿度回显
			{
				Seg_Buf[0] = 13;
				Seg_Buf[1] = 10;
				
				if(Coll_Count != 0) //触发次数不为0才显示
				{
					Seg_Buf[4] = 11;
					Seg_Buf[2] = humi_max / 10;
					Seg_Buf[3] = humi_max % 10;
					Seg_Buf[5] = (unsigned char)humi_ave / 10;
					Seg_Buf[6] = (unsigned char)humi_ave % 10;
					Seg_Buf[7] = (unsigned char)(humi_ave * 10) % 10;
					Seg_Point[6] = 1;
				}
				else
				{
					Seg_Buf[4] = 10;
					Seg_Buf[2] = 10;
					Seg_Buf[3] = 10;
					Seg_Buf[5] = 10;
					Seg_Buf[6] = 10;
					Seg_Buf[7] = 10;
					Seg_Point[6] = 0;
				}
			}
			if(Retn_Mode == 2) //时间回显
			{
				Seg_Buf[0] = 14;
				Seg_Buf[1] = Coll_Count / 10;
				Seg_Buf[2] = Coll_Count % 10;
				Seg_Point[6] = 0;
				if(Coll_Count !=0 ) //触发次数不为0才显示
				{
					Seg_Buf[5] = 11;
					Seg_Buf[3] = RTC_Recd[0] / 10;
				  Seg_Buf[4] = RTC_Recd[0] % 10;
				  Seg_Buf[6] = RTC_Recd[1] / 10;
				  Seg_Buf[7] = RTC_Recd[1] % 10;
				}
				else
				{
					Seg_Buf[3] = 10;
					Seg_Buf[4] = 10;
					Seg_Buf[5] = 10;
					Seg_Buf[6] = 10;
					Seg_Buf[7] = 10;
				}
			}
		break;
			
		case 2: //参数界面
		  Seg_Buf[0] = 15;
		  Seg_Buf[1] = 10;
		  Seg_Buf[2] = 10;
		  Seg_Buf[3] = 10;
		  Seg_Buf[4] = 10;
		  Seg_Buf[5] = 10;
		  Seg_Buf[6] = t_set / 10;
		  Seg_Buf[7] = t_set % 10;
		  Seg_Point[6] = 0;
		break;
		
		case 3: //温湿度界面
		  Seg_Buf[0] = 16;
		  Seg_Buf[1] = Seg_Buf[2] = 10;
		  Seg_Point[6]= 0;
		  Seg_Buf[5] = 11;	
		  Seg_Buf[3] = t / 10; 
		  Seg_Buf[4] = t % 10; 
		  if(humi)
		  {
			  Seg_Buf[6] = humi / 10; 
		    Seg_Buf[7] = humi % 10; 
		  }
		  else //无效数据显示AA
		  {
			  Seg_Buf[6] = 17; 
		    Seg_Buf[7] = 17; 
		  }
			if(Tim1_Flag == 0) //3秒后返回上个界面
		    Seg_Mode = Seg_Mode_Recd; 
	  break;
	}
}

void LED_Proc()
{
	ucLED[0] = (Seg_Mode == 0);//互斥点亮L1~3
	ucLED[1] = (Seg_Mode == 1);
	ucLED[2] = (Seg_Mode == 3);
	ucLED[3] = Blink_Flag; //采集温度大于温度参数时，L4闪烁
	ucLED[4] = (!humi && t); //采集到无效湿度数据时，L5点亮
}

void Uart_Proc()
{
	if(Uart_Slow) return;
	Uart_Slow = 1;
	
	printf("频率 = %u",Freq);
}
	
void Timer0_Init()		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}

void Timer1_Init()		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;
	EA = 1;
}

void Timer1_Server() interrupt 3
{
	if(++Key_Slow == 10) Key_Slow = 0;
	if(++Seg_Slow == 250) Seg_Slow = 0;
	if(++Seg_Pos == 8) Seg_Pos = 0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	LED_Disp(Seg_Pos,ucLED[Seg_Pos]);
	
	if(++Uart_Slow == 250) Uart_Slow = 0;
	
	if(++Timer_1000ms == 1000)
	{
		Timer_1000ms = 0;
		TR0 = 0;
		Freq = (TH0 << 8) | TL0;
		TH0 = TL0 = 0;
		TR0 = 1;
	}
	
	if(Tim1_Flag)
		if(++Timer_3000ms == 3000)
		{
			Timer_3000ms = 0;
			Tim1_Flag = 0;
		}
	
	if(Tim2_Flag)
	{
		if(++Timer_2000ms == 2500)
			Timer_2000ms = 2500;
	}
	else
	  Timer_2000ms = 0;
	
	if(t > t_set) //采集温度大于温度参数时，L4闪烁
	{
		if(++Timer_100ms == 100)
		{
			Timer_100ms = 0;
			Blink_Flag ^= 1;
		}
	}
	else
	{
		Timer_100ms = 0;
		Blink_Flag = 0;
	}
}

void main()
{
	Sys_Init();
	Timer0_Init();
	Timer1_Init();
	Uart_Init();
	Convert_T();
	AD_Read(0x41);
	Write_RTC(ucRTC);
	Read_RTC(ucRTC);
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		LED_Proc();
//	  Uart_Proc();
	}
}