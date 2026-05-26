#include "stm32f4xx.h"  // STM32F4标准外设库
#include "bsp_led.h"    // LED驱动头文件
#include "Delay.h"      // 延时函数头文件
#include "SI5351_Soft.h" // SI5351时钟芯片驱动
#include "ADS805.h"     // ADS805 ADC驱动
#include <stdio.h>
#include <stdbool.h>
#include "math.h"
#include "arm_math.h"   // ARM数学库
#include <stdlib.h>
#include <stdint.h>
#include "DAC.h" 
#include "TIM_ETR.h"
#include "stdarg.h"	
/* 引脚分配：
    SI5351 I2C引脚：PB6(SCL), PB7(SDA)
    LED引脚：PA15
    ADS805引脚：PD0-PD15,PA8作为TIM1_CH1
    DAC触发：PE9；DAC1:PA4； DAC2:PA5
    TIM4 ETR引脚：PE0, PB9 已验证
    TIM3 ETR引脚：PC9, PD2 
    TIM2 ETR引脚：PA0, PA3
*/
// 宏定义
#define FFT_ADC_N 4096  // FFT采样点数
#define FFT_SIN_BASE 98000  // 正弦波基准值
#define FFT_TRI_BASE 76100  // 三角波基准值
#define FFT_TRI3_BASE 10000 // 三次谐波基准值
#define WAVE_LENGTH 1024    // 波形数据长度
#define DAC_RESOLUTION 4096 // 12位DAC分辨率
#define TRIGGER_FREQ 1000000   // 触发频率1MHz

// 通信变量及宏定义
u8  UART4_RX_BUF[30]; 				//接收缓冲,最大UART4_MAX_RECV_LEN个字节.
u8  UART4_TX_BUF[30]; 			//发送缓冲,最大UART4_MAX_SEND_LEN字节
vu16 UART4_RX_STA=0; 

// 波形类型枚举
typedef enum {
    WAVE_SINE,      // 正弦波
    WAVE_TRIANGLE,  // 三角波
} WaveType;
//DAC相关配置变量
    // 通道配置结构体
    typedef struct {
        WaveType type;   // 波形类型
        float frequency; // 频率(Hz)
        uint16_t effectivePoints; // 有效点数
        uint16_t *waveData; // 波形数据指针
    } ChannelConfig;

    // 系统配置结构体
    typedef struct {
        ChannelConfig ch1; // 通道1配置
        ChannelConfig ch2; // 通道2配置
        float phaseDiff;   // 相位差(度)
    } SystemConfig;

    // 全局变量
    uint16_t ch1Wave[WAVE_LENGTH]; // 通道1波形数据
    uint16_t ch2Wave[WAVE_LENGTH]; // 通道2波形数据
    uint16_t ch1_valid_length = WAVE_LENGTH; // 通道1有效长度
    uint16_t ch2_valid_length = WAVE_LENGTH; // 通道2有效长度
    bool needUpdate = false;       // 更新标志

// 外部引用
extern const arm_cfft_instance_f32 arm_cfft_sR_f32_len4096; // ARM FFT实例
extern uint16_t ADS805_Data[Length];  // ADC数据缓冲区
extern uint16_t ADS805_Data_Completed; // ADC采集完成标志

// FFT相关变量
float fft_in_adc_data[FFT_ADC_N * 2] = {0}; // FFT输入数据(复数形式)
float fft_out_adc_data[FFT_ADC_N] = {0};    // FFT输出幅度
float fft_max_value_1 = 0;    // FFT最大幅值1
u32 fft_max_value_index_1 = 0; // FFT最大幅值索引1
float fft_max_value_2 = 0;    // FFT最大幅值2
u32 fft_max_value_index_2 = 0; // FFT最大幅值索引2
float f_s = 1.024e6;          // 采样频率1.024MHz

// 比较器相关
u16 comper = 0;
u16 comper_array[16] = {0};

// 信号类型标志
bool signal_type = false; // 信号类型标志
u8 signal_mode_1 = 0;      // 信号1模式
u8 signal_mode_2 = 0;      // 信号2模式
u32 signal1_freq = 0; // 信号1频率
u32 signal2_freq = 0; // 信号2频率
// 相位差
u16 phase = 0;         // 相位差
// 函数声明
void FFT_Deal(uint16_t adc_value[], float *fft_input, int data_length);// FFT处理函数
void u4_printf(char* fmt,...);//串口函数生成
void uart4_init(u32 bound);
void SendData(void);
void DataMeasure(void); // 测量数据
//DAC相关函数
void WaveGen_Init(SystemConfig *config);// 波形生成初始化
void GenerateTriangleWave(uint16_t *buffer, uint16_t points, uint16_t amplitude, uint16_t offset);// 生成三角波
void GenerateSineWave(uint16_t *buffer, uint16_t points, uint16_t amplitude, uint16_t offset);// 生成正弦波
void GenerateWaveform(ChannelConfig *ch);// 生成波形
void ApplyPhaseDifference(SystemConfig *config);// 应用相位差
void UpdatePhaseDifference(float degrees);// 更新相位差
void UpdateChannel1(SystemConfig *cfg,WaveType type, float freq);// 更新通道1波形
void UpdateChannel2(SystemConfig *cfg,WaveType type, float freq);// 更新通道2波形

