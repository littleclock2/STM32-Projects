#include "uart.h"
void USART1_Init(u32 bound)
{
	/**一、配置串口***/
    //1、使能串口及复用引脚外设的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    
/**二、串口引脚复用映射***/
    //映射Tx
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
    //映射Rx
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
    
/**三、定义GPIO结构体变量，初始化引脚模式为复用，复用为Pin_10: rx、Pin_9:tx***/
    GPIO_InitTypeDef GPIO_InitStructure_usart1;
    
    GPIO_InitStructure_usart1.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
    GPIO_InitStructure_usart1.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure_usart1.GPIO_Speed = GPIO_High_Speed;
    GPIO_InitStructure_usart1.GPIO_OType = GPIO_OType_PP; //推挽复用输出
    GPIO_InitStructure_usart1.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOA,&GPIO_InitStructure_usart1);
    
/**四、定义串口结构体，初始化串口：这里初始化 USART1***/
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = bound;  
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
    USART_InitStruct.USART_Mode = USART_Mode_Tx|USART_Mode_Rx; 
    USART_InitStruct.USART_Parity = USART_Parity_No; 
    USART_InitStruct.USART_StopBits = USART_StopBits_1; 
    //配置 ：9600波特率、8位字长、无校验、1位停止位、物流控、发送和接收模式
    USART_Init(USART1, &USART_InitStruct);
    
/**五、使能串口***/
    USART_Cmd(USART1,ENABLE);
    
/**六、配置接收中断 ***/

    //1、中断方法
    //1.1、配置串口中断(开启标志位到NVIC的输出)
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
    //1.2、配置NVIC中断向量（中断优先级）
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置系统中断优先级分组
    //1.3、初始化中断
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn ; //中断是：串口1中断
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3; //子优先级
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStruct);  
}
