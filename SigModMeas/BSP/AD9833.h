#include "stm32f4xx.h"
#include "stdbool.h"
// AD9833 控制引脚定义 (GPIOE)
#define AD9833_PORT       GPIOE
#define AD9833_CS1_PIN    GPIO_Pin_0   // PE0
#define AD9833_CS2_PIN    GPIO_Pin_1   // PE1
#define AD9833_SCLK_PIN   GPIO_Pin_2   // PE2
#define AD9833_PICO_PIN   GPIO_Pin_3   // PE3
/*电平变化宏*/
#define WAIT __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();//__NOP();__NOP();//可以设置延迟时间

// 使用标准库的GPIO操作
#define CS1_P  GPIO_SetBits(AD9833_PORT, AD9833_CS1_PIN)
#define CS1_N  GPIO_ResetBits(AD9833_PORT, AD9833_CS1_PIN)
#define CS2_P  GPIO_SetBits(AD9833_PORT, AD9833_CS2_PIN)
#define CS2_N  GPIO_ResetBits(AD9833_PORT, AD9833_CS2_PIN)
#define SCLK_P GPIO_SetBits(AD9833_PORT, AD9833_SCLK_PIN)
#define SCLK_N GPIO_ResetBits(AD9833_PORT, AD9833_SCLK_PIN)
#define PICO_P GPIO_SetBits(AD9833_PORT, AD9833_PICO_PIN)
#define PICO_N GPIO_ResetBits(AD9833_PORT, AD9833_PICO_PIN)

/* Command definitions in hexadecimal */
#define COMMAND_B28     0x2000  // 0b0010000000000000
#define COMMAND_HLB     0x1000  // 0b0001000000000000
#define COMMAND_FSEL    0x0800  // 0b0000100000000000
#define COMMAND_PSEL    0x0400  // 0b0000010000000000
#define COMMAND_RESET   0x0100  // 0b0000000100000000
#define COMMAND_SLEEP1  0x0080  // 0b0000000010000000
#define COMMAND_SLEEP2  0x0040  // 0b0000000001000000
#define COMMAND_OPBITEN 0x0020  // 0b0000000000100000
#define COMMAND_DIV2    0x0008  // 0b0000000000001000
#define COMMAND_MODE    0x0002  // 0b0000000000000010

/*寄存器地址*/
#define AD9833_SEL_CTR 0x0000
#define AD9833_SEL_FREQ0 0x4000
#define AD9833_SEL_FREQ1 0x8000
#define AD9833_SEL_PHASE0 0xC000
#define AD9833_SEL_PHASE1 0xE000


/*函数声明*/
void AD9833_SetRefClk(double freq); //设置参考时钟频率
void AD9833_Init(void);//初始化AD9833
void AD9833_Reset(bool channel1_en, bool channel2_en);//复位AD9833
void AD9833_WriteFreqReg(uint32_t data, bool channel1_en, bool channel2_en);//写入频率寄存器
void AD9833_AddPhaseReg(uint16_t data, bool channel1_en, bool channel2_en);//写入相位寄存器
void AD9833_SetFreq(double freq, bool channel1_en, bool channel2_en);//设置频率
void AD9833_AddPhase(double phase, bool channel1_en, bool channel2_en);//设置相位
void AD9833_SetWaveform(uint8_t waveform0,bool channel1_en, bool channel2_en);//设置波形
void AD9833_ALLInit(double freq_0,double phase_0, uint8_t wave_0,double freq_1,double phase_1, uint8_t wave_1);//初始化AD9833
void AD9833_Sleep(uint8_t sleep1_choice,uint8_t sleep2_chioce);//休眠

void AD9833_GPIO_Init(void);