// 主函数
int main(void){
    SystemConfig cfg; // 系统配置
    
    // 硬件初始化
    SystemInit();// 系统初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组

    TIM_ETR_Init32();// 初始化定时器ETR为32分频
    TIM_ETR_Init16();// 初始化定时器ETR为16分频
    TIM_ETR_InitCustom(16);// 初始化定时器ETR为16分频

    ADS805_Init();// 初始化ADS805 ADC
    Start_ADS805();// 启动ADS805 ADC采集
    Si5351Init();// 初始化SI5351时钟芯片
    Led_Init();  // 初始化LED

    DAC1_Init();// 初始化DAC1
    DAC2_Init();// 初始化DAC2

	uart4_init(115200);//初始化串口	

	DMA_Cmd(DMA1_Stream5, DISABLE);	//关闭DMA传输
	DMA_Cmd(DMA1_Stream6, DISABLE);
	WaveGen_Init(&cfg);// 波形生成初始化
	DMA_Cmd(DMA1_Stream5, ENABLE);	//开启DMA传输
	DMA_Cmd(DMA1_Stream6, ENABLE);
	
    EXTI_Configuration();// 外部中断配置
    GPIO_Configuration();// GPIO配置
    
    
    // 设置SI5351输出频率
    SetFrequency(f_s, 0);
    SetFrequency(f_s, 1);
    SetFrequency(f_s, 2);
    
    delay_ms(1000); // 延时1秒
    
    while (1){  // 主循环
        LED_On();       // LED亮
        delay_ms(50);  // 延时500ms
        LED_Off();      // LED灭
        delay_ms(50);  // 延时500ms
        
        if (ADS805_Data_Completed) { // ADC数据采集完成
            // FFT处理
            FFT_Deal(ADS805_Data, fft_in_adc_data, FFT_ADC_N);
            arm_cfft_f32(&arm_cfft_sR_f32_len4096, fft_in_adc_data, 0, 1);
            arm_cmplx_mag_f32(fft_in_adc_data, fft_out_adc_data, FFT_ADC_N);
            fft_out_adc_data[0] = 0; // 去除直流分量
            
             // 获取FFT最大幅值及其索引
            arm_max_f32(fft_out_adc_data, FFT_ADC_N/2, &fft_max_value_1, &fft_max_value_index_1);
			
            // 检测信号特征
			comper = 0;//归零比较变量
            for(u16 i = 0; i < FFT_ADC_N/2; i++){
                if(fft_out_adc_data[i] > 100000){
                    comper_array[comper] = i;//记录比较器索引
                    comper++;
                }
            }
            
			DataMeasure(); // 测量数据
            
            // // 计算相位差
            // if(!signal_type){ // 简单信号
            //     if(signal_mode == 1 && phase != 180) // 正弦波
            //         phase = acos(fft_max_value_1/(2*FFT_SIN_BASE));
            //     else if(signal_mode == 0 && phase != 180) // 三角波
            //         phase = atan2f(fft_max_value_1, fft_out_adc_data[fft_max_value_index_1+1]);
            // }
            // else if(signal_type){ // 复合信号
            //     if(signal_mode){ // 正弦波
            //         float temp = (double)FFT_SIN_BASE*FFT_SIN_BASE + (double)FFT_TRI_BASE*FFT_TRI_BASE - (double)fft_max_value_1*fft_max_value_1;
            //         phase = acos(temp/(2.0*FFT_SIN_BASE*FFT_TRI_BASE));
            //     }
            //     else{ // 三角波
            //         phase = atan2f(fft_max_value_1, fft_out_adc_data[fft_max_value_index_1+1]);
            //     }
            // }
            SetFrequency((signal1_freq+signal2_freq)*16000,2);
            SendData(); // 发送数据到串口
            // 重新启动ADC采集
            Start_ADS805();
            
            if(needUpdate){ // 需要更新波形
								DMA_Cmd(DMA1_Stream5, DISABLE);	//关闭DMA传输
								DMA_Cmd(DMA1_Stream6, DISABLE);
                UpdateChannel1(&cfg,WAVE_SINE, 20000.0f);    // 通道1更新为20kHz正弦波
                UpdateChannel2(&cfg,WAVE_TRIANGLE, 50000.0f); // 通道2更新为50kHz三角波
                UpdatePhaseDifference(90.0f);           // 更新相位差为90度
                needUpdate = 0;
								DMA_Cmd(DMA1_Stream5, ENABLE);	//开启DMA传输
								DMA_Cmd(DMA1_Stream6, ENABLE);
            }
        }
        delay_ms(10);
    }
}

