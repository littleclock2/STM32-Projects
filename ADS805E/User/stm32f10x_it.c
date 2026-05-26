/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

#define BUFFER_SIZE 1024
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stdbool.h"
#include "TIM_capture.h"
#include "TIM_ETR.h"
#include "myfunc.h"
#include "public.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */


  /*频率计相关中断函数 */
double UnknownSig_fre 				= 0;
float UnknownSig_Duty 				= 0;
double UnknownSig_fre_capture = 0;
double UnknownSig_fre_ETR     = 0;
typedef enum{ETR = 0,CAPTURE = 1} TIM_State;
static TIM_State ETR_state = ETR,Last_State = ETR;
  /*IsETR为1,频率高，采用ETR输入作频率计,IsETR为0，频率低(300kHZ以下),采用捕获定时器作等精度采样*/
  void TIM2_IRQHandler(void){
    static uint16_t cnt1 = 0;//计数开始后,完整的一个待测信号周期中,计数器更新次数
    static uint16_t cnt2 = 0;//计数开始后,在高电平时间里,计数器更新次数
    static bool flag = false,HighLevel_flag = false;//计数开始标志位与高电平标志位
    static uint16_t CCR0 = 0, CCR1 = 0;
    volatile uint16_t CCR2 = 0;
    
    /*
    CCR0: 计数开始时捕获寄存器的值
    CCR1: 下降沿时捕获寄存器的值
    CCR2: 上升沿时捕获寄存器的值
    */
    if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET){//更新中断
      if(flag){
        cnt1++;
        
      if(HighLevel_flag)
        cnt2++;
      }
      else{
        cnt1 = 0;
        cnt2 = 0;
      }
			TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
      
    }
    if(TIM_GetITStatus(TIM2,TIM_IT_CC3) == SET){//捕获中断,上升沿
      
      if(!flag){//开始计数
        CCR0 = TIM_GetCapture3(TIM2);
        flag = true;
        HighLevel_flag = true;
      }
      else{//测到一个周期的值，开始计算
        CCR2 = TIM_GetCapture3(TIM2);
        UnknownSig_fre_capture = 72000000.0 / (cnt1 * 65536 + CCR2 - CCR0);
        UnknownSig_Duty = (1.0 * (cnt2 * 65536 + CCR1 - CCR0)) / (cnt1 * 65536 + CCR2 - CCR0);
        flag = false;
        cnt1 = 0;
        cnt2 = 0;
      }
			TIM_ClearITPendingBit(TIM2,TIM_IT_CC3);
    }
    
    if(TIM_GetITStatus(TIM2,TIM_IT_CC4) == SET){//捕获中断,下降沿
      if(flag){//开始计数后才会更新这个值
        CCR1 = TIM_GetCapture4(TIM2);
        HighLevel_flag = false;
      }
			TIM_ClearITPendingBit(TIM2,TIM_IT_CC4);
    }
    
    
    return;
  }
  
  uint16_t cnt_update = 0;//ETR模式下更新次数，与TIM6中断函数共用
  
  /*PE0作为输入时钟*/
  void TIM4_IRQHandler(void){
    /* 捕获定时器 变量 */
    /* ETR 变量 */
    if(TIM_GetFlagStatus(TIM4,TIM_IT_Update) == SET){
      cnt_update++;
      TIM_ClearFlag(TIM4,TIM_IT_Update);
    }
    
      return;
  }
  void TIM6_IRQHandler(void){
    volatile uint16_t temp = TIM_GetCounter(TIM4);
    if(TIM_GetFlagStatus(TIM6,TIM_FLAG_Update) == SET){
      TIM_Cmd(TIM4,DISABLE);
      TIM_Cmd(TIM6,DISABLE);
      UnknownSig_fre_ETR = TIM_GetCounter(TIM4)+cnt_update*65536.0;
      cnt_update = 0;
      TIM_SetCounter(TIM4,0);
      TIM_SetCounter(TIM6,0);
			TIM6->CR1 |= TIM_CR1_CEN;;
			TIM4->CR1 |= TIM_CR1_CEN;
			
      Last_State = ETR_state;
			if(ETR_state == ETR){
				if(UnknownSig_fre_ETR < 80000){
					ETR_state = CAPTURE;
					UnknownSig_fre = UnknownSig_fre_capture;
					TIM_ITConfig(TIM2,TIM_IT_Update | TIM_IT_CC3 | TIM_IT_CC4,ENABLE);//频率小，使能Capture中断
				}
				else
					UnknownSig_fre = UnknownSig_fre_ETR * 1.000101 + 0.2718;
				
			}
			
			else{
				if(UnknownSig_fre_ETR > 100000){
					ETR_state = ETR;
					UnknownSig_fre = UnknownSig_fre_ETR * 1.0001 + 0.2718;
					TIM_ITConfig(TIM2,TIM_IT_Update | TIM_IT_CC3 | TIM_IT_CC4,DISABLE);//频率过大，失能Capture中断，防止卡死;
				}
				else{
					UnknownSig_fre = UnknownSig_fre_capture;
				}
			}
			
      
      TIM_ClearFlag(TIM6,TIM_FLAG_Update);
    }
  }

/*DMA中断指示位*/
bool ADC_done = false;
void DMA1_Channel5_IRQHandler(void){
  if(DMA_GetITStatus(DMA1_IT_TC5)){
    ADC_done = true;
    DMA_ClearITPendingBit(DMA1_IT_TC5);
  }
}


u8 uart_data[30];
u8 warning_data = 0;
void USART1_IRQHandler(void) {
  static u8 uart_cnt = 0,warning_cnt = 0,warning_temp = 0;
  u8 data = 0;
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		data = USART_ReceiveData(USART1);
    if(data != 0xff){
      warning_data = data;
      warning_cnt = 0;
    }
    else{
      if(++warning_cnt == 3){
        warning_data = warning_temp;
        warning_cnt = 0;
        uart_cnt = 0;
      }
    }

		if(data == '\n'){
			reaction();
			uart_data[uart_cnt] = '\0';
			uart_cnt = 0;
		}
		else if(!isprint(data) || data == '#')
			;
		else
			uart_data[uart_cnt++] = data;  
			USART_ClearITPendingBit(USART1,USART_IT_RXNE | USART_IT_TC);
  }
}

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
