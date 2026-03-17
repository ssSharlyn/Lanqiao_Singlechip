#include <STC15F2K60S2.H>
#include "system.h"
#include "key.h"
#include "seg.h"
#include "led.h"
#include "uart.h"
#include "ultrasonic.h"
#include "iic.h"
#include "onewire.h"
#include "ds1302.h"

#include <stdio.h>
#include <string.h>
#include <intrins.h>

/** scheduler **/
idata unsigned long int uwTick = 0; //系统计时

/** key **/
idata unsigned char key_val, key_down, key_up, key_old;

/** seg **/
pdata unsigned char seg_buf[8] = {10,10,10,10,10,10,10,10};
idata unsigned char seg_pos;

/** led **/
pdata unsigned char ucled[8] = {0,0,0,0,0,0,0,0};
idata unsigned char pwm_period = 10; //pwm调光周期
idata unsigned char pwm_duty = 1; //pwm调光占空比

/** ad_da **/
idata unsigned int ad_rd1_x100, ad_rb2_x100; 

/** temperature **/
idata unsigned int temperature_x100;

/** RTC **/
pdata unsigned char ucRTC[3] = {11,23,59};

/** distance **/
idata unsigned char dist;

/** freq **/
idata unsigned int freq;
idata unsigned int timer_1s;

/** Uart **/
pdata unsigned char uart_buf[10];  //串口接收数据缓存区
idata unsigned char uart_rx_index; //串口接收数据索引
idata unsigned char uart_tick;  //串口接收时间
idata bit uart_rx_flag;            //串口接收标志

/** EEPROM **/
idata unsigned char eep_lock = 16;
idata unsigned int eep_read, eep_write;
idata unsigned char eep_low, eep_high;


void Delay(unsigned int time_ms)	//@12.000MHz
{
	unsigned char data i, j;
	
	while(time_ms--)
	{
		i = 12;
	  j = 169;
		do
	  {
		  while (--j);
	  } while (--i);
	}
}


/*** key ***/
void key_proc(void)
{
	key_val = key_read();
	key_down = key_val & (key_val ^ key_old);
	key_up = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 4)
	{
	  eep_write = freq;
	  eeprom_write(&eep_write, 0, 2);
	  eeprom_write(&eep_lock, 16, 1);
	}
}

/*** seg ***/
void seg_proc(void)
{
//	seg_buf[0] = ad_rd1_x100 / 100 + '.'; 
//	seg_buf[1] = ad_rd1_x100 / 10 % 10; 
//	seg_buf[2] = ad_rd1_x100 % 10; 
//	
//	seg_buf[5] = ad_rb2_x100 / 100 + '.'; 
//	seg_buf[6] = ad_rb2_x100 / 10 % 10; 
//	seg_buf[7] = ad_rb2_x100 % 10; 
	
//	seg_buf[0] = temperature_x100 / 1000; 
//	seg_buf[1] = temperature_x100 / 100 % 10 + '.'; 
//	seg_buf[2] = temperature_x100 / 10 % 10; 
//	seg_buf[3] = temperature_x100 % 10;	

//	seg_buf[2] = seg_buf[5] = 11;
//  seg_buf[0] = ucRTC[0] / 10;
//	seg_buf[1] = ucRTC[0] % 10;
//	seg_buf[3] = ucRTC[1] / 10;
//	seg_buf[4] = ucRTC[1] % 10;
//	seg_buf[6] = ucRTC[2] / 10;
//	seg_buf[7] = ucRTC[2] % 10;
	
//	seg_buf[0] = dist / 100;
//	seg_buf[1] = dist / 10 % 10; 
//	seg_buf[2] = dist % 10;

//  seg_buf[0] = freq / 10000;
//	seg_buf[1] = freq / 1000 % 10;
//	seg_buf[2] = freq / 100 % 10;
//	seg_buf[3] = freq / 10 % 10;
//	seg_buf[4] = freq % 10;
	
	seg_buf[0] = freq / 1000;
	seg_buf[1] = freq / 100 % 10;
	seg_buf[2] = freq / 10 % 10;
	seg_buf[3] = freq % 10;
	
	seg_buf[4] = eep_read / 1000;
	seg_buf[5] = eep_read / 100 % 10;
	seg_buf[6] = eep_read / 10 % 10;
	seg_buf[7] = eep_read % 10;
	
	printf("%u", eep_read);
}

/*** led ***/
void led_proc(void)
{
//	led_disp(ucled);
}