void DataMeasure(void){ // 测量数据
    //根据FFT结果测频并得出两个信号频率
    signal1_freq = comper_array[0]/4; // 信号1频率
    // 判断信号类型与信号频率位置，signal_2一定大于signal_1
    if(comper == 2){//等于三时为复合信号一三角一正弦
        //signal_type = true; // 复合信号
        u8 temp = 0;
        for(u8 i = 0; i < comper; i++){//寻找最大幅值索引在比较数组中的位置
            if(comper_array[i] == fft_max_value_index_1){
                temp = i;
            }
        }
        if(fft_out_adc_data[3*comper_array[0]] > 20000 && fft_out_adc_data[3*comper_array[1]] > 20000){
            signal_mode_1 = 2; // 三角波
            signal_mode_2 = 2; // 三角波
        }
        else if(abs((int)fft_out_adc_data[comper_array[0]] - (int)fft_out_adc_data[comper_array[1]]) < 20000){
            signal_mode_1 = 1; // 正弦波
            signal_mode_2 = 1; // 正弦波
        }
        else if(temp == 0){//最大幅值在第一个位置
            signal_mode_1 = 1; // 正弦波
            signal_mode_2 = 2; // 三角波
        }
        else {//最大幅值不在第一个位置
            signal_mode_1 = 2; // 三角波
            signal_mode_2 = 1; // 正弦波
        }
        
    }
    else if(comper == 1){//等于一时一定为同频正弦波信号
        if(fft_out_adc_data[3*comper_array[0]] > 20000  ){//检测三倍频的谐波分量
            if(fft_out_adc_data[3*comper_array[0]] < 50000){//谐波分量过大，则存在两个三角波
                signal_mode_1 = 1; // 正弦波
                signal_mode_2 = 2; // 三角波
            }
            else{
                signal_mode_1 = 2; // 三角波
                signal_mode_2 = 2; // 三角波
            }
            
        }
        else{
            signal_mode_1 = 1; // 正弦波
            signal_mode_2 = 1; // 正弦波
        }
        
    }            
    if(comper == 2){ // 如果有第二个信号
        signal2_freq = comper_array[1]/4; // 信号2频率
    } 
    else {
        signal2_freq = signal1_freq;
    }
}
// 串口数据发送函数
void SendData(void){//发送数据函数
	// 发送状态信息
	char str[40]={0};
	//char short_str[10] = {0};
	char *pt = str;
	//double temp = 0;
	// 发送信号1的类型及频率
	switch(signal_mode_1){
        case 0:
            sprintf(str,"t1.txt=\"\\\"\xff\xff\xff");   
            break;
		case 1://正弦波
			sprintf(str,"t1.txt=\"正弦波\"\xff\xff\xff");
			break;
		case 2://三角波
			sprintf(str,"t1.txt=\"三角波\"\xff\xff\xff");
			break;
		default:
			break;
	}
	while(*pt){
		USART_SendData(UART4,*pt++);
		delay_us(200);
    }
	delay_ms(10);

	pt = str;
     // 发送信号2的类型及频率
	switch(signal_mode_1){
        case 0:
            sprintf(str,"t3.txt=\"\\\"\xff\xff\xff");   
            break;
		case 1://正弦波
			sprintf(str,"t3.txt=\"正弦波\"\xff\xff\xff");
			break;
		case 2://三角波
			sprintf(str,"t3.txt=\"三角波\"\xff\xff\xff");
			break;
		default:
			break;
	}
	while(*pt){
		USART_SendData(UART4,*pt++);
		delay_us(200);
    }
	delay_ms(10);

    pt = str;
     // 发送信号2的类型及频率
	if(signal_mode_1!=0){
        sprintf(str,"t6.txt=\"%dkHz\"\xff\xff\xff", signal1_freq);
    }
	while(*pt){
		USART_SendData(UART4,*pt++);
		delay_us(200);
    }
	delay_ms(10);

    pt = str;
     // 发送信号2的类型及频率
	if(signal_mode_2!=0){
        sprintf(str,"t7.txt=\"%dkHz\"\xff\xff\xff", signal2_freq);
    }
	while(*pt){
		USART_SendData(UART4,*pt++);
		delay_us(200);
    }
	delay_ms(10);
}

