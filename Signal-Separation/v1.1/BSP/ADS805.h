#ifndef __ADS805_H
#define __ADS805_H

#define Length 1024
#include "stm32f4xx.h"
extern uint16_t ADS805_Data[Length];
extern uint16_t ADS805_Data_Completed;

void ADS805_Init(void);
void Start_ADS805(void);

#endif
