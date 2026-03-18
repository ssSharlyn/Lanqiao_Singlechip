#include <STC15F2K60S2.H>
#include <string.h>
#include "sys.h"
#include "seg.h"
#include "key.h"
#include "led.h"
#include "uart.h"
#include "ultra.h"
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"

// ** 参数定义区 ** //

/* scheduler */
idata unsigned long int uwTick; // 系统计时
idata unsigned char task_num; // 调度器任务函数数量

/* key */ 
idata unsigned char key_val, key_down, key_up, key_old; // 按键状态，瞬时状态、下降沿、上升沿、上次状态

/* seg */
pdata unsigned char seg_buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管状态缓存数组，初始为熄灭
idata unsigned char seg_pos; // 数码管位置索引
idata unsigned char seg_mode; // 数码管显示模式，0-频率

/* LED */
pdata unsigned char ucled[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // LED状态缓存数组，初始为熄灭

/* Ultrasonic */
idata unsigned int dist; // 超声波所测距离值(单位: cm)

/* RTC */
pdata unsigned char ucRTC[3] = {23, 59, 55}; // 时钟数据缓存数组

/* AD/DA */
idata unsigned int volt_1_x100, volt_2_x100; // 光敏电阻/电位器电压值x100(节省存储空间和简化计算)

/* Temperature */
idata unsigned int temperature_x100; // 温度值x100(节省存储空间和简化计算)

/* Freq */
idata unsigned int freq; // 频率值
idata unsigned int timer_1s; // 1s计时

/* UART */
//*** 注：pdata数据必须初始化值
pdata unsigned char uart_rx_buf[10] = {0}; // 串口数据接收缓冲区
idata unsigned char uart_rx_index; // 串口数据接收索引
idata bit uart_rx_flag; // 串口数据接收完成标志位
idata unsigned char uart_tick; // 串口接收超时计时

/* other parameters */



// ** 函数定义区 ** //

/**
 * @brief 按键状态机任务函数
 * @note  利用位运算进行边沿检测
 */
void key_task(void) {
	key_val = key_read(); // 获取当前按键状态
	key_down = key_val & (key_val ^ key_old); // 获取按键下降沿状态
	key_up = ~key_val & (key_val ^ key_old);  // 获取按键上升沿状态
	key_old = key_val; // 更新上一次按键状态
	
	switch(key_down) {
		// --- 切换数码管显示模式 --- 
		case 4:
			seg_mode = (seg_mode + 1) % 5; 
		  break;
		
	}
}

/**
 * @brief 数码管显示任务函数
 * @note  需要显示小数点时，在值后加上'.'
 */
void seg_task(void) {
  
	switch(seg_mode) {
		// --- 模式0，显示频率值 ----
		case 0:
	    seg_buf[0] = seg_buf[1] = seg_buf[2] = 10; // 熄灭不需要的数码管
	    seg_buf[3] = freq >= 10000 ? freq / 10000 % 10 : 10; // 高位有数值时才显示
	    seg_buf[4] = freq >= 1000 ? freq / 1000 % 10 : 10;
	    seg_buf[5] = freq >= 100 ? freq / 100 % 10 : 10;
	    seg_buf[6] = freq >= 10 ? freq / 10 % 10 : 10; 
	    seg_buf[7] = freq % 10; // 个位始终显示
		  break;
		// --- 模式1，显示距离值 ----
		case 1:
	    seg_buf[0] = seg_buf[1] = seg_buf[2] = seg_buf[3] = seg_buf[4] = 10; // 熄灭不需要的数码管
	    seg_buf[5] = dist >= 100 ? dist / 100 % 10 : 10; // 高位有数值时才显示
	    seg_buf[6] = dist >= 10 ? dist / 10 % 10 : 10; 
	    seg_buf[7] = dist % 10; // 个位始终显示
		  break; 
		// --- 模式2，显示时间 ----
		case 2:
			seg_buf[2] = seg_buf[5] = 11; // 显示短横线"-"
	    seg_buf[0] = ucRTC[0] / 10; // 显示时
		  seg_buf[1] = ucRTC[0] % 10;
		  seg_buf[3] = ucRTC[1] / 10; // 显示分
		  seg_buf[4] = ucRTC[1] % 10;
		  seg_buf[6] = ucRTC[2] / 10; // 显示秒
		  seg_buf[7] = ucRTC[2] % 10;
		  break; 
		// --- 模式3，显示电压值 ----
		case 3:
			seg_buf[0] = seg_buf[4] = 10; // 熄灭不需要的数码管
		  // 光敏电阻电压值 
	    seg_buf[1] = volt_1_x100 / 100 % 10 + '.'; 
      seg_buf[2] = volt_1_x100 / 10 % 10; 
	    seg_buf[3] = volt_1_x100 % 10; 
		  // 电位器电压值 
	    seg_buf[5] = volt_2_x100 / 100 % 10 + '.'; 
      seg_buf[6] = volt_2_x100 / 10 % 10; 
	    seg_buf[7] = volt_2_x100 % 10; 
		  break; 
		// --- 模式4，显示温度值 ----
		case 4:
			seg_buf[0] = seg_buf[1] = seg_buf[2] = seg_buf[3] = 10; // 熄灭不需要的数码管
	    // 小数点前部分
		  seg_buf[4] = temperature_x100 >= 1000 ? temperature_x100 / 1000 % 10 : 10;
	    seg_buf[5] = temperature_x100 / 100 % 10 + '.'; // 个位始终显示
	  	// 小数点后部分(始终显示）
	    seg_buf[6] = temperature_x100 / 10 % 10; 
	    seg_buf[7] = temperature_x100 % 10; 
		  break; 
	}
}

/**
 * @brief LED任务函数
 */
void led_task(void) {
  led_disp(ucled);	
}

/**
 * @brief 串口数据解析任务函数
 */
void uart_task(void)
{
	if(uart_rx_index == 0) return;
	
	if(uart_tick >= 10)
	{
		uart_rx_flag = 0;
		uart_tick = 0;

		//数据解析
		//...

		memset(uart_rx_buf, 0, uart_rx_index); 
		uart_rx_index = 0;
	}
}

/**
 * @brief 串口中断服务函数
 */
void uart_server(void) interrupt 4 
{
	if(RI)
	{
		uart_rx_flag = 1;
		uart_tick = 0;
		uart_rx_buf[uart_rx_index++] = SBUF;
		RI = 0;
		if(uart_rx_index >= 10)
		{
			uart_rx_index = 0;
			memset(uart_rx_buf, 0, 10); 
		}
	}
}	

/**
 * @brief 超声波测距任务函数
 */
void ultra_task(void) {
	dist = ultra_dat();
}

/**
 * @brief DS1302时钟任务函数
 */
void rtc_task(void) {
	read_rtc(ucRTC);
}
	 
/**
 * @brief AD/DA转换任务函数
 * @note  由于AD转换读取到的是上一次转换的数值，因此读取两个通道的数值时需要交换通道
 */
void ad_da_task(void) {
	volt_1_x100 = ad_read(0x43) * 100 / 51; // 读取光敏电阻电压值
	volt_2_x100 = ad_read(0x41) * 100 / 51; // 读取电位器电压值
}

/**
 * @brief DS18B20温度转换任务函数
 */
void temperature_task(void) {
	temperature_x100 = read_temperature() * 100;
}

/* Timer 0 - NE555 */
/**
 * @brief Timer 0初始化函数
 * @note  初始化NE555
 */
void timer0_init(void)		
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;     //设置定时器0为16位不自动重装载计数模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计数
}

