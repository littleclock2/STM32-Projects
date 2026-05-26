#ifndef __AD9910_H__
#define __AD9910_H__

#include "stm32f10x.h"




#define uchar unsigned char
#define uint  unsigned int	
#define ulong  unsigned long int

#define AD9910_PWR		PCout(13)
#define AD9910_SDIO		PFout(12)  //MOSI
#define DRHOLD				PCout(1)
#define DROVER				PCout(2)
#define UP_DAT				PCout(3)
//#define PROFILE1			PCout(9)
#define PROFILE1			PFout(9)
#define MAS_REST			PAout(6)
#define SCLK					PFout(11)  //SCK
#define DRCTL					PAout(4)
//#define OSK						PCout(8)
#define OSK						PCout(0)
#define PROFILE0			PCout(4)
#define PROFILE2			PCout(5)
//#define CS						PBout(10) // CS
#define CS						PBout(2) // CS


////#define AD9910_CSN_Set CS = 1
////#define AD9910_CSN_Clr CS = 0

////#define AD9910_IUP_Set UP_DAT = 1     
////#define AD9910_IUP_Clr UP_DAT = 0

typedef enum {
	TRIG_WAVE = 0,
	SQUARE_WAVE,
	SINC_WAVE,
} AD9910_WAVE_ENUM;

void AD9110_IOInit(void);
void Init_AD9910(void);
void AD9910_FreWrite(ulong Freq);										//???
void AD9910_AmpWrite(uint16_t Amp);
void AD9910_RAM_WAVE_Set(AD9910_WAVE_ENUM wave);
void AD9910_DRG_AMP_Init(void);
void AD9910_DRG_FreInit_AutoSet(FunctionalState autoSweepEn);
void AD9910_DRG_FrePara_Set(u32 lowFre, u32 upFre, u32 posStep, u32 negStep, u16 posRate, u16 negRate);



#endif


