#ifndef __LED_H // 检查_LED_H宏是否未定义，以防止头文件被重复包含
#define __LED_H // 定义_LED_H宏

#include "main.h" // 包含主头文件，其中包含了MCU系列和HAL库的配置

void LED_ON(void); // 声明LED_ON函数，该函数用于打开LED
void LED_OFF(void); // 声明LED_OFF函数，该函数用于关闭LED
void LED_Toggle(void);


#endif // _LED_H
