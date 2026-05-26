#include "stm32f10x.h"
void delay_ms(uint16_t time);
void delay_us(uint16_t time);
void start_count(void);//用来给程序计时，和后面的配套使用
uint32_t stop_count(void);//返回从开始计时到现在的时钟周期数
