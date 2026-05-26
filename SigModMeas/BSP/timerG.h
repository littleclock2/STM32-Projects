#include "stm32f4xx.h"

// 测量通道定义
typedef enum {
    FREQ_CH1 = 0,  // PC6 (TIM3_CH1)
    FREQ_CH2       // PC7 (TIM3_CH2)
} FreqChannel;

// 测量结果结构体
typedef struct {
    uint32_t frequency;  // 频率值(Hz)
    uint8_t valid;       // 数据有效标志
} FreqMeasureResult;

void FreqMeasure_Init(void);
FreqMeasureResult FreqMeasure_GetValue(FreqChannel ch);
