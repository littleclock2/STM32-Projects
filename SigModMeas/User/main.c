#include "stm32f4xx.h"  // STM32F4标准外设库
#include "bsp_led.h"    // LED驱动头文件
#include "Delay.h"      // 延时函数头文件
#include "SI5351_Soft.h" // SI5351时钟芯片驱动
#include "ADS805.h"     // ADS805 ADC驱动
#include <stdio.h>
#include <stdbool.h>
#include "math.h"
#include "arm_math.h"   // ARM数学库
#include <stdlib.h>
#include <stdint.h>
#include "stdarg.h"	
#include "uart.h"
#include "AD9833.h"
#include "adc.h"
#include "stm32f4xx_rcc.h"
#include "timerG.h"
// 宏定义
#define FFT_ADC_N 4096  // FFT采样点数
// 继电器引脚定义
#define RELAY_PINS (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)

u8  UART4_RX_BUF[30]; 				//接收缓冲,最大UART4_MAX_RECV_LEN个字节.
u8  UART4_TX_BUF[30]; 			//发送缓冲,最大UART4_MAX_SEND_LEN字节

vu16 UART4_RX_STA=0; 

// 全局变量
// 外部引用
extern const arm_cfft_instance_f32 arm_cfft_sR_f32_len4096; // ARM FFT实例
extern uint16_t ADS805_Data[Length];  // ADC数据缓冲区
extern uint16_t ADS805_Data_Completed; // ADC采集完成标志
// FFT相关变量
float fft_in_adc_data[FFT_ADC_N * 2] = {0}; // FFT输入数据(复数形式)
float fft_out_adc_data[FFT_ADC_N] = {0};    // FFT输出幅度
float f_s = 1.024e6;          // 采样频率1.024MHz
float f_s0 = 0;	//输出频率0
float f_s2 = 0; // 输出频率2

// 信号类型标志
bool signal_type = false; // 信号类型标志
u8 signal_mode = 0;      // 信号调制模式
u8 signal_mode_total = 0; // 上一次信号调制模式
u8 signal_mode_index = 0; // 信号调制模式变化标志
u8 signal_flag = 0;       // 信号标志
//ADC变量
uint16_t adc1_value, adc2_value;
uint16_t adc1_value_arr[128]  ={0}, adc2_value_arr[128]  ={0}; // ADC值
uint32_t adc1_value_sum = 0, adc2_value_sum = 0; // ADC值和
uint32_t adc1_value_avg = 0, adc2_value_avg = 0; // ADC值平均值
uint8_t adc_index = 0; // ADC索引
// 频率测量结果
uint32_t old_fre = 0;
//标定用变量组
uint32_t temp_1[128] = {0};
uint32_t temp_2[128] = {0};
uint32_t temp_3[128] = {0};
uint32_t temp_4[128] = {0};
int temp_index  =0;
// 继电器通道定义
typedef enum {
    RELAY_CH0 = 0,  // PC0
    RELAY_CH1,      // PC1
    RELAY_CH2,      // PC2
    RELAY_CH3       // PC3
} RelayChannel;

// 继电器状态
typedef enum {
    RELAY_OFF = 0,
    RELAY_ON
} RelayState;
//不同调制方式的参数
double M_a = 0;// AM调制参数
double M_f = 0;// FM调制参数
double M_f_detal = 0;// FM调制最大偏移量
double M_f_s = 0;// FM调制频率
double Rc_ASK = 0;// ASK调制参数
double Rc_FSK = 0;// FM调制参数
double h_FSK = 0;// FSK移频键控系数
double Rc_FSK_delta = 0; // AM调制参数
double Rc_PSK = 0;// PSK调制参数

FreqMeasureResult ch1 = {0, 0}; // 通道1频率测量结果
FreqMeasureResult ch2 = {0, 0}; // 通道2频率测量结果
//
uint16_t temp[64]={0};
int temp_0 = 0;
uint32_t temp_total = 0;

