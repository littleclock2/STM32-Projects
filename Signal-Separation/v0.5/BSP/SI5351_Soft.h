#ifndef _si5351a_h
#define _si5351a_h
#include "stm32f4xx.h"
//Si5351寄存器声明
#define SI_CLK0_CONTROL	16			// Register definitions
#define SI_CLK1_CONTROL	17
#define SI_CLK2_CONTROL	18
#define SI_SYNTH_PLL_A	26
#define SI_SYNTH_PLL_B	34
#define SI_SYNTH_MS_0	42
#define SI_SYNTH_MS_1	50
#define SI_SYNTH_MS_2	58
#define SI_PLL_RESET		177

#define SI_R_DIV_1		0x00			// R-division ratio definitions
#define SI_R_DIV_2     0x10  // 0b00010000
#define SI_R_DIV_4     0x20  // 0b00100000
#define SI_R_DIV_8     0x30  // 0b00110000
#define SI_R_DIV_16    0x40  // 0b01000000
#define SI_R_DIV_32    0x50  // 0b01010000
#define SI_R_DIV_64    0x60  // 0b01100000
#define SI_R_DIV_128   0x70  // 0b01110000

#define SI_CLK_SRC_PLL_A	0x00
#define SI_CLK_SRC_PLL_B	0b00100000
#define XTAL_FREQ	25000000			// Crystal frequency
//IIC总线引脚配置
#define SDA(n) {n?GPIO_SetBits(GPIOB,GPIO_Pin_7):GPIO_ResetBits(GPIOB,GPIO_Pin_7);}
#define CLK(n) {n?GPIO_SetBits(GPIOB,GPIO_Pin_6):GPIO_ResetBits(GPIOB,GPIO_Pin_6);}
//相关函数声明
void Si5351Init(void);//初始化Si5351的GPIO
void SetPLLClk(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom);//设置PPL时钟
void SetFrequency(double frequency,uint16_t channel);//时钟Si5351时钟频率
void SetMultisynth(uint8_t synth, uint32_t a, uint32_t b, uint32_t c, uint8_t rDiv);
void IICstart(void);
void IICstop(void);
void IICsend(u8 DATA);
void IICsendreg(uint8_t reg, uint8_t data);


#endif