/* Timer 1 */
/**
 * @brief Timer 1初始化函数
 */
void timer1_init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
  ET1 = 1;        // 使能定时器1中断
	EA = 1;         // 使能全局中断
}

/**
 * @brief Timer 1中断服务函数
 */

void timer1_server(void) interrupt 3 {
  // --- 系统时钟 ---
  uwTick++; 	
	
	// --- 数码管 ---
	seg_pos = (++seg_pos) % 8; // 数码管位置索引在 0-7 之间循环切换
	if(seg_buf[seg_pos] > 20) seg_disp(seg_pos, seg_buf[seg_pos] - '.', 1); // 显示小数点
	else seg_disp(seg_pos, seg_buf[seg_pos], 0); // 熄灭小数点
	
	// --- LED ---
	
	
	// --- Freq ---
	// 计时1秒，读取频率值
	if(++timer_1s >= 1000) {
		timer_1s = 0; // 重置计时
		TR0 = 0; // 停止定时器0计数
		freq = (TH0 << 8) | TL0; // 读取频率值
		TL0 = TH0 = 0; // 重置计数初值
		TR0 = 1; // 开启定时器0计数
	}
	
	// --- Uart ---
	// 串口未接收到数据时，持续计时
	if(uart_rx_flag)
		uart_tick++;
}

// *** 调度器 *** //

/* 调度器任务结构体定义 */
typedef struct {
	void (*task_func)(void);  // 任务函数
	unsigned long int prd_ms; // 任务执行周期
	unsigned long int lst_ms; // 上次任务函数执行的时间
} task_t;

/* 调度器任务函数列表数组 */
idata task_t scheduler_task[] = {
	{key_task, 10, 0},
	{seg_task, 200, 0},
	{led_task, 1, 0},
	{uart_task, 10, 0},
	{ultra_task, 100, 0},
	{rtc_task, 200, 0},
	{ad_da_task, 200, 0},
	{temperature_task, 200, 0},
};

/**
 * @brief 调度器初始化函数
 * @note  获取调度器任务函数数量
 */
void scheduler_init(void) {
	task_num = sizeof(scheduler_task) / sizeof(task_t);
}

/**
 * @brief 调度器任务调度函数
 * @note  时间片轮询调度任务函数
 */
void scheduler_run(void) {
	unsigned char i;
	
	for(i = 0; i < task_num; i++) {
	  unsigned long int now_time = uwTick; // 获取当前系统时间
    if(now_time - scheduler_task[i].lst_ms >= scheduler_task[i].prd_ms) {
		  scheduler_task[i].lst_ms = now_time; // 更新任务函数上次执行时间
			scheduler_task[i].task_func(); // 执行任务函数
		}			
	}		
}

void main(void) {
	sys_init();
	write_rtc(ucRTC); // 写入时钟
	timer0_init();
	timer1_init();
	scheduler_init();
	while(1) {
		scheduler_run();
	}
}