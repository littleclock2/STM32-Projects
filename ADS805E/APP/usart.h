#include "stm32f10x.h"
#include <stdio.h>

void USART1_Init(void);
int fputc(int ch, FILE *f);// 重定向printf到USART1