// 函数声明
void FFT_Deal(uint16_t adc_value[], float *fft_input, int data_length);
void u4_printf(char* fmt,...);//串口函数生成
void uart4_init(u32 bound);
float DFT_Calculation(float *Data, int num, int N);//DFT函数
void Judge(void);
void RCC_Configuration(void);
void SendData(void);
void DataMeasure(void); // 数据处理函数
// 继电器函数声明
void Relay_Init(void);
void Relay_Set(RelayChannel ch, RelayState state);
void Relay_Toggle(RelayChannel ch);
void Relay_Switch(void);
// 主函数
int main(void){    
    // 硬件初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组
    ADS805_Init();// ADS805 ADC初始化
    Start_ADS805();// 启动ADS805 ADC连续转换
    Si5351Init();// 初始化Si5351时钟芯片
    Led_Init();// LED初始化  
	AD9833_GPIO_Init();//AD9833端口初始化
	AD9833_Init();//AD9833初始化
	RCC_Configuration();// RCC时钟配置
    FreqMeasure_Init();// 初始化频率测量(PC6和PC7)
	// 初始化继电器控制
    Relay_Init();
	// 初始化ADC
    ADC_GPIO_Init();
    adc_Init();
    // 启动连续转换
    ADC_StartDualConversion();
	delay_ms(10);
	AD9833_ALLInit(1.8e6,0,0,114514,0,0);//初始化所有频率
	uart4_init(115200);//初始化串口	
	DMA_Cmd(DMA1_Stream5, DISABLE);	//关闭DMA传输
	DMA_Cmd(DMA1_Stream6, DISABLE);
	DMA_Cmd(DMA1_Stream5, ENABLE);	//开启DMA传输
	DMA_Cmd(DMA1_Stream6, ENABLE); 
    // 设置SI5351输出频率
    SetFrequency(f_s, 0);
    SetFrequency(f_s, 1);
    SetFrequency(f_s, 2);
    delay_ms(100); // 延时1秒
    while (1){  // 主循环
	    //USART_SendData(UART4, 123);   
	    //u4_printf("收到的消息：%c/r/n",
		if (ADS805_Data_Completed) { // ADC数据采集完成
            // FFT处理
            FFT_Deal(ADS805_Data, fft_in_adc_data, FFT_ADC_N);//加入虚部，使其完整
            arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_in_adc_data, 0, 1);//启动FFT
            arm_cmplx_mag_f32(fft_in_adc_data, fft_out_adc_data, FFT_ADC_N);
			// 调制模式判断
			Judge(); // 调用调制模式判断函数
			if(signal_mode_index >= 5 ){ // 如果信号调制模式发生变化
				if(signal_mode_total/5 == signal_mode){
					Relay_Switch();
				}
				signal_mode_index  = 0; // 设置信号模式变化标志
				signal_mode_total = 0;
			}
			else if(signal_mode_index <5){ // 如果信号调制模式未发生变化
				signal_mode_total += signal_mode; // 重置信号模式变化标志
				signal_mode_index++;
			}
			
			//Relay_Switch();
			delay_ms(200); // 延时100毫秒
            // 重新启动ADC采集
            Start_ADS805();
        }
		//开始测频
		ch1 = FreqMeasure_GetValue(FREQ_CH1);
        ch2 = FreqMeasure_GetValue(FREQ_CH2);
		//自动四舍五入测得的频率
		if(ch1.valid){
			ch1.frequency = (ch1.frequency + 400) / 1000 * 1000; // 四舍五入到千Hz
		}
		if(ch2.valid){
			ch2.frequency = (ch2.frequency + 400) / 1000 * 1000; // 四舍五入到千Hz
		}
		
		//截止频率配置
		if(signal_mode == 1|| signal_mode == 2 ){ //如果此信号为AM或ASK调制信号
			if(ch1.valid){
				f_s0 = ch1.frequency * 1.5 * 32; // 获取通道1频率值(PC6输入)
				f_s2 = f_s0;
			}
		}
		else if(signal_mode == 3 || signal_mode == 4 || signal_mode == 5){ //如果此信号为FM、FSK或PSK调制信号
			if(ch2.valid){
				f_s0 = ch2.frequency * 1.5 * 32; // 获取通道1频率值(PC7输入)
				f_s2 = f_s0;
			}
		}
//		if(temp_0 < 64){
//			temp[temp_0++] = f_s0;
//		}
//		else{
//			temp_0 =0;
//		}
		if(temp_0 > 5 ){ // 如果频率计数发生变化
			temp_0  = 0; // 设置信号模式变化标志
			if(temp_total/5 == f_s0){
			// 设置SI5351输出频率
				//if(f_s0 != old_fre && f_s0 != 0){
					//old_fre = f_s0;
					SetFrequency(f_s0, 0);
					SetFrequency(f_s2, 2);
				//}
			}
			temp_total = 0;
		}
		else if(temp_0 <=5){ // 如果信号调制模式未发生变化
			temp_total += f_s0; // 重置信号模式变化标志
			temp_0 ++;
		}
		
	    
		for(int i = 0;i<32;i++){
			// 获取双ADC值
			ADC_GetDualValues(&adc1_value,&adc2_value);
			adc1_value_sum += adc1_value; // 累加ADC1值
			adc2_value_sum += adc2_value; // 累加ADC2值
		}
		adc1_value_avg = adc1_value_sum / 32; // 计算ADC
		adc2_value_avg = adc2_value_sum / 32; // 计算ADC2平均值
		adc1_value_sum = 0; // 重置ADC1值和
		adc2_value_sum = 0; // 重置ADC2值和
		DataMeasure(); // 数据处理函数
	    //对各个数据进行处理
        SendData(); // 发送数据到串口
        delay_ms(10);
    }
}
// 串口数据发送函数
void SendData(void){//发送数据函数
	// 发送状态信息
	char str[40]={0};
	//char short_str[10] = {0};
	char *pt = str;
	//double temp = 0;
	// 第一次发送调制模式信息
	switch(signal_mode){// 信号调制模式判断
		case 0://连续载波
			sprintf(str,"t1.txt=\"CW\"\xff\xff\xff");
			break;
		case 1://AM调制
			sprintf(str,"t1.txt=\"AM\"\xff\xff\xff");
			break;
		case 2://ASK调制
			sprintf(str,"t1.txt=\"ASK\"\xff\xff\xff");
			break;
		case 3://FM调制
			sprintf(str,"t1.txt=\"FM\"\xff\xff\xff");
			break;
		case 4://FSK调制
			sprintf(str,"t1.txt=\"FSK\"\xff\xff\xff");
			break;
		case 5://PSK调制
			sprintf(str,"t1.txt=\"PSK\"\xff\xff\xff");
			break;
		default:
			//sprintf(str,"t1.txt=\"CW\"\xff\xff\xff");
			break;
	}
	while(*pt){
		USART_SendData(UART4,*pt++);
		delay_us(200);
    }
	delay_ms(10);
	char *pt0 = str;
// 第二次发送参数信息
	switch(signal_mode){// 信号调制模式判断
		case 1://AM调制
			sprintf(str,"t3.txt=\"%.2f%%\"\xff\xff\xff",M_a);
			break;
		case 2://ASK调制
			sprintf(str,"t3.txt=\"%.2f\"\xff\xff\xff",Rc_ASK);
			break;
		case 3://FM调制
			sprintf(str,"t3.txt=\"%.2f\"\xff\xff\xff",M_f);
			break;
		case 4://FSK调制
			sprintf(str,"t3.txt=\"%.2f\"\xff\xff\xff",Rc_FSK);
			break;
		case 5://PSK调制
			sprintf(str,"t3.txt=\"%.2f\"\xff\xff\xff",Rc_PSK);
			break;
		default:
			break;
	}
	while(*pt0){
		USART_SendData(UART4,*pt0++);
		delay_us(200);
    }
	delay_ms(10);
	char *pt1 = str;
	// 第三次发送频率信息
	switch(signal_mode){// 信号调制模式判断
		case 3://FM调制
			sprintf(str,"t5.txt=\"%.2f\"\xff\xff\xff",M_f_detal);
			break;
		case 4://FSK调制
			sprintf(str,"t5.txt=\"%.2f\"\xff\xff\xff",h_FSK);
			break;
		default:
			break;
	}
	while(*pt1){
		USART_SendData(UART4,*pt1++);
		delay_us(200);
    }
	delay_ms(10);
	char *pt2 = str;
	
	// 第三次发送频率信息
	switch(signal_mode){// 信号调制模式判断
		case 3://FM调制
			sprintf(str,"t7.txt=\"%.2f\"\xff\xff\xff",M_f_s);
			while(*pt2){
				USART_SendData(UART4,*pt2++);
				delay_us(200);
			}
			break;
		default:
			break;
	}
	
}

