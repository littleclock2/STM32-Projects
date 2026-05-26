#define Length 1024
#include "stm32f4xx.h"


#define WAVE_LENGTH 1024 // ÍÆ¼ö1024µã²¨ÐÎ

extern uint16_t ch1Wave[WAVE_LENGTH];
extern uint16_t ch2Wave[WAVE_LENGTH];

void DAC1_Init(void);
void DAC2_Init(void);
void EXTI_Configuration(void);
void GPIO_Configuration(void);
void DAC_DMA_Reconfig(DMA_Stream_TypeDef* DMAy_Streamx,uint16_t* data,uint16_t length);
uint16_t DAC1_GetValue(void);
uint16_t DAC2_GetValue(void);
