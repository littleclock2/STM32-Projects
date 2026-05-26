#include "myfunc.h"
void Delay_us(uint16_t time){
	u32 temp = 0;
	SysTick -> LOAD = time * 9;
	SysTick -> CTRL = SysTick_CTRL_ENABLE_Msk;//使能时钟
	SysTick -> VAL  = 0;//清零计数器
	while(!(temp & SysTick_CTRL_COUNTFLAG_Msk))
		temp = SysTick -> CTRL;;
	return;
}

void Delay_ms(uint16_t time){
	u32 temp = 0;
	SysTick -> LOAD = time * 9000;
	SysTick -> CTRL = SysTick_CTRL_ENABLE_Msk;//使能时钟
	SysTick -> VAL  = 0;//清零计数器
	while(!(temp & SysTick_CTRL_COUNTFLAG_Msk))
		temp = SysTick -> CTRL;;
	return;
}
void start_count(void){//用来给程序计时，和后面的配套使用
	SysTick -> LOAD = 0xffffff;
	SysTick -> CTRL = SysTick_CTRL_ENABLE_Msk;
	SysTick -> VAL = 0;//清空
	return;
}

uint32_t stop_count(void){//返回从开始计时到现在的时钟周期数
	u32 temp = SysTick -> VAL;
	SysTick -> CTRL = 0;
	SysTick -> VAL = 0;//清空
	return 0xffffff - temp;
}
