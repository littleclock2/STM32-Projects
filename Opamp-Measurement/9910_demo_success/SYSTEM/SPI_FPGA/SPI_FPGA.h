/*命令定义-结尾*******************************/
/*SPI接口定义-开头****************************/
#ifndef __SPI_FPGA_H
#define __SPI_FPGA_H


#include "stm32f10x.h"

#include <stdio.h>

#define      FPGA_SPIx                        SPI2
#define      FPGA_SPI_APBxClock_FUN          RCC_APB1PeriphClockCmd
#define      FPGA_SPI_CLK                     RCC_APB1Periph_SPI2

//CS(NSS)引脚 片选选普通GPIO即可
#define      FPGA_SPI_CS_APBxClock_FUN       RCC_APB2PeriphClockCmd
#define      FPGA_SPI_CS_CLK                  RCC_APB2Periph_GPIOC    
#define      FPGA_SPI_CS_PORT                 GPIOB
#define      FPGA_SPI_CS_PIN                  GPIO_Pin_12

//SCK引脚
#define      FPGA_SPI_SCK_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define      FPGA_SPI_SCK_CLK                 RCC_APB2Periph_GPIOB   
#define      FPGA_SPI_SCK_PORT                GPIOB   
#define      FPGA_SPI_SCK_PIN                 GPIO_Pin_13
//MISO引脚
#define      FPGA_SPI_MISO_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      FPGA_SPI_MISO_CLK                RCC_APB2Periph_GPIOB    
#define      FPGA_SPI_MISO_PORT               GPIOB 
#define      FPGA_SPI_MISO_PIN                GPIO_Pin_14
//MOSI引脚
#define      FPGA_SPI_MOSI_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      FPGA_SPI_MOSI_CLK                RCC_APB2Periph_GPIOB    
#define      FPGA_SPI_MOSI_PORT               GPIOB 
#define      FPGA_SPI_MOSI_PIN                GPIO_Pin_15

#define  		SPI_FPGA_CS_LOW()     						GPIO_ResetBits( FPGA_SPI_CS_PORT, FPGA_SPI_CS_PIN )
#define  		SPI_FPGA_CS_HIGH()    						GPIO_SetBits( FPGA_SPI_CS_PORT, FPGA_SPI_CS_PIN )

#define Dummy_Byte 0xFF
/*SPI接口定义-结尾****************************/

/*等待超时时间*/
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))

/*信息输出*/
#define FPGA_DEBUG_ON         1

#define FPGA_INFO(fmt,arg...)           printf("<<-FPGA-INFO->> "fmt"\n",##arg)
#define FPGA_ERROR(fmt,arg...)          printf("<<-FPGA-ERROR->> "fmt"\n",##arg)
#define FPGA_DEBUG(fmt,arg...)          do{\                                        if(FPGA_DEBUG_ON)\                                         printf("<<-FPGA-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\                                     }while(0)
static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT; 
void SPI_FPGA_Init(void);
u8 SPI_FPGA_ReadByte(void);
u8 SPI_FPGA_SendByte(u8 byte);
void Delay(__IO uint32_t nCount);
static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode);																				
#endif /* __SPI_FPGA_H */
