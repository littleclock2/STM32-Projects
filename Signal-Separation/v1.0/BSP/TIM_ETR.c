#include "TIM_ETR.h"
// 16分频分频器，占用TIM4
void TIM_ETR_Init16(void){// 用于分频器的定时器ETR初始化
	// 结构体定义
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure = {
		.TIM_Prescaler = 1 - 1,
		.TIM_Period = 16 - 1,// 32分频
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_ClockDivision = TIM_CKD_DIV1
	};
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB, ENABLE);
	// GPIO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource0,GPIO_AF_TIM4);//PE0 
	// GPIO配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitStructure);// PD15
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_TIM4);//PB9
	
	// 定时器配置
	TIM_TimeBaseInit(TIM4,&TIM_BaseInitStructure);
	TIM_ETRClockMode2Config(TIM4,
													1 - 1,// 外部触发分频器														
													TIM_ExtTRGPolarity_NonInverted,	// 外部触发极性		
													1);																
	
	TIM_OCInitTypeDef TIM_OC_InitStructure;
	TIM_OC_InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC_InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC_InitStructure.TIM_Pulse = 8;
	TIM_OC_InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM4,&TIM_OC_InitStructure);
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_Cmd(TIM4,ENABLE);
}
// 16分频分频器，占用TIM1
void TIM_ETR_Init32(void){// 用于分频器的定时器ETR初始化
	// 结构体定义
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure = {
		.TIM_Prescaler = 1 - 1,
		.TIM_Period = 16 - 1,// 16分频
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_ClockDivision = TIM_CKD_DIV1
	};
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOD, ENABLE);
	// GPIO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_TIM3);//PD2
	// GPIO配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_TIM3);//PC9   //不行，要重新配
	
	// 定时器配置
	TIM_TimeBaseInit(TIM3,&TIM_BaseInitStructure);
	TIM_ETRClockMode2Config(TIM3,
													1 - 1,// 外部触发分频器														
													TIM_ExtTRGPolarity_NonInverted,	// 外部触发极性		
													1);																
	
	TIM_OCInitTypeDef TIM_OC_InitStructure;
	TIM_OC_InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC_InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC_InitStructure.TIM_Pulse = 8;
	TIM_OC_InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM3,&TIM_OC_InitStructure);
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);
	TIM_Cmd(TIM3,ENABLE);
}
//自定义分频器，5k-20k=2分频,20k-40k=4分频,40k-80k=8分频,80k-160k=16分频
// 该函数用于初始化定时器ETR以实现自定义分频
//初始化设定为16分频
void TIM_ETR_InitCustom(u8 div){// 用于分频器的定时器ETR初始化
	// 结构体定义
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure = {
		.TIM_Prescaler = 1 - 1,
		.TIM_Period = div - 1,// 16分频
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_ClockDivision = TIM_CKD_DIV1
	};
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	// GPIO配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM2);//PA0
	// GPIO配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_TIM2);//PC6 
	
	// 定时器配置
	TIM_TimeBaseInit(TIM2,&TIM_BaseInitStructure);
	TIM_ETRClockMode2Config(TIM2,
													1 - 1,// 外部触发分频器														
													TIM_ExtTRGPolarity_NonInverted,	// 外部触发极性		
													1);																
	
	TIM_OCInitTypeDef TIM_OC_InitStructure;
	TIM_OC_InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC_InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC_InitStructure.TIM_Pulse = div/2;
	TIM_OC_InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM2,&TIM_OC_InitStructure);
	TIM_OC4PreloadConfig(TIM2,TIM_OCPreload_Enable);
	TIM_Cmd(TIM2,ENABLE);
}

void TIM_ETR_CunstomConfig(uint32_t frequency) {
	// 根据输入频率选择分频器
	if (frequency <= 20 && frequency > 5) {// 5kHz-20kHz使用2分频
		TIM_ETR_InitCustom(2); // 初始化为2分频
	} 
	else if (frequency <= 40 && frequency > 20) {
		TIM_ETR_InitCustom(4); // 初始化为2分频
	} 
	else if (frequency <= 80 && frequency > 40) {
		TIM_ETR_InitCustom(6); // 初始化为2分频
	}
	else if (frequency <= 100 && frequency > 80) {
		TIM_ETR_InitCustom(8); // 初始化为2分频
	} 
}