// FFT数据处理函数
void FFT_Deal(uint16_t adc_value[], float *fft_input, int data_length) {
    for(int i = 0; i < data_length; i++) {
        fft_input[2 * i] = (float)adc_value[i]; // 实部
        fft_input[2 * i + 1] = 0;              // 虚部
    }
}

// 波形发生器初始化
void WaveGen_Init(SystemConfig *config) {
	// 确保effectivePoints是WAVE_LENGTH的约数
    config->ch1.effectivePoints = WAVE_LENGTH / 2;  // 示例：4个完整周期
    config->ch2.effectivePoints = WAVE_LENGTH / 2;  // 示例：2个完整周期
	
    // 初始化通道1配置
    config->ch1.type = WAVE_SINE;
    config->ch1.frequency = 10000; // 10kHz
    config->ch1.waveData = ch1Wave;
    
    // 初始化通道2配置
    config->ch2.type = WAVE_TRIANGLE;
    config->ch2.frequency = 10000; // 10kHz
    config->ch2.waveData = ch2Wave;
    config->ch2.waveData[WAVE_LENGTH-1] = config->ch2.waveData[0];
    config->phaseDiff = 0; // 初始相位差0度
    
    // 计算有效点数
    config->ch1.effectivePoints = (uint16_t)(TRIGGER_FREQ / config->ch1.frequency + 0.5f);
    config->ch2.effectivePoints = (uint16_t)(TRIGGER_FREQ / config->ch2.frequency + 0.5f);
		DMA_MemoryTargetConfig(DMA1_Stream5,(uint32_t)&config->ch1.waveData[0],DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream5, config->ch1.effectivePoints);	//设置DMA传输数据长度
		DMA_MemoryTargetConfig(DMA1_Stream6,(uint32_t)&config->ch2.waveData[0],DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream6, config->ch2.effectivePoints);	//设置DMA传输数据长度
    // 生成波形
    GenerateWaveform(&config->ch1);
    GenerateWaveform(&config->ch2);
    ApplyPhaseDifference(config);
}

// 生成波形函数
void GenerateWaveform(ChannelConfig *ch) {
    uint16_t amplitude = (uint16_t)(0.8f * DAC_RESOLUTION / 2); // 幅值
    uint16_t offset = DAC_RESOLUTION / 2; // 直流偏置
    
    switch(ch->type) {
        case WAVE_SINE:
            GenerateSineWave(ch->waveData, ch->effectivePoints, amplitude, offset);
            break;
        case WAVE_TRIANGLE:
            GenerateTriangleWave(ch->waveData, ch->effectivePoints, amplitude, offset);
            break;
    }
}

// 生成完美连续正弦波
void GenerateSineWave(uint16_t *buffer, uint16_t points, uint16_t amplitude, uint16_t offset) {
    // 参数严格校验
    points = (points == 0 || points > WAVE_LENGTH) ? WAVE_LENGTH : points;
    const float cycles = (float)WAVE_LENGTH / (float)points; // 完整周期数
    
    for(int i = 0; i < WAVE_LENGTH; i++) {
        // 使用双精度计算避免累积误差
        double phase = 2.0 * PI * (double)i / (double)points;
        float value = sinf((float)phase);
        
        // 幅度限制保护
        value = fmaxf(-1.0f, fminf(1.0f, value));
        buffer[i] = (uint16_t)(offset + amplitude * value);
    }
    
    // 数学保证闭合性（无需强制赋值）
}

// 生成平滑三角波（改进算法）
void GenerateTriangleWave(uint16_t *buffer, uint16_t points, uint16_t amplitude, uint16_t offset) {
    points = (points == 0 || points > WAVE_LENGTH) ? WAVE_LENGTH : points;
    const float half_amplitude = (float)amplitude;
    const float period = (float)points;
    
    for(int i = 0; i < WAVE_LENGTH; i++) {
        float pos = fmodf((float)i, period) / period; // 归一化位置
        
        // 使用分段线性插值（优化后的算法）
        float value;
        if (pos < 0.25f) {
            value = pos * 4.0f;
        } else if (pos < 0.75f) {
            value = 2.0f - pos * 4.0f;
        } else {
            value = -4.0f + pos * 4.0f;
        }
        
        buffer[i] = (uint16_t)(offset + half_amplitude * value);
    }
}