// FFT数据处理函数
void FFT_Deal(uint16_t adc_value[], float *fft_input, int data_length) {
    for(int i = 0; i < data_length; i++) {
        fft_input[2 * i] = (float)adc_value[i]; // 实部
        fft_input[2 * i + 1] = 0;              // 虚部
    }
}
//调制模式判断函数
void Judge(void){
	signal_flag = 0; // 初始化信号标志
	//先进行ASK判断，时域
	u8 ask_flag1 = 0; // ASK标志
	u8 ask_flag2 = 0; // ASK标志2
	//遍历ADS805_Data数组
	for(int i = 70;i<240;i++){
		if(ADS805_Data[i]<14500 && ADS805_Data[i+1]<14500){
			if(ADS805_Data[i]>14200 && ADS805_Data[i+1]>14200){
				ask_flag1 ++; // 计数
			}
		}
		if(ADS805_Data[i]<14000){
			ask_flag2 ++; // 计数
		}
	}
	if(ask_flag1 + ask_flag2 >= 80 && ask_flag2 > 10){ // 如果计数大于10，则认为是ASK调制信号
		signal_mode = 2; // ASK调制信号
		return; // 直接返回，不再进行其他判断
	}
	//再进行剩余的判断
	//遍历基频附近的的fft_out_adc_data数组
	for(int i = 600; i < 1000; i++) {//在基频附近左右两边寻找（基频200k，在数组中为801，左右找100个点,共计两百个点）
		if(fft_out_adc_data[i] > 80000 && fft_out_adc_data[i+2]< 80000 && fft_out_adc_data[i-2]<80000){
			signal_flag ++;
		}
	}
	if(signal_flag == 1){//如果有一个点大于70k，则认为是连续载波信号
		signal_mode = 0; // 连续载波信号
		return; // 直接返回，不再进行其他判断
	}
	else if(signal_flag == 3){//如果有三个点大于70k，则认为是AM调制信号
		signal_mode = 1; // AM调制信号
		return; // 直接返回，不再进行其他判断
	}
	else if(signal_flag >3){//如果有三个点以上大于70k，则进行FM与FSK的判断
		//FM判断
		double signal_max1 = 0; // 信号最大值1
		double signal_max2 = 0; // 信号最大值2
		uint16_t signal_max1_index = 0; // 信号最大值1索引
		uint16_t signal_max2_index = 0; // 信号最大值2索引
		signal_max1 = fft_out_adc_data[800]; // 更新最大值2
		signal_max1_index = 800; // 更新最大值2索引
		// 搜索范围从600到1000
		for(int i = 600; i < 1000; i++) {//第一次遍历求出最大值
			// 检查当前点是否大于基频的值
			if(fft_out_adc_data[i] >= fft_out_adc_data[800]) {
				if(fft_out_adc_data[i] > signal_max1){//如果当前点大于最大值2
					signal_max1 = fft_out_adc_data[i]; // 更新最大值2
					signal_max1_index = i; // 更新最大值2索引
				}
			}
		}
		for(int j = 600; j < 1000; j++) {//第二次遍历求出第二大值
			// 检查当前点是否大于基频的值
			if(fft_out_adc_data[j] >= fft_out_adc_data[800] ) {
				if(fft_out_adc_data[j]>signal_max2 && j != signal_max1_index){//如果当前点大于最大值2且不是最大值1
					signal_max2 = fft_out_adc_data[j]; // 更新最大值2
					signal_max2_index = j; // 更新最大值2索引
				}
			}
			else if(fft_out_adc_data[j] >= fft_out_adc_data[800] - 10000 ) {
				if(fft_out_adc_data[j]>signal_max2 && j != signal_max1_index){//如果当前点大于最大值2且不是最大值1
					signal_max2 = fft_out_adc_data[j]; // 更新最大值2
					signal_max2_index = j; // 更新最大值2索引
				}
			}
		}
		if(signal_max1_index == 800 && signal_max2_index ==0){//此时仅有一个最大值，为FM信号
			signal_mode = 3;//FM调制信号
			return; // 直接返回，不再进行其他判断
		}
		if(signal_max1_index != 800){
			if(signal_max1_index + signal_max2_index == 1600){//如果两个最值相加相当于基频两倍，则认为是调制信号
				if(fft_out_adc_data[800]<50000 && signal_max1-signal_max2>10000) {
					signal_mode = 5;//PSK调制信号
					return;
				}
				signal_mode = 3; // FM调制信号
				return; // 直接返回，不再进行其他判断
			}
			else{
				signal_mode = 4; // FSK调制信号
				return; // 直接返回，不再进行其他判断
			}
		}
	}
} 

