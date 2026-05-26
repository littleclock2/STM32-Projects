# STM32-ADS805E

基于 STM32F103 + ADS805E 高速 ADC 的数据采集系统，支持 DMA 传输与 SI5351 时钟同步。

## 硬件需求

- 主控芯片：STM32F103ZET6
- ADC：ADS805E（12-bit, 20MSPS）
- 时钟发生器：SI5351（I2C 控制，PB6/PB7）
- 显示：迪文串口屏（USART1）

## 软件环境

- IDE：Keil MDK-ARM
- 标准外设库：STM32F10x Standard Peripheral Library

## 功能特性

- ADS805E 高速 ADC 采集，PD0-15 并行数据读取
- TIM4 CH3（PB8）触发 DMA 传输，最高 7.2M 采样率
- SI5351 可编程时钟同步采样
- 串口屏显示波形（400 点）
- 支持单次/连续采样模式
- 垂直/水平缩放控制

## 目录结构

```
STM32-ADS805E/
├── APP/                  # 外设驱动（ADC、定时器、SI5351、USART）
├── Libraries/            # STM32F10x 标准外设库
├── StaruUp/              # 启动文件
├── User/                 # 主程序与中断处理
├── empty_project.uvprojx # Keil 工程文件
└── LICENSE
```

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
