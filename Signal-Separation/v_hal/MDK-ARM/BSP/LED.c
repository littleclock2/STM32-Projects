#include "LED.h" // 包含LED相关的头文件，其中定义了与LED相关的宏和函数原型

void LED_ON(void) // 定义一个函数，用于打开LED
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); // 写GPIO引脚状态为低电平（GPIO_PIN_RESET），通常用于打开LED
}

void LED_OFF(void) // 定义一个函数，用于关闭LED
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // 写GPIO引脚状态为高电平（GPIO_PIN_SET），通常用于关闭LED
}
void LED_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);	
}