void DataMeasure(void){ // 数据处理函数
	switch (signal_mode){  // 根据不同的表达式进行处理
		case 0: // 连续载波信号
			// 处理连续载波信号的逻辑
			break;
		case 1: // AM调制信号
			if(f_s0 != 0){ // 如果f_s0为0，则设置为默认值
				// 处理AM调制信号的逻辑
				switch((int)f_s0){
					case 48000: //1K
						//M_a = adc2_value_avg; // 1K调制参数
						M_a = 0.096133530530318900 * adc2_value_avg + 0.206000422565072000 ;
						break;
					case 96000: // 2k
						//M_a = adc2_value_avg; // 2K调制参数
						M_a = 0.05320896 * adc2_value_avg + 1.30222646 ;
						break;
					case 144000: // 3K
						M_a = 0.0004180598 * adc2_value_avg + 0.0085917510 ;
						M_a *= 100;
						break;
					case 192000: // 4K
						//M_a = adc2_value_avg; // 4K调制参数
						M_a = 0.0003682015 * adc2_value_avg + 0.0079946078 ;
						M_a *= 100;
						break;
					case 240000: // 5K
						//M_a = adc2_value_avg; // 5K调制参数
						M_a = 0.0003450458 * adc2_value_avg + 0.0057132055 ;
						M_a *= 100;
						break;
					default:
						break;
				}
			}
			break;
		case 2: // ASK调制信号
			// 处理ASK调制信号的逻辑
			if(f_s0 != 0){
				Rc_ASK = ch1.frequency; // ASK调制参数
			

			}
			
			break;
		case 3: // FM调制信号
			// 处理FM调制信号的逻辑
			if(f_s0 != 0){ // 如果f_s0为0，则设置为默认值
				switch(ch2.frequency){
					case 1000: //1K
						// 1K调制参数
						M_f = 0.0209531836 * adc1_value_avg + 0.0502573797 ;
						//M_f_detal *= 100;
						break;
					case 2000: // 2k
						// 2K调制参数
						M_f = 0.0108920998 * adc1_value_avg +  0.1027014623  ;
						//M_f_detal *= 100;
						break;
					case 3000: // 3K
						M_f = 0.0074643625 * adc1_value_avg + 0.1129504481 ;
						//M_f_detal *= 100;
						break;
					case 4000: // 4K
						// 4K调制参数
						M_f = 0.0057286387 * adc1_value_avg + 0.1515935505 ;
						//M_f_detal *= 100;
						break;
					case 5000: // 5K 
						M_f = 0.0047683361 * adc1_value_avg + 0.1681208481 ;
						//M_f_detal *= 100;
						break;
					default:
						break;
				}
				M_f_s = ch2.frequency;
				M_f_detal = 1.0 *  M_f * M_f_s; // FM调制参数
			}
			break;
		case 4: // FSK调制信号
			// 处理FSK调制信号的逻辑
			if(f_s0 != 0){ // 如果f_s0为0，则设置为默认值
				Rc_FSK = f_s0/48; // FSK调制参数
				switch((int)f_s0){
					case 288000: //1K
						// 1K调制参数
						Rc_FSK_delta = 0.0065934437 * adc1_value_avg + 0.2381292269  ;
						//Rc_FSK_delta *= 100;
						break;
					case 384000: // 2k
						// 2K调制参数
						Rc_FSK_delta = 0.0066402937 * adc1_value_avg + 0.0385097342 ;
						//Rc_FSK_delta *= 100;
						break;
					case 480000: // 3K
						Rc_FSK_delta =0.0130536017 * adc1_value_avg - 1.5107182444 ;
						//Rc_FSK_delta *= 100;
						break;
					default:
						break;
				}
				h_FSK = Rc_FSK_delta; // FSK移频
			}
			//Rc_FSK_delta = adc1_value_avg; // FSK调制偏移量
			
			
			break;
		case 5: // PSK调制信号
			// 处理PSK调制信号的逻辑
			if(f_s0 != 0) // 如果f_s0为0，则设置为默认值
				Rc_PSK = ch2.frequency; // PSK调制参数
			break;
		default:
			break;
	}
	//查看标定用数组
	// if(temp_index < 128) { // 如果索引小于128
	// 	temp_1[temp_index] = adc1_value_avg; // 存储通道1频率
	// 	temp_2[temp_index] = adc2_value_avg; // 存储通道2频率
	// 	temp_3[temp_index] = ch1.frequency; // 存储ADC1平均值
	// 	temp_4[temp_index] = ch2.frequency; // 存储ADC2平均值
	// 	temp_index++; // 索引加1
	// } 
	// else if(temp_index >= 128) { // 如果索引等于128
	// 	temp_index = 0; // 重置索引
	// }
}
// 外部中断处理函数
void EXTI9_5_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line9) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line9); // 清除中断标志
        // 可以添加DMA相关处理代码
    }
}

