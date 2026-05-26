#include "ADS805E.h"
#include "public.h"
//配置PD0-11作为GPIO输入 
//TIM4 CH3作为捕获通道 在PB8 触发DMA传输
//DMA1 channel5 传输数据
uint16_t ADC_Value[BUFFER_SIZE] = {0};
void ADS805E_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {
        .GPIO_Mode = GPIO_Mode_IPD,
        .GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15,
        .GPIO_Speed = GPIO_Speed_50MHz
    };
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = 65536 - 1,
        .TIM_Prescaler = 0,
        .TIM_RepetitionCounter = 0
    };
    TIM_ICInitTypeDef TIM_ICInitStructure = {
        .TIM_Channel = TIM_Channel_3,
        .TIM_ICFilter = 0x00,
        .TIM_ICPolarity = TIM_ICPolarity_Falling,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICSelection = TIM_ICSelection_DirectTI
    };
    DMA_InitTypeDef DMA_InitStructure = {
        .DMA_BufferSize = BUFFER_SIZE,
        .DMA_DIR = DMA_DIR_PeripheralSRC,
        .DMA_M2M = DMA_M2M_Disable,//可能是ENEABLE
        .DMA_MemoryBaseAddr = (uint32_t)ADC_Value,
        .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
        .DMA_MemoryInc = DMA_MemoryInc_Enable,
        .DMA_Mode = DMA_Mode_Normal,
        .DMA_PeripheralBaseAddr = (uint32_t)&GPIOD->IDR,//
        .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
        .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
        .DMA_Priority = DMA_Priority_VeryHigh
    };

    NVIC_InitTypeDef NVIC_InitStructure = {
        .NVIC_IRQChannel = DMA1_Channel5_IRQn,
        .NVIC_IRQChannelCmd = ENABLE,
        .NVIC_IRQChannelPreemptionPriority = 3,
        .NVIC_IRQChannelSubPriority = 3
    };
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    
    TIM_Cmd(TIM4, ENABLE);
    //TIM4 CH3 作为捕获通道,产生触发事件
    //TIM_GenerateEvent(TIM4, TIM_EventSource_CC3);
		TIM_DMACmd(TIM4,TIM_DMA_CC3,ENABLE);
    //TIM_SelectIutputTrigger(TIM4, TIM_TS_TI2FP2);
    TIM_SelectMasterSlaveMode(TIM4, TIM_MasterSlaveMode_Enable);
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);
    NVIC_Init(&NVIC_InitStructure);
    return;
}

void DMA_Start(void){
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel5, BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel5, ENABLE);
    return;
}

