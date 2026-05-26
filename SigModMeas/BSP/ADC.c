#include "adc.h"

void ADC_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOA时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    // 配置PA9-PA12为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void adc_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    
    // 使能ADC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
    
    // ADC通用配置
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; // 独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; // ADC时钟=84MHz/4=21MHz
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
    
    // ADC1配置
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; // 单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // 连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // ADC2配置（与ADC1类似）
    ADC_Init(ADC2, &ADC_InitStructure);
    
    // 配置ADC1通道9
    ADC_RegularChannelConfig(ADC1, ADC1_CHANNEL, 1, ADC_SampleTime_56Cycles);
    
    // 配置ADC2通道10
    ADC_RegularChannelConfig(ADC2, ADC2_CHANNEL, 1, ADC_SampleTime_56Cycles);
    
    // 使能ADC1和ADC2
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);
}

void ADC_StartDualConversion(void)
{
    // 启动双ADC同步规则组转换
    ADC_SoftwareStartConv(ADC1);
    ADC_SoftwareStartConv(ADC2);
}

uint16_t ADC_GetValue(ADC_TypeDef* ADCx)
{
    while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC)); // 等待转换完成
    return ADC_GetConversionValue(ADCx);
}

void ADC_GetDualValues(uint16_t* adc1_val, uint16_t* adc2_val)
{
    *adc1_val = ADC_GetValue(ADC1);
    *adc2_val = ADC_GetValue(ADC2);
}
