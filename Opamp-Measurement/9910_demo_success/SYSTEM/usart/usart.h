#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 

//STM32F103ZE核心板
//串口1初始化		   

//********************************************************************************

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
extern int uart1_res;
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern int uart5_res;
extern int state;
extern uint32_t output_freq;    
extern uint16_t output_amp;
extern int istest;
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void USART2_Init(u32 My_BaudRate);
void usart3_init(u32 bound);
void HMISendstart(void);
void HMISendb(u8 k)		;
void HMISends(char *buf1)	;
void UART5_init(u32 bound);
//void UART4_Init(u32 bound);
#endif


