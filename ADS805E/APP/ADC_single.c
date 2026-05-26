#include "ADC_single.h"
#include "public.h"
//配置PC2作为ADC的输入通道 Channel12
void ADC_Single_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	GPIO_InitTypeDef ADCIn_InitStruct = {
		.GPIO_Pin = GPIO_Pin_2,
		.GPIO_Mode = GPIO_Mode_AIN,
		.GPIO_Speed = GPIO_Speed_50MHz
	};
	ADC_InitTypeDef ADC_InitStruct = {
		.ADC_ContinuousConvMode = DISABLE,//使能单次转化模式(失能连续转化模式)
		.ADC_DataAlign = ADC_DataAlign_Right,//右对齐
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None,//软件使能转化，以单次触发
		.ADC_Mode = ADC_Mode_Independent,//ADC1 ADC2独立工作
		.ADC_NbrOfChannel = 1,//顺序进行regular转换的通道数
		.ADC_ScanConvMode = DISABLE,//失能扫描模式
	};
	NVIC_InitTypeDef ADC_NVICInitStruct = {
		.NVIC_IRQChannel = ADC1_2_IRQn,//使能ADC1和2共用的中断
		.NVIC_IRQChannelCmd = ENABLE,
		.NVIC_IRQChannelPreemptionPriority = 2,
		.NVIC_IRQChannelSubPriority = 2
	};
	GPIO_Init(GPIOC,&ADCIn_InitStruct);
	ADC_Init(ADC1,&ADC_InitStruct);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_13Cycles5);//配置regular通道
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//配置12M时钟 ADC时钟最大不大于14M
	NVIC_Init(&ADC_NVICInitStruct);
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);//regular转化完成中断
	ADC_Cmd(ADC1,ENABLE);
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) != SET)
			;
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);//开始转化
	return;
}
