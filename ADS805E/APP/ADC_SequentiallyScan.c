#include "ADC_single.h"
#include "public.h"
//配置PC0 PC1 PC2 PC3 PC4 PC5 温度传感器 内部参考电压 作为ADC的输入通道 Channel10-17,共8个通道
int16_t ADC_in[8];
void ADC_SequentiallyScan_Init(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//配置12M时钟 ADC时钟最大不大于14M
	GPIO_InitTypeDef ADCIn_InitStruct = {
		.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5,
		.GPIO_Mode = GPIO_Mode_AIN,
		.GPIO_Speed = GPIO_Speed_50MHz
	};
	ADC_InitTypeDef ADC_InitStruct = {
		.ADC_ContinuousConvMode = DISABLE,//使能连续转化模式
		.ADC_DataAlign = ADC_DataAlign_Right,//右对齐
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None,//软件使能转化，以单次触发
		.ADC_Mode = ADC_Mode_Independent,//ADC1 ADC2独立工作
		.ADC_NbrOfChannel = 8,//顺序进行regular转换的通道数
		.ADC_ScanConvMode = ENABLE,//使能扫描模式
	};
	NVIC_InitTypeDef DMA_NVICInitStruct = {
		.NVIC_IRQChannel = DMA1_Channel1_IRQn,//使能ADC1和2共用的中断
		.NVIC_IRQChannelCmd = ENABLE,
		.NVIC_IRQChannelPreemptionPriority = 3,
		.NVIC_IRQChannelSubPriority = 2
	};
	GPIO_Init(GPIOC,&ADCIn_InitStruct);
	ADC_Init(ADC1,&ADC_InitStruct);
	/*配置regular通道组*/
	ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_11,2,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,3,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_13,4,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_14,5,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_15,6,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_16,7,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_17,8,ADC_SampleTime_55Cycles5);
	
	DMA_InitTypeDef DMA_InitStruct =  {
		.DMA_BufferSize = 8,
		.DMA_DIR = DMA_DIR_PeripheralSRC,
		.DMA_M2M = DMA_M2M_Disable,
		.DMA_MemoryBaseAddr = (u32)ADC_in,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_Mode = DMA_Mode_Normal,
		.DMA_PeripheralBaseAddr = (u32)&ADC1->DR,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_Priority = DMA_Priority_Medium,
		
	};
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);
	
	NVIC_Init(&DMA_NVICInitStruct);
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);//使能Channel1中断
	DMA_SetCurrDataCounter(DMA1_Channel1, 8);//给传输计数器负值参数dataNumber:指定给传输计数器写入的值
	DMA_Cmd(DMA1_Channel1,ENABLE);
	
	ADC_Cmd(ADC1,ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1) != SET)
			;
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);//开始转化
	return;
}



