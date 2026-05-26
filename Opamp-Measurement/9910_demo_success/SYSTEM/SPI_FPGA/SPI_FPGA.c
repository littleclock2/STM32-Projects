/**
  * @brief  SPI_FPGA初始化
  * @param  无
  * @retval 无
  */
#include "SPI_FPGA.h"

void SPI_FPGA_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 使能SPI时钟 */
	FPGA_SPI_APBxClock_FUN ( FPGA_SPI_CLK, ENABLE );
	
	/* 使能SPI引脚相关的时钟 */
 	FPGA_SPI_CS_APBxClock_FUN ( FPGA_SPI_CS_CLK|FPGA_SPI_SCK_CLK|
																	FPGA_SPI_MISO_PIN|FPGA_SPI_MOSI_PIN, ENABLE );
	
  /* 配置SPI的 CS引脚，普通IO即可 */
  GPIO_InitStructure.GPIO_Pin = FPGA_SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(FPGA_SPI_CS_PORT, &GPIO_InitStructure);
	
  /* 配置SPI的 SCK引脚*/
  GPIO_InitStructure.GPIO_Pin = FPGA_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(FPGA_SPI_SCK_PORT, &GPIO_InitStructure);

  /* 配置SPI的 MISO引脚*/
  GPIO_InitStructure.GPIO_Pin = FPGA_SPI_MISO_PIN;
  GPIO_Init(FPGA_SPI_MISO_PORT, &GPIO_InitStructure);

  /* 配置SPI的 MOSI引脚*/
  GPIO_InitStructure.GPIO_Pin = FPGA_SPI_MOSI_PIN;
  GPIO_Init(FPGA_SPI_MOSI_PORT, &GPIO_InitStructure);

  /* 停止信号 FPGA: CS引脚高电平*/
  SPI_FPGA_CS_HIGH();

  /* SPI 模式配置 */
  // FPGA芯片 支持SPI模式0及模式3，据此设置CPOL CPHA
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//时钟极性设置为高电平
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;//偶数边沿采样，即本代码模式下为上升沿采样
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(FPGA_SPIx , &SPI_InitStructure);

  /* 使能 SPI  */
  SPI_Cmd(FPGA_SPIx , ENABLE);
	
}

/**
  * @brief  使用SPI发送一个字节的数据
  * @param  byte：要发送的数据
  * @retval 返回接收到的数据
  */
u8 SPI_FPGA_SendByte(u8 byte)
{
	 SPITimeout = SPIT_FLAG_TIMEOUT;
  /* 等待发送缓冲区为空，TXE事件 */
  while (SPI_I2S_GetFlagStatus(FPGA_SPIx , SPI_I2S_FLAG_TXE) == RESET)
	{
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
   }

  /* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
  SPI_I2S_SendData(FPGA_SPIx , byte);

	SPITimeout = SPIT_FLAG_TIMEOUT;
  /* 等待接收缓冲区非空，RXNE事件 */
  while (SPI_I2S_GetFlagStatus(FPGA_SPIx , SPI_I2S_FLAG_RXNE) == RESET)
  {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
   }

  /* 读取数据寄存器，获取接收缓冲区数据 */
  return SPI_I2S_ReceiveData(FPGA_SPIx );
}
 /**
  * @brief  使用SPI读取一个字节的数据
  * @param  无
  * @retval 返回接收到的数据
  */
u8 SPI_FPGA_ReadByte(void)
{
  return (SPI_FPGA_SendByte(Dummy_Byte));
}
/**
  * @brief  等待超时回调函数
  * @param  None.
  * @retval None.
  */
static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
{
  /* 等待超时后的处理,输出错误信息 */
  FPGA_ERROR("SPI 等待超时!errorCode = %d",errorCode);
  return 0;
}
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}
