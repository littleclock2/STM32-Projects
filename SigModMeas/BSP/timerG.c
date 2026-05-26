#include "timerG.h"
#include "misc.h"

typedef struct {
    volatile uint32_t last_capture;
    volatile uint32_t period;
    volatile uint32_t last_systick;
    volatile uint8_t valid;
} ChannelData;

static ChannelData ch_data[2] = {0};
FreqMeasureResult result = {0, 0};
void TIM3_IRQHandler(void)
{
    // 通道1(PC6)处理
    if(TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)//
    {
        uint16_t capture = TIM_GetCapture1(TIM3);//
        ch_data[0].period = capture - ch_data[0].last_capture;//
        ch_data[0].last_capture = capture;//
        ch_data[0].last_systick = SysTick->VAL;//
        ch_data[0].valid = 1;//
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);//
    }
    
    // 通道2(PC7)处理
    if(TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
    {
        uint16_t capture = TIM_GetCapture2(TIM3);
        ch_data[1].period = capture - ch_data[1].last_capture;
        ch_data[1].last_capture = capture;
        ch_data[1].last_systick = SysTick->VAL;
        ch_data[1].valid = 1;
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
    }
}

void FreqMeasure_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 2. 配置GPIOC
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3);

    // 3. 定时器基础配置
    TIM_TimeBaseStruct.TIM_Prescaler = 84-1;  // 84MHz/84=1MHz
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_Period = 0xFFFF;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);

    // 4. 输入捕获配置
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICFilter = 0xF;
    TIM_ICInit(TIM3, &TIM_ICInitStruct);
    
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM3, &TIM_ICInitStruct);

    // 5. 中断配置
    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_CC2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

FreqMeasureResult FreqMeasure_GetValue(FreqChannel ch)
{
    
    uint32_t current_tick = SysTick->VAL;
    
    if((current_tick - ch_data[ch].last_systick) < (SystemCoreClock/8/2)){
        if(ch_data[ch].period != 0 && ch_data[ch].valid){
            result.frequency = 1000000 / ch_data[ch].period;
            result.valid = 1;
        }
    }
    else{
        ch_data[ch].valid = 0;
    }
    return result;
}
