# STM32 Projects

STM32 系列 MCU 完整项目合集，包含 ADS805E 高速采集、信号分离、运放测量、信号调制识别等。

## 项目列表

| 项目 | 说明 |
|------|------|
| ADS805E | STM32F103 + ADS805E 高速 ADC 数据采集（DMA 传输） |
| Signal-Separation | STM32F407 + ADS805 信号分离装置（FFT 频谱分析） |
| Opamp-Measurement | STM32F103 + ADS1118 + AD9910 集成运放参数测量 |
| SigModMeas | STM32F407 + ADS805 + AD9833 信号调制方式识别 |

## 硬件需求

- 主控芯片：STM32F103ZET6 / STM32F407VET6
- ADC：ADS805（12-bit）、ADS1118（16-bit）
- DDS：AD9833、AD9910
- 时钟：SI5351
- 显示：迪文串口屏（HMI）

## 软件环境

- IDE：Keil MDK-ARM
- 标准外设库：STM32F10x / STM32F4xx Standard Peripheral Library
- DSP 库：ARM CMSIS-DSP

## 目录结构

```
STM32-Projects/
├── ADS805E/              # 高速 ADC 数据采集
├── Signal-Separation/    # 信号分离装置
├── Opamp-Measurement/    # 运放参数测量
├── SigModMeas/           # 信号调制识别
├── LICENSE
└── README.md
```

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)

 


---

Maintained by contributors.


