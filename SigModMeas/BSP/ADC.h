#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#define ADC1_CHANNEL ADC_Channel_4
#define ADC2_CHANNEL ADC_Channel_5

void ADC_GPIO_Init(void);
void adc_Init(void);
void ADC_StartDualConversion(void);
uint16_t ADC_GetValue(ADC_TypeDef* ADCx);
void ADC_GetDualValues(uint16_t* adc1_val, uint16_t* adc2_val);
