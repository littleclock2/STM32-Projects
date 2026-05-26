#ifndef __KEY_H // 检查_KEY_H宏是否未定义，以防止头文件被重复包含
#define __KEY_H // 定义_KEY_H宏

#include "main.h" // 包含主头文件，其中包含了MCU系列和HAL库的配置

extern int Key1_Num; // 声明一个全局变量Key1_Num，该变量在其他文件中定义（可能是main.c）
void Get_KeyNum(void); // 声明Get_KeyNum函数，该函数用于获取按键编号

#endif // __KEY_H
