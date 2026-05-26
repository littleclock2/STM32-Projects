# STM32-Signal-Separation

基于 STM32F407 + ADS805 ADC + ARM DSP 的信号分离装置，支持 FFT 频谱分析与 DAC 信号输出。

## 硬件需求

- 主控芯片：STM32F407VET6
- ADC：ADS805（12-bit, 并行数据接口）
- DAC：内置 12-bit DAC（PA4/PA5）
- 时钟发生器：SI5351（I2C，PB6/PB7）
- 显示：迪文串口屏（HMI）
- 数学库：ARM CMSIS-DSP（Cortex-M4F）

## 软件环境

- IDE：Keil MDK-ARM
- 标准外设库：STM32F4xx Standard Peripheral Library
- DSP 库：ARM CMSIS-DSP

## 功能特性

- ADS805 高速 ADC 采样（PD0-15 并行）
- 4096 点 FFT 频谱分析
- SI5351 可编程时钟同步
- DAC 信号输出（正弦波、三角波、方波）
- 串口屏波形显示
- 多版本迭代（v0.5 / v1.0 / v1.1 / Hal 版本）

## 目录结构

```
STM32-Signal-Separation/
├── v0.5/          # 早期版本
├── v1.0/          # 基础版本
├── v1.1/          # 改进版本
├── v_hal/         # HAL 库版本
├── LICENSE
└── README.md
```

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
