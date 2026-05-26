#pragma once
#include "stm32f10x.h"
void ADCTimingInit(void);
void ADC_TimerSet(uint16_t psc,uint16_t arr);
void ADC_EXTIInit(void);
void EXTIInit(void);
