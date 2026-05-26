# STM32-Opamp-Measurement

基于 STM32F103 + ADS1118 ADC + AD9910 DDS 的集成运算放大器参数测量系统。

## 硬件需求

- 主控芯片：STM32F103
- ADC：ADS1118（16-bit 精密 ADC）
- DDS 信号发生器：AD9910
- 显示：0.96 寸 OLED（I2C）
- 运放电路：待测集成运放

## 软件环境

- IDE：Keil MDK-ARM
- 标准外设库：STM32F10x Standard Peripheral Library

## 功能特性

- AD9910 产生激励信号（正弦波）
- ADS1118 精密采样输入/输出电压
- 运放增益、带宽、失真度测量
- OLED 实时显示测量结果
- 串口数据传输

## 目录结构

```
STM32-Opamp-Measurement/
├── 9910_demo_success/    # 主项目代码
│   ├── CORE/             # 核心启动文件
│   ├── HARDWARE/         # 外设驱动（ADS1118、AD9910、OLED）
│   ├── STM32F10x_FWLib/  # 标准外设库
│   ├── SYSTEM/           # 系统函数（delay、usart、sys）
│   └── USER/             # 主程序
├── LICENSE
└── README.md
```

## 许可证

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.zh-hans)
