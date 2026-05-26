/*
* ADS8055E连接PD0-11 外部时钟 
* 输入到TIM4CH3 PB8输入捕获
* 最大实时采样率可达7.2M
* 连接PA2 PA3 PE0 接入方波测量信号频率 
* 配合PB6 PB7 I2C控制的SI5351可实现精确等效采样
*/

#include "stm32f10x.h"
#include "myfunc.h"
#include "ADC_TIM.h"
#include "TIM_PWM.h"
#include "public.h"
#include "SI5351_Soft.h"
#include "usart.h"
#include "TIM_ETR.h"
#include "TIM_capture.h"
#include "Basic_TIM.h"
#include "ADS805E.h"
extern u8 uart_data[20];
u8 vert_scale = 0;
u8 horz_scale = 1;
u16 trigger_level = 0;
u8 stop = 0;
u8 handle_arr[400];
u8 store_memory[400];
float vpp = 0;
bool init = true;
/*
* horz:
1: 20ms/div
2: 2us/div
3: 100ns/div
4: auto

vert:
1: 1V/div
2: 0.1V/div
3: 2mV/div

trigger_level:
电压值
*/

extern double UnknownSig_fre;
extern uint16_t ADC_Value[BUFFER_SIZE];
extern bool ADC_done;

State state = STATE_SEQUENTIAL_SAMPLE;//状态机
AMP_TIMES amp_times = AMP_0_04;//放大倍数

int main(void){
	SystemInit();
	TIM_ETR_Init();//配置TIM1外部时钟 PE0
	TIM_capture_init();//配置TIM2捕获 PA2 PA3
	BasicTIM_Init();//配置1s定时器
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	ADS805E_Init();
	Si5351Init();//初始化SI5351,I2C数据引脚是PB6和PB7


    SetFrequency(10000000,1);
	delay_ms(1000);//等待系统稳定
	init = false;
	while(1){
		//if(ADC_done){//采集完毕 开始处理数据
			//update_SI5351();
//			if(UnknownSig_fre < 500000)
//				SetFrequency(UnknownSig_fre*100/101.0,1);
//			else 
//				SetFrequency(UnknownSig_fre*60/601.0,1);
			SetFrequency(40000,1);
			ADC_done = false;
			DMA_Start();
		//}
		delay_ms(10);
	}
	
	return 0;
}


void update_SI5351(void){
	volatile double temp = 0;
	switch(horz_scale){
		case 1: 
			SetFrequency(54758,1);
			ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_239Cycles5);
		break;
		case 2: 
			SetFrequency(UnknownSig_fre/(10+UnknownSig_fre*0.5e-7),1);
		break;
		case 3: 
			SetFrequency(UnknownSig_fre/(200+UnknownSig_fre*0.25e-8),1);
		break;
		case 4:
		default:
			SetFrequency(UnknownSig_fre/(10+UnknownSig_fre*0.25e-8),1);
		break; 
	}
}


void send_data(u8* arr){
	const char* begin= "addt s0.id,0,400";
	while(*begin){
	  USART_SendData(USART1,*begin);
	  begin++;
	}
	delay_ms(50);
	for(u16 i = 0; i < 400; i++){
	  USART_SendData(USART1,arr[i]);
	  delay_us(100);
	  USART_SendData(USART1,0);
	  delay_us(100);
	}
	return;
  }

void reaction(void){
	if(isdigit(uart_data[0])){
		switch(uart_data[0]){
			case '0':
			vert_scale = 1;
			break;

			case '1':
			switch(uart_data[1]){
				case '0': 
					horz_scale = 1;
					ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_239Cycles5);
				break;
				case '1': 
					horz_scale = 2;
					ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_1Cycles5);
				break;
				case '2': 
					horz_scale = 3;
					ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_1Cycles5);
				break;
				default: vert_scale = 2;
				break;
			}
			break;

			case '2':
			switch(uart_data[1]){
				case '0': trigger_level = 0;
				break;
				case '1': trigger_level = 3;
				break;
				case '2': trigger_level = 100;
				break;
				case '3': trigger_level = 300;
				break;
				case '4': trigger_level = 500;
				break;
				case '5': trigger_level = 2000;
				break;
				default: vert_scale = 3;
				break;
			}
			break;

			default:
			break;
		}
	}
	else if(!strncmp((const char*)uart_data,"single",6))//单次触发模式
		state = STATE_SINGLE_SAMPLE;
	else if(!strncmp((const char*)uart_data,"nosingle",8))//退出单次触发模式
		state = STATE_SEQUENTIAL_SAMPLE;
	else if(!strncmp((const char*)uart_data,"run",3))
		state = STATE_SEQUENTIAL_SAMPLE;
	else if(!strncmp((const char*)uart_data,"stop",4)){
		state = STATE_STOP;
		stop = true;
	}
	else if(!strncmp((const char*)uart_data,"save",4))
		state = STATE_SAVE;
	else if(!strncmp((const char*)uart_data,"recall",6))
		state = STATE_RECALL;
    else if(!strncmp((const char*)uart_data,"autoset",7)){
    state = STATE_SEQUENTIAL_SAMPLE;
    horz_scale = 4;
	}

  return;
}

void operation(u8 *arr){
  if(vert_scale == 1){
	for(u16 i = 0; i < 400; i++){
	  arr[i] = arr[i] >> 2;
	}
  }
  else if(vert_scale == 2){
	for(u16 i = 0; i < 400; i++){
	  arr[i] = arr[i] >> 4;
	}
  }
  else if(vert_scale == 3){
	for(u16 i = 0; i < 400; i++){
	  arr[i] = arr[i] >> 6;
	}
  }
  return;
}