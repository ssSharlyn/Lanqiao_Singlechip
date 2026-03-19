#include <STC15F2K60S2.H>
#include <string.h>
#include "sys.h"
#include "seg.h"
#include "key.h"
#include "led.h"
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"

// ** 参数定义区 ** //

/* scheduler */
idata unsigned long int uwTick; // 系统计时
idata unsigned char task_num; // 调度器任务函数数量

/* key */ 
idata unsigned char key_val, key_down, key_up, key_old; // 按键状态，瞬时状态、下降沿、上升沿、上次状态
bit key_flag; // 长按计时标志
unsigned int timer_2s; // ws计时

/* seg */
pdata unsigned char seg_buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管状态缓存数组，初始化为熄灭
idata unsigned char seg_pos; // 数码管位置索引
idata unsigned char seg_mode; // 数码管显示模式，0-时间，1-回显，2-参数
unsigned char seg_mode_rcd; // 数码管显示模式记录
idata unsigned char rtn_mode; // 回显模式，0-温度，1-湿度，2-时间

/* LED */
pdata unsigned char ucled[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // LED状态缓存数组，初始化为熄灭
bit led_flag; // LED闪烁标志
unsigned char timer_100ms; // 100ms计时

/* RTC */
pdata unsigned char ucRTC[3] = {23, 59, 55}; // 时钟数据缓存数组
unsigned char ucRTC_rcd[2] = {00, 00}; // 采集时间数据缓存数组  

/* AD/DA */
idata unsigned int volt_1_x100, volt_2_x100; // 光敏电阻/电位器电压值x100(节省存储空间和简化计算)
bit lit_flag = 0, lit_flag_old = 0; // 当前/上次亮暗状态 0-暗 1-亮
idata unsigned int timer_3s; // 3s计时
bit cllct_flag = 0; // 数据采集标志
bit cllct_lock_flag = 0; // 数据采集锁定标志(防止3s内重复采集)
unsigned char cllct_num = 0; // 采集次数

/* Temperature */
idata unsigned char temperature; // 温度值
idata unsigned char humi; // 湿度值
unsigned char temperature_rcd; // 上次温度值
unsigned char humi_rcd; // 上次湿度值
unsigned char temperature_p = 30; // 温度参数
unsigned char temperature_max; // 最大温度
unsigned int temperature_ave_x10; // 平均温度x10
unsigned int temperature_sum; // 温度总和
unsigned char humi_max;        // 最大湿度
unsigned int humi_ave_x10;        // 平均湿度x10
unsigned int humi_sum;        // 湿度总和
bit data_flag = 0; // 数据标志位(用于判断数据是否升高)

/* Freq */
idata unsigned int freq; // 频率值
idata unsigned int timer_1s; // 1s计时													 

/* other parameters */


// ** 函数定义区 ** //

/**
 * @brief 按键状态机任务函数
 * @note  利用位运算进行边沿检测
 */
void key_task(void) {
	// 温湿度界面下，所有按键无效，直接返回
	if(seg_mode == 3) return;
	
	key_val = key_read(); // 获取当前按键状态
	key_down = key_val & (key_val ^ key_old); // 获取按键下降沿状态
	key_up = ~key_val & (key_val ^ key_old);  // 获取按键上升沿状态
	key_old = key_val; // 更新上一次按键状态
	
	switch(key_down) {
		// --- 界面按键 --- 
		case 4:
			seg_mode = (seg_mode + 1) % 3; 
		  // 每次从时间界面切换到回显界面时，处于温度回显界面
		  if(seg_mode == 1) {
				rtn_mode = 0;
			}
		  break;	
		// --- 回显按键 --- 
		case 5:
			// 只在回显界面生效
		  if(seg_mode == 1)
			  rtn_mode = (rtn_mode + 1) % 3; 
		  break;	
		// --- + 按键 --- 
		case 8:
			// 只在参数界面生效
			if(seg_mode == 2) {
			  if(++temperature_p > 99) {
					temperature_p = 0;
				}
			}
		  break;	
		// --- - 按键 --- 
		case 9:
			// 只在参数界面生效
		  if(seg_mode == 2) {
			  if(--temperature_p > 200) {
					temperature_p = 99;
				}
			}
		  break;	
	}
	
	// --- 按键9 长按功能 ---
	// 只在时间回显界面生效
	if(seg_mode == 1 && rtn_mode == 2) {
		// 检测到按键9按下
	  if(key_down == 9) {
	  	key_flag = 1; // 开始长按计时	
		}
		// 检测到按键9松开
		if(key_up == 9) {
			// 长按达到2s,清除所有记录的数据
		  if(timer_2s >= 2000) {
				temperature_max = 0; 
        temperature_ave_x10 = 0;  
        temperature_sum = 0;  
        humi_max = 0;        
        humi_ave_x10 = 0;        
        humi_sum = 0;  
       	memset(ucRTC_rcd, 0, 2);
        cllct_num = 0;				
			}	
      key_flag = 0; // 重置长按计时标志	
      timer_2s = 0;	// 重置计时		
		}
	}
}

/**
 * @brief 数码管显示任务函数
 * @note  需要显示小数点时，在值后加上'.'
 */
void seg_task(void) {
  
	switch(seg_mode) {
		// --- 时间界面 ----
		case 0:
	    seg_buf[2] = seg_buf[5] = 11; // 显示短横线"-"
	    seg_buf[0] = ucRTC[0] / 10; // 显示时
		  seg_buf[1] = ucRTC[0] % 10;
		  seg_buf[3] = ucRTC[1] / 10; // 显示分
		  seg_buf[4] = ucRTC[1] % 10;
		  seg_buf[6] = ucRTC[2] / 10; // 显示秒
		  seg_buf[7] = ucRTC[2] % 10;
		  break;
		// --- 回显界面 ----
		case 1:
			seg_buf[0] = 12 + rtn_mode; // 显示字母
	    switch(rtn_mode) {
				// -- 温度回显 --
				case 0:
					seg_buf[1] = 10;
				  // 触发次数不为0才显示
				  if(cllct_num > 0) {
				    seg_buf[2] = temperature_max / 10 % 10;
	          seg_buf[3] = temperature_max % 10;
		        seg_buf[4] = 11;
				    seg_buf[5] = temperature_ave_x10 / 100 % 10;
		        seg_buf[6] = temperature_ave_x10 / 10 % 10 + '.';
		        seg_buf[7] = temperature_ave_x10 % 10;
					}
					else {
						seg_buf[2] = 10;
	          seg_buf[3] = 10;
		        seg_buf[4] = 10;
				    seg_buf[5] = 10;
		        seg_buf[6] = 10;
		        seg_buf[7] = 10;
					}
				  break;
				// -- 湿度回显 --
				case 1:
					seg_buf[1] = 10;
					// 触发次数不为0才显示  
				  if(cllct_num > 0) {
				    seg_buf[2] = humi_max / 10 % 10;
	          seg_buf[3] = humi_max % 10;
		        seg_buf[4] = 11;
				    seg_buf[5] = humi_ave_x10 / 100 % 10;
		        seg_buf[6] = humi_ave_x10 / 10 % 10 + '.';
		        seg_buf[7] = humi_ave_x10 % 10;
					}
				  else {
						seg_buf[2] = 10;
	          seg_buf[3] = 10;
		        seg_buf[4] = 10;
				    seg_buf[5] = 10;
		        seg_buf[6] = 10;
		        seg_buf[7] = 10;
					}
				  break;
				// -- 时间回显 --
				case 2:
					seg_buf[1] = cllct_num / 10 % 10;
				  seg_buf[2] = cllct_num % 10;
				  // 触发次数不为0才显示
				  if(cllct_num > 0) {
	          seg_buf[3] = ucRTC_rcd[0] / 10; // 显示时
		        seg_buf[4] = ucRTC_rcd[0] % 10;
				    seg_buf[5] = 11;            // 显示短横线"-"
		        seg_buf[6] = ucRTC_rcd[1] / 10; // 显示分
		        seg_buf[7] = ucRTC_rcd[1] % 10;	     
					}
					else {
						seg_buf[3] = 10;
		        seg_buf[4] = 10;
				    seg_buf[5] = 10;
		        seg_buf[6] = 10;
		        seg_buf[7] = 10;
					}
				  break;
			}
		  break; 
		// --- 参数界面 ----
		case 2:
			seg_buf[0] = 15;
		  seg_buf[1] = seg_buf[2] = seg_buf[3] = seg_buf[4] = seg_buf[5] = 10;
		  seg_buf[6] = temperature_p / 10;
		  seg_buf[7] = temperature_p % 10;
		  break; 
		// --- 温湿度界面 ----
		case 3:
			seg_buf[0] = 16;
		  seg_buf[1] = seg_buf[2] = 10;
		  // 温度数据
		  seg_buf[3] = temperature / 10 % 10;
		  seg_buf[4] = temperature % 10;
			seg_buf[5] = 11;
		  // 湿度数据有效
		  if(humi != 255) {
		    seg_buf[6] = humi / 10 % 10;
			  seg_buf[7] = humi % 10;
			}
			// 湿度数据无效
			else {
				seg_buf[6] = seg_buf[7] = 17; // 显示AA
			}
		  break; 
	}
}

/**
 * @brief LED任务函数
 */
void led_task(void) {
	unsigned char i;
	
	// --- 界面指示灯 ---
	for(i = 0; i < 2; i++){
	  ucled[i] = (i == seg_mode); // 对应界面点亮对应LED
	}
	ucled[2] = (seg_mode == 3);
	
	// -- 报警指示灯 ---
	ucled[3] = led_flag; // 采集温度大于温度参数时，指示灯LED4闪烁
	ucled[4] = (humi == 255); // 采集到无效湿度数据时，LED5点亮
	ucled[5] = data_flag; // 采集到的数据均升高时，LED5点亮
	
  led_disp(ucled);	
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
	// 3s内不重复触发采集，直接返回
	if(cllct_lock_flag == 1) return; 
	
	volt_1_x100 = ad_read(0x43) * 100 / 51; // 读取光敏电阻电压值
	volt_2_x100 = ad_read(0x41) * 100 / 51; // 读取电位器电压值
	
	// 检测到状态为亮
	if(volt_1_x100 > 250) {
		lit_flag = 1; // 拉高标志位
	}
	// 检测到状态为暗
	else {
		lit_flag = 0; // 重置标志位
	}
	
	// 从亮状态切换到暗状态
	if(lit_flag == 0 && lit_flag_old == 1) {
		cllct_flag = 1; // 触发温湿度采集
		cllct_lock_flag = 1; // 锁定采集
		timer_3s = 0;   // 重置3s计时
		seg_mode_rcd = seg_mode; // 记录切换前的界面
		seg_mode = 3;   // 切换到温湿度界面
	}
	
	lit_flag_old = lit_flag; // 更新上次亮暗状态
}

/**
 * @brief DS18B20温度转换任务函数
 */
void temperature_task(void) {
	// 采集标志为低，直接返回
	if(cllct_flag == 0) return;
	
	// 采集标志为高，开始采集
	cllct_flag = 0; // 重置采集标志
	
	// 读取温度
	temperature = read_temperature();
	
  // 湿度数据无效
	if(freq < 200 || freq > 2000) {
		humi = 255;	
	}
	// --- 数据有效时才做记录 ---
	else {	
	  cllct_num++;    // 采集次数+1
    memcpy(ucRTC_rcd, ucRTC, 2); // 记录采集时的时间
		
		// --- 温度数据处理 ---
	  temperature_max = temperature > temperature_max ? temperature : temperature_max; // 计算最大温度 
	  temperature_sum += temperature; // 计算温度总和
    temperature_ave_x10 = temperature_sum * 10 / cllct_num; // 计算平均温度
		
		// --- 湿度数据处理 ---
    humi = 8.0 / 180 * (freq - 200) + 10;
    humi_max = humi > humi_max ? humi : humi_max; // 计算最大湿度 
	  humi_sum += humi; // 计算湿度总和
    humi_ave_x10 = humi_sum * 10 / cllct_num; // 计算平均湿度
	}
	
	// 当采集到的数据均升高时，拉高标志位
	if(cllct_num >= 2 && temperature > temperature_rcd && humi > humi_rcd && humi != 255) {
		data_flag = 1;
	}
	else {
		data_flag = 0;
	}
	
	// 更新上一次采集到的数据记录
  temperature_rcd = temperature;
	humi_rcd = humi;
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
	
	// --- 按键 ---
	if(key_flag) {
		if(++timer_2s >= 2000) {
			timer_2s = 2000; // 将计时钳位在2000 
		}
	}
	
	// --- 数码管 ---
	seg_pos = (++seg_pos) % 8; // 数码管位置索引在 0-7 之间循环切换
	if(seg_buf[seg_pos] > 20) seg_disp(seg_pos, seg_buf[seg_pos] - '.', 1); // 显示小数点
	else seg_disp(seg_pos, seg_buf[seg_pos], 0); // 熄灭小数点
	
	// --- LED ---
	// 采集温度大于温度参数时，开始计时
	if(temperature > temperature_p) {	
		// 每计时100ms翻转LED状态
		if(++timer_100ms >= 100) {
			timer_100ms = 0;
		  led_flag ^= 1;	
		}
	}
	// 采集温度小于等于温度参数时，LED熄灭
	else {
		timer_100ms = 0;
		led_flag = 0;
	}
	
	// --- Freq ---
	// 计时1秒，读取频率值
	if(++timer_1s >= 1000) {
		timer_1s = 0; // 重置计时
		TR0 = 0; // 停止定时器0计数
		freq = (TH0 << 8) | TL0; // 读取频率值
		TL0 = TH0 = 0; // 重置计数初值
		TR0 = 1; // 开启定时器0计数
	}
	
	// --- 温湿度采集 ---
	if(cllct_lock_flag == 1) {
		timer_3s++;
		// 3秒时间到
		if(timer_3s >= 3000) {
			cllct_lock_flag = 0; // 解除采集锁定
			timer_3s = 0;
			seg_mode = seg_mode_rcd; // 返回切换前的界面
		}
	}
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
	read_temperature(); // 上电时读一下温度，防止后面读到85度
	timer0_init();
	timer1_init();
	scheduler_init();
	while(1) {
		scheduler_run();
	}
}