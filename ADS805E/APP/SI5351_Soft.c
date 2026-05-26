
#include "stm32f10x.h"
#include "SI5351_Soft.h"
#include "myfunc.h"			

void IICstart()//IIC总线起始信号
{
		
	  SDA(1);
	  CLK(1);
	  SDA(0);
    delay_us(100); 
	  CLK(0);
    delay_us(100); 	
}
void IICstop()//IIC总线停止信号
{
	  CLK(0);
    delay_us(100); 
	  SDA(0);
    delay_us(100); 
	  CLK(1);
    delay_us(100); 
	  SDA(1);	
    delay_us(100); 
}
void IICsend(u8 DATA)//IIC总线发送信数据
{
		u16 i;
    for(i=0;i<8;i++) //发送一个字节数据 
    {     
        CLK(0); 		 //拉低时钟线，准备开始给SDA赋值
			
        if((DATA&0x80)==0)  
        {SDA(0);}  
        else  
        {SDA(1);}  
				
        DATA<<=1;  
        delay_us(100);    
        CLK(1);
        delay_us(100);      //等待从设备把数据接收完
		}  
    CLK(0);  
    SDA(1);	 				   //释放数据线   
    delay_us(100);   
    CLK(1); 
    delay_us(100); 
		while(i<1000){i++;}//等待从设备应答ACK 
		CLK(0); 					 //释放时钟线，为下次操作做准备
}
void I2C_Sendreg(uint8_t reg, uint8_t data)
{
  IICstart();      //起始信号
  delay_us(200);  
  IICsend(0xC0);   //发送设备地址+写信号
  delay_us(200);  
  IICsend(reg);    //内部寄存器地址
  delay_us(200);  
  IICsend(data);   //内部寄存器数据
  delay_us(200);  
  IICstop(); 
}
void Si5351Init()//初始化Si5351芯片
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		GPIO_InitTypeDef GPIO_Initure = {
			.GPIO_Mode = GPIO_Mode_Out_PP,
			.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7,
			.GPIO_Speed = GPIO_Speed_10MHz,
		};
	  GPIO_Init(GPIOB,&GPIO_Initure);//初始化GPIOB
}
//void SI5351_Init(void)
//{
//    I2C_GPIO_Init();
//	Delay_ms(10);
//}

void SetFrequency(double frequency,uint16_t channel) {
    uint32_t xtalFreq = XTAL_FREQ;
    uint32_t pllFreq;
    uint8_t mult, rDiv = SI_R_DIV_1;
    uint32_t a, b = 0, c = 1;

    // 动态选择R分频器
    if (frequency < 1000000) {
        if (frequency <= 8000) rDiv = SI_R_DIV_128;
        else if (frequency <= 16000) rDiv = SI_R_DIV_64;
        else if (frequency <= 32000) rDiv = SI_R_DIV_32;
        else if (frequency <= 64000) rDiv = SI_R_DIV_16;
        else if (frequency <= 128000) rDiv = SI_R_DIV_8;
        else if (frequency <= 256000) rDiv = SI_R_DIV_4;
        else rDiv = SI_R_DIV_2;
    }

    // 动态选择PLL频率（示例：800MHz）
    pllFreq = 800000000;

    // 计算分频系数
    float divider = (float)pllFreq / (frequency * (1 << (rDiv >> 4)));
    a = (uint32_t)divider;
    float fraction = divider - a;
    b = (uint32_t)(fraction * 0xFFFFF); // 20-bit分母
    c = 0xFFFFF;

    // 配置PLL
    mult = pllFreq / xtalFreq;
    uint32_t remainder = pllFreq % xtalFreq;
    uint32_t num = (remainder * c) / xtalFreq;

    // 应用配置
		if(channel==0)
		{   
			SetPLLClk(SI_SYNTH_PLL_A, mult, num, c);
			SetMultisynth(SI_SYNTH_MS_0, a, b, c, rDiv);
			I2C_Sendreg(SI_PLL_RESET, 0xA0);
			I2C_Sendreg(SI_CLK0_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
		}
		else if(channel==1)
		{
			SetPLLClk(SI_SYNTH_PLL_A, mult, num, c);
			SetMultisynth(SI_SYNTH_MS_1, a, b, c, rDiv);
			I2C_Sendreg(SI_PLL_RESET, 0xA0);
			I2C_Sendreg(SI_CLK1_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
		}	
		else if(channel==2)
		{
			SetPLLClk(SI_SYNTH_PLL_A, mult, num, c);
			SetMultisynth(SI_SYNTH_MS_2, a, b, c, rDiv);
			I2C_Sendreg(SI_PLL_RESET, 0xA0);
			I2C_Sendreg(SI_CLK2_CONTROL, 0x4F | SI_CLK_SRC_PLL_A);
		}			

		
}


void SetPLLClk(uint8_t pll, uint8_t mult, uint32_t num, uint32_t denom)//设置PPL时钟
{
  uint32_t P1;					// PLL config register P1
  uint32_t P2;					// PLL config register P2
  uint32_t P3;					// PLL config register P3

  P1 = (uint32_t)(128 * ((float)num / (float)denom));
  P1 = (uint32_t)(128 * (uint32_t)(mult) + P1 - 512);
  P2 = (uint32_t)(128 * ((float)num / (float)denom));
  P2 = (uint32_t)(128 * num - denom * P2);
  P3 = denom;

  I2C_Sendreg(pll + 0, (P3 & 0x0000FF00) >> 8);
  I2C_Sendreg(pll + 1, (P3 & 0x000000FF));
  I2C_Sendreg(pll + 2, (P1 & 0x00030000) >> 16);
  I2C_Sendreg(pll + 3, (P1 & 0x0000FF00) >> 8);
  I2C_Sendreg(pll + 4, (P1 & 0x000000FF));
  I2C_Sendreg(pll + 5, ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16));
  I2C_Sendreg(pll + 6, (P2 & 0x0000FF00) >> 8);
  I2C_Sendreg(pll + 7, (P2 & 0x000000FF));
}

void SetMultisynth(uint8_t synth, uint32_t a, uint32_t b, uint32_t c, uint8_t rDiv) 
{
    uint32_t P1, P2, P3;

    // 计算P1, P2, P3寄存器值（参考SI5351数据手册）
    P1 = 128 * a + (uint32_t)(128 * (float)b / c) - 512;
    P2 = 128 * b - c * (uint32_t)(128 * (float)b / c);
    P3 = c;

    // 写入寄存器
    I2C_Sendreg(synth + 0, (P3 >> 8) & 0xFF);
    I2C_Sendreg(synth + 1, P3 & 0xFF);
    I2C_Sendreg(synth + 2, ((P1 >> 16) & 0x03) | (rDiv & 0xF0));
    I2C_Sendreg(synth + 3, (P1 >> 8) & 0xFF);
    I2C_Sendreg(synth + 4, P1 & 0xFF);
    I2C_Sendreg(synth + 5, ((P3 >> 12) & 0xF0) | ((P2 >> 16) & 0x0F));
    I2C_Sendreg(synth + 6, (P2 >> 8) & 0xFF);
    I2C_Sendreg(synth + 7, P2 & 0xFF);
}
