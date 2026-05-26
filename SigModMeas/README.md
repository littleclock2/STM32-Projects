# STM32-SigModMeas

基于 STM32F407 + ADS805 ADC + AD9833 DDS + ARM DSP 的信号调制方式识别与参数估计系统。

## 硬件需求

- 主控芯片：STM32F407VET6
- ADC：ADS805（12-bit, 并行数据接口）
- DDS 信号发生器：AD9833
- 时钟发生器：SI5351（I2C，PB6/PB7）
- 显示：迪文串口屏（HMI）
- 继电器：GPIO 控制信号路径切换

## 软件环境

- IDE：Keil MDK-ARM
- 标准外设库：STM32F4xx Standard Peripheral Library
- DSP 库：ARM CMSIS-DSP（Cortex-M4F）

## 功能特性

- ADS805 高速 ADC 采样（PD0-15 并行）
- 4096 点 FFT 频谱分析
- AD9833 激励信号输出
- SI5351 可编程时钟同步
- 继电器信号路径切换
- 调制方式识别（AM/FM/PM/ASK/FSK/PSK）
- 串口屏波形与参数显示

## 目录结构

```
STM32-SigModMeas/
├── BSP/              # 外设驱动
├── DSP_LIB/          # ARM CMSIS-DSP 库
├── Libraries/        # STM32F4xx 标准外设库
├── User/             # 主程序
├── STM32F407.uvprojx # Keil 工程文件
├── LICENSE
└── README.md
```

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