// 相位差应用（硬件级精确实现）
void ApplyPhaseDifference(SystemConfig *config) {
    if(config->ch2.effectivePoints == 0) return;
    
    // 计算实际需要移动的采样点数
    uint32_t shiftPoints = (uint32_t)((config->phaseDiff / 360.0f) * config->ch2.effectivePoints);
    shiftPoints %= config->ch2.effectivePoints; // 确保在有效范围内
    
    // 环形缓冲区实现
    uint16_t tempBuffer[WAVE_LENGTH];
    uint16_t *src = config->ch2.waveData;
    uint16_t effectivePts = config->ch2.effectivePoints;
    
    // 分段复制保证内存连续性
    for(int i = 0; i < WAVE_LENGTH; i++) {
        uint32_t srcPos = (i + shiftPoints) % effectivePts;
        tempBuffer[i] = src[srcPos];
    }
    
    // 内存对齐拷贝（ARM优化）
    memcpy(config->ch2.waveData, tempBuffer, WAVE_LENGTH * sizeof(uint16_t));
    
    // 闭合性数学保证
    if(config->ch2.effectivePoints < WAVE_LENGTH) {
        uint16_t cycles = WAVE_LENGTH / config->ch2.effectivePoints;
        for(int c = 1; c <= cycles; c++) {
            uint16_t start = c * config->ch2.effectivePoints;
            if(start < WAVE_LENGTH) {
                memcpy(&config->ch2.waveData[start], 
                      config->ch2.waveData,
                      config->ch2.effectivePoints * sizeof(uint16_t));
            }
        }
    }
}


// 更新通道1配置
void UpdateChannel1(SystemConfig *cfg,WaveType type, float freq) {    
    cfg->ch1.type = type;
    cfg->ch1.frequency = fmaxf(5000, fminf(100000, freq)); // 限制频率范围5kHz-100kHz
    cfg->ch1.effectivePoints = (uint16_t)(TRIGGER_FREQ / cfg->ch1.frequency + 0.5f);
    GenerateWaveform(&cfg->ch1);
		DMA_MemoryTargetConfig(DMA1_Stream5,(uint32_t)&cfg->ch1.waveData[0],DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream5, cfg->ch1.effectivePoints);	//设置DMA传输数据长度
    ApplyPhaseDifference(cfg);
}

// 更新通道2配置
void UpdateChannel2(SystemConfig *cfg,WaveType type, float freq) {
    cfg->ch2.type = type;
    cfg->ch2.frequency = fmaxf(5000, fminf(100000, freq)); // 限制频率范围5kHz-100kHz
    cfg->ch2.effectivePoints = (uint16_t)(TRIGGER_FREQ / cfg->ch2.frequency + 0.5f);
    GenerateWaveform(&cfg->ch2);
		DMA_MemoryTargetConfig(DMA1_Stream6,(uint32_t)&cfg->ch2.waveData[0],DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream6, cfg->ch2.effectivePoints);	//设置DMA传输数据长度
    ApplyPhaseDifference(cfg);
}

// 更新相位差
void UpdatePhaseDifference(float degrees) {
    SystemConfig *cfg;
    cfg->phaseDiff = fmaxf(0, fminf(180, degrees)); // 限制相位差范围0-180度
    ApplyPhaseDifference(cfg);
}


// 外部中断处理函数
void EXTI9_5_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line9) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line9); // 清除中断标志
        // 可以添加DMA相关处理代码
    }
}

void uart4_init(u32 bound){
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//使能USART4时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4); //GPIOC11复用为UART4
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4); //GPIOC10复用为UART4	
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10; //GPIOC11和GPIOC10初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); //初始化GPIOC端口

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART4, &USART_InitStructure); //初始化串口4
	
	USART_Cmd(UART4, ENABLE);  //使能串口4 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
		
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启中断 

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器	
}

void UART4_IRQHandler(void){                	//串口4中断服务程序
	u8 Res;   
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET){  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		Res =USART_ReceiveData(UART4);//(USART1->DR);	//读取接收到的数据
		if(Res==0x00){//接收到0x00,不处理
            return; //如果是0x00,则不处理
        }
        else{
            phase = Res; // 清除相位差
            printf("相位设置: %d\n", phase);
        }
		  		 
  } 
} 

void u4_printf(char* fmt,...)  {  
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)UART4_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)UART4_TX_BUF);//此次发送数据的长度
	for(j=0;j<i;j++)//循环发送数据
	{
	  while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET);  //等待上次传输完成 
		USART_SendData(UART4,(uint8_t)UART4_TX_BUF[j]); 	 //发送数据到串口3 
	}
	
}
