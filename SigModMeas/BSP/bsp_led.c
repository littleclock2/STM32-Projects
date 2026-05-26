/***********************************************************************************************************************************
 ** 【文件名称】  led.c
 ** 【编写人员】  荣洋电子-李工
 ** 【淘    宝】  荣洋电子      https://shop572511740.taobao.com/?spm=pc_detail.29232929/evo365560b447259.shop_block.dshopinfo.296e7dd6rGcJmq
 ***********************************************************************************************************************************
 ** 【文件功能】  实现LED指示灯常用的初始化函数、功能函数
 **
 ** 【移植说明】
 **
 ** 【更新记录】
 **
***********************************************************************************************************************************/
#include "bsp_led.h"



void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;    // 定义一个GPIO_InitTypeDef类型的结构体

    // 使能所用引脚端口时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 配置引脚工作模式
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;     // 选择要控制的GPIO引脚
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;   // 引脚模式：输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   // 输出类型：推挽输出
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;    // 上下拉：上拉模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 引脚速率：2MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);    // 调用库函数，使用上面的配置初始化GPIO

    GPIO_ResetBits(GPIOA, GPIO_Pin_15);     // 点亮LED，低电平点亮    
}

void LED_On(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_15);     // 点亮LED，低电平点亮  	
}
	
void LED_Off(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_15);     // 点亮LED，低电平点亮  	
}
