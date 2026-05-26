#include "stm32f4xx.h"
#include "DAC.h"


void GPIO_Configuration(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    
    
    /* 启用GPIOE时钟（外部触发引脚PE9） */
    RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    
    /* 配置PE9为浮空输入（外部触发信号） */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}
//DAC1; PA4; DMA_CH7; DMA1_Stream5; TIM4;
void DAC1_Init(){
	//结构体定义
	GPIO_InitTypeDef  GPIO_InitStructure;	//GPIO结构体
	DAC_InitTypeDef DAC_InitStructure;		//DAC结构体
	DMA_InitTypeDef DMA_InitStructure;		//DMA结构体
	
	//时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);		//使能DAC时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);		//使能DMA1时钟
	
	//GPIO配置初始化 PA4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				//PIN4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;			//模拟模式		
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//GPIO结构体初始化
	
	//DAC配置初始化
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_Ext_IT9; 			//！！！定时器触发转换 一次转移数组里的一个数据 TIM4
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);					//DAC1初始化
		//DAC设置数据对齐
	DAC_SetChannel1Data(DAC_Align_12b_R, 0);						//右对齐


	//DMA配置初始化
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;				//DMA_CH7
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 			//循环模式
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;		//内存到外设
	DMA_InitStructure.DMA_BufferSize = WAVE_LENGTH;		//传输次数:数组长度
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;		//优先级
		//DMA_FIFO
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ch1Wave; 		//内存地址:输出DA值数组
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//16 bit
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;				//内存自增

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(DAC->DHR12R1); 		//外设地址:DAC1地址
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//16 bit
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设不自增
	DMA_Init(DMA1_Stream5,&DMA_InitStructure);									//DMA1_Stream5 初始化
	
	DMA_Cmd(DMA1_Stream5,ENABLE);		//DMA1_Stream5 使能
	DAC_DMACmd(DAC_Channel_1, ENABLE);	//DAC1_DMA 使能  	
	DAC_Cmd(DAC_Channel_1,ENABLE); 		//DAC1 使能

}
void EXTI_Configuration(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 连接EXTI线9到PE9 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource9);
    /* 配置EXTI线9 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  // 中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // 上升沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;            // 启用线9
    EXTI_Init(&EXTI_InitStructure);
    
    /* 配置NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;   // EXTI9_5中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;    // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       // 启用中断
    NVIC_Init(&NVIC_InitStructure);
}
//DAC1; PA5; DMA_CH7; DMA1_Stream6; TIM5;
void DAC2_Init(){
	//结构体定义
	GPIO_InitTypeDef  GPIO_InitStructure;	//GPIO结构体
	DAC_InitTypeDef DAC_InitStructure;		//DAC结构体
	DMA_InitTypeDef DMA_InitStructure;      //DMA结构体

	//时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);     //使能DAC时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);     //使能DMA1时钟
	
	//GPIO配置初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				//PIN5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;            //模拟模式		
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        //浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);                  //GPIO结构体初始化
	
	//DAC配置初始化
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_Ext_IT9;					//！！！定时器触发转换 一次转移数组里的一个数据 TIM5
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;         
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);                            //DAC2初始化
		//DAC设置数据对齐
	DAC_SetChannel2Data(DAC_Align_12b_R, 0);								//右对齐
	
	
	//DMA配置初始化
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;							//DMA_CH7
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         //循环模式
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                 //内存到外设
	DMA_InitStructure.DMA_BufferSize = WAVE_LENGTH;                    //传输次数:数组长度
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //优先级
		//DMA_FIFO
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ch2Wave;			//内存地址:输出DA值数组
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;     //16 bit
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //内存自增

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(DAC->DHR12R2);			//外设地址:DAC2地址
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;             
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;     //16 bit
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                //外设不自增
	DMA_Init(DMA1_Stream6,&DMA_InitStructure);			                            //DMA1_Stream6 初始化 
	
	DMA_Cmd(DMA1_Stream6,ENABLE);				//DMA1_Stream6 使能
	DAC_DMACmd(DAC_Channel_2, ENABLE);          //DAC2_DMA 使能  	
	DAC_Cmd(DAC_Channel_2,ENABLE);              //DAC2 使能
}

void DAC_DMA_Reconfig(DMA_Stream_TypeDef* DMAy_Streamx,uint16_t* data,uint16_t length){//重配置
	DMA_Cmd(DMAy_Streamx, DISABLE);	//关闭DMA传输
	DMA_MemoryTargetConfig(DMAy_Streamx,(uint32_t)&data[0],DMA_Memory_0);
	DMA_SetCurrDataCounter(DMAy_Streamx, length);	//设置DMA传输数据长度
	DMA_Cmd(DMAy_Streamx,ENABLE);
	return;
}

//获取输出数据
uint16_t DAC1_GetValue(){

	return DAC_GetDataOutputValue(DAC_Channel_1);

}

uint16_t DAC2_GetValue(){

	return DAC_GetDataOutputValue(DAC_Channel_2);

}
