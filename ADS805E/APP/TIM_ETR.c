#include "public.h"
#include "TIM_PWM.h"



/*TIM4 CH1 ETR 作为输入通道,对应PE0*/
void TIM_ETR_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

	GPIO_InitTypeDef PA0_InitStructure = {
		.GPIO_Pin = GPIO_Pin_0,
		.GPIO_Mode = GPIO_Mode_IN_FLOATING,
		.GPIO_Speed = GPIO_Speed_50MHz
	};
	
	TIM_TimeBaseInitTypeDef TIM4_BaseInitStructure = {
		.TIM_Prescaler = 1 - 1,
		.TIM_Period = 65536 - 1,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_ClockDivision = TIM_CKD_DIV1
	};
	NVIC_InitTypeDef TIM4_NVIC = {
		.NVIC_IRQChannel = TIM4_IRQn,
		.NVIC_IRQChannelCmd = ENABLE,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 3
	};
	NVIC_Init(&TIM4_NVIC);
	GPIO_Init(GPIOE,&PA0_InitStructure);
	TIM_TimeBaseInit(TIM4,&TIM4_BaseInitStructure);
	TIM_ETRClockMode2Config(TIM4,
													1 - 1,															//分频系数
													TIM_ExtTRGPolarity_NonInverted,			//不反转极性
													1);																	//滤波
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	TIM_ClearFlag(TIM4,TIM_IT_Update);
	TIM_Cmd(TIM4,ENABLE);
}