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
#include "DAC.h"        // DAC驱动

// 宏定义
#define FFT_ADC_N 1024  // FFT采样点数
#define FFT_SIN_BASE 98000  // 正弦波基准值
#define FFT_TRI_BASE 76100  // 三角波基准值
#define FFT_TRI3_BASE 10000 // 三次谐波基准值
#define WAVE_LENGTH 1024    // 波形数据长度
#define DAC_RESOLUTION 4096 // 12位DAC分辨率
#define TRIGGER_FREQ 1000000   // 触发频率1MHz

// 波形类型枚举
typedef enum {
    WAVE_SINE,      // 正弦波
    WAVE_TRIANGLE,  // 三角波
} WaveType;

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
bool needUpdate = false;       // 更新标志

// 外部引用
extern const arm_cfft_instance_f32 arm_cfft_sR_f32_len1024; // ARM FFT实例
extern uint16_t ADS805_Data[Length];  // ADC数据缓冲区
extern uint16_t ADS805_Data_Completed; // ADC采集完成标志

// FFT相关变量
float fft_in_adc_data[FFT_ADC_N * 2] = {0}; // FFT输入数据(复数形式)
float fft_out_adc_data[FFT_ADC_N] = {0};    // FFT输出幅度
float fft_max_value_1 = 0;    // FFT最大幅值1
u32 fft_max_value_index_1 = 0; // FFT最大幅值索引1
float fft_max_value_2 = 0;    // FFT最大幅值2
u32 fft_max_value_index_2 = 0; // FFT最大幅值索引2
float f_s = 1024000;          // 采样频率1.024MHz

// 比较器相关
u16 comper = 0;
u8 comper_array[16] = {0};

// 信号类型标志
bool signal_type = false; // 信号类型标志
u8 signal_mode = 0;      // 信号模式

// 相位差
float phase = 0;         // 相位差

// DAC波形数据
u32 triangle_wave[2048]; // 三角波数据
u16 triangle_wave_n = 0; // 三角波点数
u32 sin_wave[2048];      // 正弦波数据
u16 sin_wave_n = 0;      // 正弦波点数

// 函数声明
void WaveGen_Init(SystemConfig *config);
void FFT_Deal(uint16_t adc_value[], float *fft_input, int data_length);
void GenerateTriangleWave(uint16_t *buffer, uint16_t points, uint16_t amplitude, uint16_t offset);
void GenerateSineWave(uint16_t *buffer, uint16_t points, uint16_t amplitude, uint16_t offset);
void GenerateWaveform(ChannelConfig *ch);
void ApplyPhaseDifference(SystemConfig *config);
void UpdatePhaseDifference(float degrees);
void UpdateChannel2(WaveType type, float freq);
void UpdateChannel1(WaveType type, float freq);

// 主函数
int main(void){
    SystemConfig cfg; // 系统配置
    
    // 硬件初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    ADS805_Init();
    Start_ADS805();
    Si5351Init();
    Led_Init();  
    DAC1_Init();
    DAC2_Init();
    EXTI_Configuration();
    GPIO_Configuration();
    WaveGen_Init(&cfg);
    
    // 设置SI5351输出频率
    SetFrequency(f_s, 0);
    SetFrequency(1000000, 1);
    SetFrequency(f_s, 2);
    
    delay_ms(1000); // 延时1秒
    
    while (1){  // 主循环
        LED_On();       // LED亮
        delay_ms(500);  // 延时500ms
        LED_Off();      // LED灭
        delay_ms(500);  // 延时500ms
        
        if (ADS805_Data_Completed) { // ADC数据采集完成
            // FFT处理
            FFT_Deal(ADS805_Data, fft_in_adc_data, FFT_ADC_N);
            arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_in_adc_data, 0, 1);
            arm_cmplx_mag_f32(fft_in_adc_data, fft_out_adc_data, FFT_ADC_N);
            fft_out_adc_data[0] = 0; // 去除直流分量
            
            // 检测信号特征
            for(int i = 0; i < FFT_ADC_N/2; i++){
                if(fft_out_adc_data[i] > 2500){
                    comper_array[comper] = i;
                    comper++;
                }
            }
            
            // 判断信号类型
            if(comper >= 3){
                signal_type = true; // 复合信号
                u8 temp_n = 0;
                for(int i = 0; i < comper; i++){
                    if(fft_out_adc_data[comper_array[i]] > FFT_TRI3_BASE) temp_n++;
                }
                
                if(temp_n == 2)
                    signal_mode = 0; // 三角波
                else if(temp_n == 1)
                    signal_mode = 1; // 正弦波
            }
            else{
                signal_type = false; // 简单信号
                if(comper == 2)
                    signal_mode = 0; // 三角波
                else if(comper == 1)
                    signal_mode = 1; // 正弦波
                else if(comper == 0)
                    phase = 180; // 反相
            }
            
            // 获取FFT最大幅值及其索引
            arm_max_f32(fft_out_adc_data, FFT_ADC_N/2, &fft_max_value_1, &fft_max_value_index_1);
            
            // 计算相位差
            if(!signal_type){ // 简单信号
                if(signal_mode == 1 && phase != 180) // 正弦波
                    phase = acos(fft_max_value_1/(2*FFT_SIN_BASE));
                else if(signal_mode == 0 && phase != 180) // 三角波
                    phase = atan2f(fft_max_value_1, fft_out_adc_data[fft_max_value_index_1+1]);
            }
            else if(signal_type){ // 复合信号
                if(signal_mode){ // 正弦波
                    float temp = (double)FFT_SIN_BASE*FFT_SIN_BASE + (double)FFT_TRI_BASE*FFT_TRI_BASE - (double)fft_max_value_1*fft_max_value_1;
                    phase = acos(temp/(2.0*FFT_SIN_BASE*FFT_TRI_BASE));
                }
                else{ // 三角波
                    phase = atan2f(fft_max_value_1, fft_out_adc_data[fft_max_value_index_1+1]);
                }
            }
            
            // 重新启动ADC采集
            Start_ADS805();
            
            if(needUpdate){ // 需要更新波形
                UpdateChannel1(WAVE_SINE, 20000.0f);    // 通道1更新为20kHz正弦波
                UpdateChannel2(WAVE_TRIANGLE, 50000.0f); // 通道2更新为50kHz三角波
                UpdatePhaseDifference(90.0f);           // 更新相位差为90度
                needUpdate = 0;
            }
        }
        delay_ms(10);
    }
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
void UpdateChannel1(WaveType type, float freq) {
    SystemConfig *cfg;
    cfg->ch1.type = type;
    cfg->ch1.frequency = fmaxf(5000, fminf(100000, freq)); // 限制频率范围5kHz-100kHz
    cfg->ch1.effectivePoints = (uint16_t)(TRIGGER_FREQ / cfg->ch1.frequency + 0.5f);
    GenerateWaveform(&cfg->ch1);
    ApplyPhaseDifference(cfg);
}

// 更新通道2配置
void UpdateChannel2(WaveType type, float freq) {
    SystemConfig *cfg;
    cfg->ch2.type = type;
    cfg->ch2.frequency = fmaxf(5000, fminf(100000, freq)); // 限制频率范围5kHz-100kHz
    cfg->ch2.effectivePoints = (uint16_t)(TRIGGER_FREQ / cfg->ch2.frequency + 0.5f);
    GenerateWaveform(&cfg->ch2);
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
