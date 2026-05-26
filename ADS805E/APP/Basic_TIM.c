#include "Basic_TIM.h"
#include "public.h"
/*配置基本定时器TIM6 or TIM7*/
void BasicTIM_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);//打开Timer6 基本定时器
	TIM_TimeBaseInitTypeDef TIM_st = {
		.TIM_Prescaler = 7200 - 1,//预分频
		.TIM_Period = 10000 - 1,//load value
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_ClockDivision = TIM_CKD_DIV1,//时钟分频
		.TIM_RepetitionCounter = 0//这个无需配置
	};
	NVIC_InitTypeDef TIM_NVIC_Init = {
		.NVIC_IRQChannel = TIM6_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 0,
		.NVIC_IRQChannelSubPriority = 3,
		.NVIC_IRQChannelCmd = ENABLE	
	};
	
	
	NVIC_Init(&TIM_NVIC_Init);//timer6内部中断初始化
	TIM_TimeBaseInit(TIM6,&TIM_st);//初始化
	TIM_ITConfig(TIM6,TIM_FLAG_Update,ENABLE);//使能中断
	TIM_ClearFlag(TIM6,TIM_FLAG_Update);//清除标志位
	TIM_Cmd(TIM6,ENABLE);//启动
	return;
}