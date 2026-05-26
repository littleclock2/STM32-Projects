#include "ADC_TIM.h"
#include "public.h"
/*配置PC2作为ADC的输入通道
* TIM7 作为定时器触发ADC采样
* DAM_Channel1 搬运数据
*/

uint16_t DMA_dest[BUFFER_SIZE];
void ADCTimingInit(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	ADC_InitTypeDef ADC_InitStruct = {
		.ADC_ContinuousConvMode = DISABLE,
		.ADC_DataAlign = ADC_DataAlign_Right,
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO ,
		.ADC_Mode = ADC_Mode_Independent,
		.ADC_NbrOfChannel = 1,
		.ADC_ScanConvMode = DISABLE
	};
	
	GPIO_InitTypeDef GPIO_InitStruct = {
		.GPIO_Mode = GPIO_Mode_AIN,
		.GPIO_Pin = GPIO_Pin_2,
		.GPIO_Speed = GPIO_Speed_50MHz
	};
	
	NVIC_InitTypeDef DMA_NVICInitStruct = {
		.NVIC_IRQChannel = DMA1_Channel1_IRQn,
		.NVIC_IRQChannelCmd = ENABLE,
		.NVIC_IRQChannelPreemptionPriority = 2,
		.NVIC_IRQChannelSubPriority = 3
	};

	DMA_InitTypeDef DMA_InitStruct  ={ 
		.DMA_BufferSize = BUFFER_SIZE,
		.DMA_DIR = DMA_DIR_PeripheralSRC,
		.DMA_M2M = DISABLE,
		.DMA_MemoryBaseAddr = (u32)DMA_dest,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_Mode = DMA_Mode_Normal,
		.DMA_PeripheralBaseAddr = (u32)&ADC1->DR,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_Priority = DMA_Priority_Medium
	};
	
	GPIO_PinRemapConfig(GPIO_Remap_ADC1_ETRGREG,ENABLE);
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);
	ADC_Init(ADC1,&ADC_InitStruct);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_13Cycles5);
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);
	NVIC_Init(&DMA_NVICInitStruct);
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);
	DMA_ClearITPendingBit(DMA1_IT_TC1);
	DMA_SetCurrDataCounter(DMA1_Channel1,BUFFER_SIZE);
	ADC_Cmd(ADC1,ENABLE);
	ADC_StartCalibration(ADC1);
	
	while(ADC_GetCalibrationStatus(ADC1) != SET)
		continue;
	
	
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	//ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	return;
}

void ADC_TimerSet(uint16_t psc,uint16_t arr){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
	TIM_TimeBaseInitTypeDef TIM_InitStruct = {
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = arr - 1,
		.TIM_Prescaler = psc - 1,
	};
	
	TIM_TimeBaseInit(TIM8,&TIM_InitStruct);
	TIM_SelectOutputTrigger(TIM8,TIM_TRGOSource_Update);
	TIM_Cmd(TIM8,ENABLE);
	return;
}

void ADC_EXTIInit(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	ADC_InitTypeDef ADC_InitStruct = {
		.ADC_ContinuousConvMode = DISABLE,
		.ADC_DataAlign = ADC_DataAlign_Right,
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO,
		.ADC_Mode = ADC_Mode_Independent,
		.ADC_NbrOfChannel = 1,
		.ADC_ScanConvMode = DISABLE
	};
	GPIO_InitTypeDef GPIO_InitStruct = {
		.GPIO_Mode = GPIO_Mode_AIN,
		.GPIO_Pin = GPIO_Pin_2,
		.GPIO_Speed = GPIO_Speed_50MHz
	};
	DMA_InitTypeDef DMA_InitStruct = {
		.DMA_BufferSize = BUFFER_SIZE,
		.DMA_DIR = DMA_DIR_PeripheralSRC,
		.DMA_M2M = DMA_M2M_Disable,
		.DMA_MemoryBaseAddr = (u32)DMA_dest,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_Mode = DMA_Mode_Normal,
		.DMA_PeripheralBaseAddr = (u32)&ADC1->DR,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_Priority = DMA_Priority_Medium
	};
	NVIC_InitTypeDef DMA_NVICInitStruct = {
		.NVIC_IRQChannel = DMA1_Channel1_IRQn,
		.NVIC_IRQChannelCmd = ENABLE,
		.NVIC_IRQChannelPreemptionPriority = 2,
		.NVIC_IRQChannelSubPriority = 3
	};
	
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	ADC_Init(ADC1,&ADC_InitStruct);
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);
	NVIC_Init(&DMA_NVICInitStruct);
	GPIO_PinRemapConfig(GPIO_Remap_ADC1_ETRGREG,ENABLE);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_13Cycles5);
	DMA_ITConfig(DMA1_Channel1,DMA1_IT_TC1,ENABLE);
	DMA_ClearITPendingBit(DMA1_IT_TC1);
	ADC_Cmd(ADC1,ENABLE);
	DMA_SetCurrDataCounter(DMA1_Channel1,BUFFER_SIZE);
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1)==SET)
		continue;
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) == SET)
		continue;
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);
}
void EXTIInit(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct = { 
		.GPIO_Mode = GPIO_Mode_IPD,
		.GPIO_Pin = GPIO_Pin_11,
		.GPIO_Speed = GPIO_Speed_50MHz
	};
	EXTI_InitTypeDef EXTI_InitStruct = {
		.EXTI_Line = EXTI_Line11,
		.EXTI_LineCmd = ENABLE,
		.EXTI_Mode = EXTI_Mode_Event,
		.EXTI_Trigger = EXTI_Trigger_Rising
	};
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	EXTI_Init(&EXTI_InitStruct);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource11);
}
