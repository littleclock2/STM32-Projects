#include "KEY.h" // 包含按键相关的头文件，其中定义了与按键相关的宏和函数原型

int Key1_Num = 0; // 定义一个全局变量Key1_Num，用于记录按键被按下的次数

void Get_KeyNum(void) // 定义一个函数，用于获取按键编号
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0) // 读取与按键1相关联的GPIO引脚状态
    {
        HAL_Delay(20); // 如果按键被按下（低电平），则延时20ms，用于消除按键抖动
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == 0); // 等待直到按键被释放
        HAL_Delay(20); // 再次延时20ms，用于消除按键抖动
        
        Key1_Num++; // 按键被按下并释放后，增加按键计数
    }
}
