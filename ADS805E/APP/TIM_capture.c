#include "public.h"
#include "TIM_capture.h"
/*TIM2 IC3 IC4 作为捕获输入通道,重映射在PA2 PA3*/
extern bool init;
void TIM_capture_init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	TIM_TimeBaseInitTypeDef TIM2_Init_Struct = {
		.TIM_Prescaler = 1 - 1,
		.TIM_Period = 65535,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_ClockDivision = TIM_CKD_DIV1
	};
	
	TIM_ICInitTypeDef ICInitStruct = {
		.TIM_Channel = TIM_Channel_3,//通道三
		.TIM_ICFilter = 0,//滤波器
		.TIM_ICPolarity = TIM_ICPolarity_Rising,//上升沿捕获
		.TIM_ICPrescaler = TIM_ICPSC_DIV1,//输入信号分频为0
		.TIM_ICSelection = TIM_ICSelection_DirectTI
	};
	
	NVIC_InitTypeDef TIM2_IC3_NVICInitStruct = {
		.NVIC_IRQChannel = TIM2_IRQn,
		.NVIC_IRQChannelCmd = ENABLE,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 4
	};
	
	GPIO_InitTypeDef IC_Channel = {//捕获的输入通道需要用GPIO输入 而不是复用输入 就很神奇
		.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3,
		.GPIO_Mode = GPIO_Mode_IN_FLOATING,
		.GPIO_Speed = GPIO_Speed_50MHz,
	};
	
	
	GPIO_Init(GPIOA,&IC_Channel);
	NVIC_Init(&TIM2_IC3_NVICInitStruct);
	
	TIM_ICInit(TIM2,&ICInitStruct);
	ICInitStruct.TIM_Channel = TIM_Channel_4;
	ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInit(TIM2,&ICInitStruct);//配置通道4
	
	TIM_TimeBaseInit(TIM2,&TIM2_Init_Struct);//默认的时基单元初始化是将预分频器的缓冲关闭的
	TIM_InternalClockConfig(TIM2);//和ETR混用时 需要把时钟CLK_PSK之前的从模式控制器配置成选择内部时钟 CLK_IN
	if(!init)
		TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC3|TIM_IT_CC4,ENABLE);
	TIM_ClearFlag(TIM2,TIM_IT_Update | TIM_IT_CC4 | TIM_IT_CC3);
	TIM2->CCR1 = 0;
	TIM2->CCR2 = 0;
	TIM2->CCR3 = 0;
	TIM2->CCR4 = 0;
	
	//TIM_ARRPreloadConfig(TIM2,DISABLE);//禁用ARR影子计数器
	TIM_Cmd(TIM2,ENABLE);
	
	return;
}
