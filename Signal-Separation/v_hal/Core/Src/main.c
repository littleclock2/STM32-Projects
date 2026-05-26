/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 主程序体
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LED.h" // 包含LED控制头文件
#include "KEY.h" // 包含按键控制头文件
#include <main.h>
#include "SI5351_Soft.h"
// 如有需要，可在此处添加额外的包含文件
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define BUFFER_SIZE 256  // 缓冲区大小
uint16_t ADC_Value[BUFFER_SIZE] = {0};  // ADC数据存储数组
volatile bool ADC_done = false;
TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch1;
// 如有需要，可在此处添加自定义类型定义
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 如有需要，可在此处添加私有宏定义
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// 如有需要，可在此处添加私有宏
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 如有需要，可在此处添加私有变量
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
 void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
     {
    if(htim == &htim2)
    {
       HAL_DMA_Start_IT(&hdma_tim2_ch1, (uint32_t)&GPIOD->IDR, (uint32_t)&ADC_Value, 1);
        
        // 如果需要连续搬运，可在此重新使能DMA
        __HAL_DMA_ENABLE(&hdma_tim2_ch1);
    }
    HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
}
     // DMA传输完成回调函数
void HAL_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma) {
    if(hdma == &hdma_tim2_ch1) {
        ADC_done = true;  // 仅标记完成，不在此重启DMA
        // 可在此添加数据处理代码
    }
    DMA_Start();
}
// 如有需要，可在此处添加私有函数原型
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rec_tra_data[1]; // 定义一个数组，用于存放接收和发送的数据
// 用户代码可以在此处添加
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  
  
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  //HAL_UART_Transmit(&huart1, "Hello World", sizeof("Hello World"), 0XFFFF); // 通过USART1发送"Hello World"
  //HAL_UART_Receive_IT(&huart1, (uint8_t *)&rec_tra_data, 1); // 使能USART1接收中断，接收1字节数据
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  //ADS805E_Init();//初始化dma与tim中断
  Si5351Init();//初始化方波发生器
  HAL_Delay(10);
  SetFrequency(1000000,1);
  SetFrequency(1000000,0);
  SetFrequency(1000000,2);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
     if (ADC_done) {
            // 数据处理逻辑
            ADC_done = false;
            DMA_Start();
     }
//    if (rec_tra_data[0] == 0x01) { HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); } // 如果接收到0x01，关闭LED
//    if (rec_tra_data[0] == 0x02) { HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); } // 如果接收到0x02，打开LED
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* 串口中断回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart1) // 判断是否为USART1的中断
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)rec_tra_data, 1, 0xffff); // 将接收到的数据发送回去
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&rec_tra_data, 1); // 再次开启串口接收中断，准备接收下一个字节
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  // 用户可以添加自己的实现来报告HAL错误返回状态
  __disable_irq(); // 禁用中断
  while (1) // 无限循环
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  // 用户可以添加自己的实现来报告文件名和行号
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