void uart4_init(u32 bound){
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//使能USART4时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4); //GPIOC11复用为UART4
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4); //GPIOC10复用为UART4	
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10; //GPIOC11和GPIOC10初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); //初始化GPIOC端口

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART4, &USART_InitStructure); //初始化串口4
	
	USART_Cmd(UART4, ENABLE);  //使能串口4 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
		
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启中断 

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器	
}
// 0:FSK,PSk;
// 1:AM;CW;
// 2:ASK;
// 3:FM;
// 继电器控制函数
void Relay_Switch(void){
	switch (signal_mode){
		case 0:// 连续载波信号
			Relay_Set(RELAY_CH0,RELAY_OFF); // 连续载波信号，关闭继电器通道0
			Relay_Set(RELAY_CH1,RELAY_ON); // 连续载波信号，开启继电器通道1
			Relay_Set(RELAY_CH2,RELAY_OFF); // 连续载波信号，关闭继电器通道2
			Relay_Set(RELAY_CH3,RELAY_OFF); // 连续载波信号，关闭继电器通道3
			break;
		case 1:// AM调制信号
			Relay_Set(RELAY_CH0,RELAY_OFF); // 连续载波信号，关闭继电器通道0
			Relay_Set(RELAY_CH1,RELAY_ON); // 连续载波信号，开启继电器通道1
			Relay_Set(RELAY_CH2,RELAY_OFF); // 连续载波信号，关闭继电器通道2
			Relay_Set(RELAY_CH3,RELAY_OFF); // 连续载波信号，关闭继电器通道3
			break;
		case 2:// ASK调制信号
			Relay_Set(RELAY_CH0,RELAY_OFF); // ASK调制信号，关闭继电器通道0
			Relay_Set(RELAY_CH1,RELAY_OFF); // ASK调制信号，关闭继电器通道1
			Relay_Set(RELAY_CH2,RELAY_ON); // ASK调制信号，开启继电器通道2
			Relay_Set(RELAY_CH3,RELAY_OFF); // ASK调制信号，关闭继电器通道3
			break;
		case 3:// FM调制信号
			Relay_Set(RELAY_CH0,RELAY_OFF); // FM调制信号，关闭继电器通道0
			Relay_Set(RELAY_CH1,RELAY_OFF); // FM调制信号，关闭继电器通道1
			Relay_Set(RELAY_CH2,RELAY_OFF); // FM调制信号，关闭继电器通道2
			Relay_Set(RELAY_CH3,RELAY_ON); // FM调制信号，开启继电器通道3
			break;
		case 4:// FSK调制信号
			Relay_Set(RELAY_CH0,RELAY_OFF); // FSK调制信号，关闭继电器通道0
			Relay_Set(RELAY_CH1,RELAY_OFF); // FSK调制信号，关闭继电器通道1
			Relay_Set(RELAY_CH2,RELAY_OFF); // FSK调制信号，关闭继电器通道2
			Relay_Set(RELAY_CH3,RELAY_OFF); // FSK调制信号，关闭继电器通道3
			break;
		case 5:// PSK调制信号
			Relay_Set(RELAY_CH0,RELAY_ON); // PSK调制信号，开启继电器通道0
			Relay_Set(RELAY_CH1,RELAY_OFF); // PSK调制信号，关闭继电器通道1
			Relay_Set(RELAY_CH2,RELAY_OFF); // PSK调制信号，关闭继电器通道2
			Relay_Set(RELAY_CH3,RELAY_OFF); // PSK调制信号，关闭继电器通道3
			break;
		default:
			break;
	}
}