/*** ad_da ***/
void ad_da_proc(void)
{
	//多通道ad转换反读
	ad_rd1_x100 = ad_read(0x43) * 100 / 51;
	ad_rb2_x100 = ad_read(0x41) * 100 / 51;
	da_write(2 * 51);
}

/*** temperature ***/
void get_temperature(void)
{
	temperature_x100 = read_temperature() * 100;
}

/*** RTC ***/
void get_time(void)
{
	Read_RTC(ucRTC);    
}

/*** distance ***/
void get_dist(void)
{
	dist = ultrasonic_dat();
}

/*** Timer 0 - NE555 ***/
void Timer0_Init(void)		//100微秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;     //设置计数器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}


/*** Timer 1 ***/
void Timer1_Init(void)		//1毫秒@12.000MHz
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

void timer1_server(void) interrupt 3
{
	//系统时钟
	uwTick++;
	//seg
	seg_pos = (++seg_pos) % 8;
	if(seg_buf[seg_pos] > 20) seg_disp(seg_pos, seg_buf[seg_pos] - '.', 1);
	else seg_disp(seg_pos, seg_buf[seg_pos], 0);
	//led - pwm
	if(++pwm_period >= 10) 
		pwm_period = 0;
	if(pwm_period < pwm_duty)
	{
		led_disp(ucled);
	}
	else
	{
		led_off();
	}
	//freq
	if(++timer_1s >= 1000)
	{
		timer_1s = 0;
		freq = (TH0 << 8) | TL0;
		TL0 = TH0 = 0;
	}
	//uart
	if(uart_rx_flag)
		uart_tick++;
}

/*** Uart 1 ***/
void uart_proc(void)
{
	unsigned char x, y;
	
	//无数据返回
	if (uart_rx_index == 0) return; 
	
	//超时解析
	if(uart_tick >= 10)
	{
		uart_rx_flag = 0;
		uart_tick = 0;
		//串口数据解析
		if(sscanf(uart_buf, "%bu, %bu", &x, &y) == 2)
			printf("x = %bu, y = %bu", x, y);
		else
			printf("error!");
		
		memset(uart_buf, 0, uart_rx_index);
		uart_rx_index = 0;
	}	  
}

void uart1_server() interrupt 4
{
	if(RI)
	{
		uart_rx_flag = 1;
		uart_tick = 0;
		uart_buf[uart_rx_index++] = SBUF;
		RI = 0;
		//缓存区溢出处理
		if(uart_rx_index > 10)
	  {
		  uart_rx_index = 0;
		  memset(uart_buf, 0, 10);
	  }
	}
}

/*< 调度器任务结构体定义 >*/
typedef struct
{
	void (*task_func)(void); //任务函数
	unsigned long int rate_ms; //任务周期
	unsigned long int last_ms;  //上次执行的时间
}task_t;

/*< 调度器任务列表数组 >*/
idata task_t scheduler_task[] = 
{
	{key_proc, 10, 0},
	{led_proc, 1, 0},
	{seg_proc, 200, 0},
	{ad_da_proc, 160, 0},
  {get_temperature, 200, 0},
	{get_time, 300, 0},
	{get_dist, 100, 0},
	{uart_proc, 10, 0}
};

/*< 调度器初始化函数 >*/
idata unsigned char task_num;

void scheduler_init(void)
{
	task_num = sizeof(scheduler_task) / sizeof(task_t);
}

/*< 调度器运行函数 >*/
void scheduler_run(void)
{
	unsigned char i;
	for(i = 0; i < task_num; i++)
	{
		unsigned long int now_time = uwTick; //获取当前时间
	  if(now_time >= scheduler_task[i].rate_ms + scheduler_task[i].last_ms)
		{
			scheduler_task[i].last_ms = now_time; // 更新任务上次运行时间
			scheduler_task[i].task_func();        // 执行任务
		}			
	}
}

void main(void)
{
	unsigned char eep_temp;
	
	sys_init();
	eeprom_read(&eep_temp, 16, 1);	
	if(eep_temp == eep_lock)
	{
		eeprom_read(&eep_high, 0, 1);
		eeprom_read(&eep_low, 1, 1);
		eep_read = (eep_high << 8) | eep_low;
	}

	read_temperature();
	Delay(750);
	Set_RTC(ucRTC);
	Uart1_Init();
	Timer0_Init();
	scheduler_init();	
	Timer1_Init();
	while(1)
	{
		scheduler_run();
	}
}