void UART4_IRQHandler(void)                	//串口4中断服务程序
{
	u8 Res;   

	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(UART4);//(USART1->DR);	//读取接收到的数据
		
		if((UART4_RX_STA&0x8000)==0)//接收未完成
		{
			if(UART4_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)UART4_RX_STA=0;//接收错误,重新开始
				else UART4_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)UART4_RX_STA|=0x4000;
				else
				{
					UART4_RX_BUF[UART4_RX_STA&0X3FFF]=Res ;
					UART4_RX_STA++;
					if(UART4_RX_STA>(30-1))UART4_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
} 
void u4_printf(char* fmt,...)  {  
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)UART4_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)UART4_TX_BUF);//此次发送数据的长度
	for(j=0;j<i;j++)//循环发送数据
	{
	  while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET);  //等待上次传输完成 
		USART_SendData(UART4,(uint8_t)UART4_TX_BUF[j]); 	 //发送数据到串口3 
	}
	
}

void RCC_Configuration(void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
    
    RCC_PLLConfig(RCC_PLLSource_HSE, 8, 336, 2, 7);
    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource() != 0x08);
    
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div4);
    RCC_PCLK2Config(RCC_HCLK_Div2);
}

void Relay_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 使能GPIOC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    // 2. 配置PC0-PC3为推挽输出
    GPIO_InitStructure.GPIO_Pin = RELAY_PINS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // 3. 初始状态全部关闭
    GPIOC->BSRRH = RELAY_PINS; // 全部置低
}

void Relay_Set(RelayChannel ch, RelayState state)
{
    uint16_t pin = GPIO_Pin_0 << ch;
    
    if(state == RELAY_ON) {
        GPIOC->BSRRL = pin;  // 置高
    } else {
        GPIOC->BSRRH = pin;  // 置低
    }
}

void Relay_Toggle(RelayChannel ch)
{
    uint16_t pin = GPIO_Pin_0 << ch;
    GPIOC->ODR ^= pin;  // 翻转状态
}
