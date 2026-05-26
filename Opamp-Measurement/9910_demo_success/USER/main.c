#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "ADS1118.h"
#include "oled.h"
#include "adc.h"
#include "stm32f10x.h"                  // Device header
#include "AD9910.h"
#include <math.h>
float buf_wave[40];
int uart5_res=0;
int uart1_flag = 0;
u16 vpp =0;
u16 idx =0;
//静态功耗接收数据
float Uo_1 = 0;
float Uo_2 = 0;


float Vavg = 0;
float Vavgs[10] = {0};
float Iavg = 0;//从Vavg计算得到的电流值

unsigned char buf[100];//串口屏输出暂存数组
// void log_sweep(unsigned int start_freq, unsigned int end_freq, unsigned int num_steps) {
//     double log_start, log_end, log_step, log_freq;
//     unsigned int freq;
// 		unsigned int i;
//     if (start_freq >= end_freq || num_steps == 0) {
//         return; // 错误处理
//     }

//     log_start = log10(start_freq);
//     log_end = log10(end_freq);
//     log_step = (log_end - log_start) / num_steps;

//     for (i = 0; i <= num_steps; i++) {
//         log_freq = log_start + i * log_step;
//         freq = (unsigned int)pow(10, log_freq);
//         AD9910_FreWrite(freq);
// //        AD9910_AmpWrite(16383); // 设置输出幅度最大
//         // 延时一段时间，以便观察频率变化
//         delay_ms(10);
// 				//printf("此时的扫频频率为:%d\r\n",freq);
//     }
// }
unsigned int log_sweep(unsigned int start_freq, unsigned int end_freq, unsigned int num_steps) {//对数扫频函数
    double log_start, log_end, log_step, log_freq;//对数扫频的起始频率，结束频率，步长，频率
    double temp_log_start, temp_log_end, temp_log_step, temp_log_freq;//对数扫频的起始频率，结束频率，步长，频率
    unsigned int freq;//频率
    unsigned int i;//循环变量
    float A_freq = 0;//频率对应幅值
	//float A_test=0;

    float temp_A_freq = 0;//频率对应幅值
    float temp_max_A_freq = 0;//频率对应最大幅值
    unsigned int temp_max_freq = 0;//频率对应最大幅值的频率

    float max_A_freq = 0;//频率对应最大幅值
    unsigned int max_freq = 0;//频率对应最大幅值的频率
    float target_A_freq_min = 0;//目标幅值最小值
    float target_A_freq_max = 0;//目标幅值最大值
    unsigned int closest_freq = 0;//最接近目标幅值的频率
    float closest_diff = 0;//最接近目标幅值的频率对应的幅值
    unsigned int freq_array[40]; // 假设最多有1000个满足条件的频率

    unsigned int temp_start_freq = 0;
    unsigned int temp_end_freq = 0;
   // int freq_count = 0;
		//int match_freq_array[20]={0};
		int k=0;
		//int freq_final=0;
		//short j=0;

    if (start_freq >= end_freq || num_steps == 0) {
        return 0; // 错误处理
    }

    log_start = log10(start_freq);//对数起始频率
    log_end = log10(end_freq);//对数结束频率
    log_step = (log_end - log_start) / num_steps;//对数步长

    for (i = 0; i <= num_steps; i++) {
        log_freq = log_start + i * log_step;//当前对数频率
        freq = (unsigned int)pow(10, log_freq);//对数频率转换为实际频率
        AD9910_FreWrite(freq);//设置频率
        AD9910_AmpWrite(11755); // 设置输出幅度
        // 延时一段时间，以便观察频率变化
        delay_ms(10);
        //printf("此时的扫频频率为:%d\r\n", freq);
		// 读取 ADC 的值，得到频率对应幅值
        ADS1118Init(ADS1118_SS_ONCE,ADS1118_MODE_DC,ADS1118_DR_128,ADS1118_PULL_UP_EN_E,ADS1118_NOP_W);//初始化ADS1118
        A_freq = 0.07+Getdata(ADS1118_MUX_0G,ADS1118_PGA_10,ADS1118_TS_MODE_ADC,CS_0);//这个地方记得改//得到当前频率对应的幅值

        //printf("此时的放大倍数为:%f", A_freq);
        // 记录最大值和对应的频率
        if (A_freq > max_A_freq) {
            max_A_freq = A_freq;
            max_freq = freq;
        }
        // if (freq >= 250000) {
		// 	printf("flag1:%f\r\n", Uo_0);
		// 	printf("flag1:%f\r\n", A_freq);
        // }
        buf_wave[i] = A_freq;//将幅值存入数组
        freq_array[i] = freq;//将频率存入数组
    }

    // 计算最大值的 0.65 倍和 0.75 倍
    target_A_freq_min = max_A_freq * 0.65;
    target_A_freq_max = max_A_freq * 0.75;


    // 重新遍历数组，找到在范围内的频率
    for (i = 0; i < num_steps-1; i++) {
        if (buf_wave[i] > target_A_freq_max && buf_wave[i + 1] <= target_A_freq_max) {
           	temp_start_freq = freq_array[i];//找到粗略最大值对应频率的位置
      	}
        else if(buf_wave[i] < target_A_freq_min && buf_wave[i - 1] >= target_A_freq_min){
            temp_end_freq = freq_array[i];//找到粗略最小值对应频率的位置
        }
    }
    //再次精细化搜索
    temp_log_start = log10(temp_start_freq);//对数起始频率
    temp_log_end = log10(temp_end_freq);//对数结束频率
    temp_log_step = (temp_log_end - temp_log_start) / num_steps;//对数步长

    for (i = 0; i <= num_steps; i++) {
        temp_log_freq = temp_log_start + i * temp_log_step;//当前对数频率
        freq = (unsigned int)pow(10, temp_log_freq);//对数频率转换为实际频率
        AD9910_FreWrite(freq);//设置频率
        AD9910_AmpWrite(11755); // 设置输出幅度
        // 延时一段时间，以便观察频率变化
        delay_ms(10);
        //printf("此时的扫频频率为:%d\r\n", freq);
		// 读取 ADC 的值，得到频率对应幅值
        ADS1118Init(ADS1118_SS_ONCE,ADS1118_MODE_DC,ADS1118_DR_128,ADS1118_PULL_UP_EN_E,ADS1118_NOP_W);//初始化ADS1118
        temp_A_freq = 0.07 +Getdata(ADS1118_MUX_0G,ADS1118_PGA_10,ADS1118_TS_MODE_ADC,CS_0);//这个地方记得改//得到当前频率对应的幅值

        //printf("此时的放大倍数为:%f", A_freq);
        // 记录最大值和对应的频率
        if (temp_A_freq > temp_max_A_freq) {
            temp_max_A_freq = temp_A_freq;
            temp_max_freq = freq;
        }
        // if (freq >= 250000) {
		// 	printf("flag1:%f\r\n", Uo_0);
		// 	printf("flag1:%f\r\n", A_freq);
        // }
        
        buf_wave[i] = temp_A_freq;//将幅值存入数组
        freq_array[i] = freq;//将频率存入数组
        
    }

    // 计算最大值的 0.7 倍和 0.707 倍
    target_A_freq_min = max_A_freq * 0.7;
    target_A_freq_max = max_A_freq * 0.707;


    // 重新遍历数组，找到在范围内的频率
    for (i = 0; i < num_steps-1 ; i++) {
        if (buf_wave[i] > target_A_freq_max && buf_wave[i + 1] <= target_A_freq_max) {
           	break;
      	}
    }

	//线性差值计算估计出较为准确的上限频率
	k=freq_array[i];
	//printf("满足条件的数%d\r\n",k);
	return k;

}
uint32_t output_freq = 10000;
uint16_t output_amp = 16383;
int uart1_res=0;
int state=0;
int istest=0;
int counter=0;
int main(void){   
	//u16 adcx;//adc采样频
   int i;
	unsigned int start_freq = 100000;  // 起始频率100kHz
	unsigned int end_freq = 5000000; // 结束频率5MHz
	unsigned int num_steps = 40;     // 步数
    
    unsigned int freq_final=0;//最终上限频率
    unsigned int  gainBandwidthProduct=0;//增益带宽积
    unsigned int  pressureSwingRate=0;//压摆率
    unsigned int  staticPowerConsumption=0;//静态功耗
    unsigned int  temp_output_amp  = 0;//临时输出幅度
    unsigned int  temp_output_freq = 0;//临时输出频率
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置中断分组
    //一堆初始化函数
	delay_init();	    //初始化延时函数

	uart_init(115200);//设置uart1串口波特率
	delay_ms(100);	    //延时一会儿，等待上电稳定
    
    LED_Init();			//初始化LED

    //UART4_Init(9600);
	delay_ms(300);	    //延时一会儿，等待上电稳定

	//SPI_FPGA_Init();  //SPI2初始化
	delay_ms(100);	    //延时一会儿，等待上电稳定

	Init_AD9910();			//AD9910控制脚及寄存器初始化
	delay_ms(100);	    //延时一会儿，等待上电稳定

    KEY_Init();//IO初始化
    delay_ms(100);	    //延时一会儿，等待上电稳定

    //TIM3_Int_Init(1,0);

    Adc_Init();//ADC初始化
	
    delay_ms(100);

    ADS1118GPIOInit();//初始化ADS1118的GPIO
	//下面这句话记得每次采样前都要写一遍
	ADS1118Init(ADS1118_SS_ONCE,ADS1118_MODE_DC,ADS1118_DR_128,ADS1118_PULL_UP_EN_E,ADS1118_NOP_W);//初始化ADS1118
    delay_ms(100);	    //延时一会儿，等待上电稳定
	UART5_init(115200);//设置串口屏用波特率
	delay_ms(300);	    //延时一会儿，等待上电稳定

	//设置模块输出 TRIG_WAVE：三角波，SQUARE_WAVE：方波，SINC_WAVE：SINC波)
    
    delay_ms(100);	    //延时一会儿，等待上电稳定
    //AD9910_RAM_WAVE_Set(SQUARE_WAVE);//设置输出波形
	AD9910_FreWrite(10000);	//写输出频率1KHz。范围：0~420000000，对应频率0Hz~420MHz
	AD9910_AmpWrite(10383);	//写输出幅度最大。范围：0~16383对应峰峰值0mv~800mv(左右)

    //程控放大器初始化
    //PD11 PD13 PD15 设为0 0 0，增益0.1dB
    GPIO_ResetBits(GPIOE,GPIO_Pin_11);
    GPIO_ResetBits(GPIOE,GPIO_Pin_13);
    GPIO_ResetBits(GPIOE,GPIO_Pin_15);
    delay_ms(1000);
    //启动串口屏通信
	HMISendstart();
	while(1){
        //先启动，先是第一步展示设置幅度和频率输出一个正弦波，接受串口屏传来的的数据，并将数据传给AD9910
        if(state==0){//状态0：输出频率和幅度调整
            temp_output_freq = output_freq * 0.001 *0.735;//将频率转换为实际频率
            sprintf((char*)buf,"page0.t4.txt=\"%d\"",temp_output_freq);
		    HMISends((char *)buf); //发送Ri的数据给page0页面的t4文本控件
		    HMISendb(0xff);//结束符
            delay_ms(100);
            printf("此时的扫频频率为:%d\r\n", 1);
            
            temp_output_amp= output_amp * 0.04639 + 1.5;//将幅值转换为实际幅值
            sprintf((char*)buf,"page0.t5.txt=\"%d\"",temp_output_amp);
            HMISends((char *)buf); //发送Ri的数据给page0页面的t5文本控件
            HMISendb(0xff);//结束符
            delay_ms(100);

            AD9910_FreWrite(output_freq);	//写输出频率output_freq。范围：0~420000000，对应频率0Hz~420MHz
            AD9910_AmpWrite(output_amp);	//写输出幅度output_amp。范围：0~16383对应峰峰值0mv~800mv(左右)
            
            GPIO_ResetBits(GPIOB,GPIO_Pin_5);
            delay_ms(100);
            GPIO_SetBits(GPIOB,GPIO_Pin_5);

//            GPIO_ResetBits(GPIOG,GPIO_Pin_2);//橙IN2
//            GPIO_ResetBits(GPIOG,GPIO_Pin_3);//蓝IN2
//            GPIO_ResetBits(GPIOG,GPIO_Pin_4);//蓝IN1
            delay_ms(1000);
             GPIO_SetBits(GPIOG,GPIO_Pin_2);
             GPIO_SetBits(GPIOG,GPIO_Pin_3);
            //GPIO_SetBits(GPIOG,GPIO_Pin_4);
 
            uart1_res = Get_Adc(0);
        }
        else if(state==1){//状态1：准备进行测试
            //如果按下了串口屏上的测试键，那么就开始测试流程
            //继电器全部断开
            GPIO_ResetBits(GPIOG,GPIO_Pin_2);//橙IN2
            GPIO_ResetBits(GPIOG,GPIO_Pin_3);//蓝IN2
            GPIO_ResetBits(GPIOG,GPIO_Pin_4);//蓝IN1
            if(istest){
                //首先进行对数扫频，得到最大幅值对应的频率
                //继电器控制，切换到对数扫频模式
                GPIO_SetBits(GPIOG,GPIO_Pin_2);//橙IN2置1（导通）
                GPIO_SetBits(GPIOG,GPIO_Pin_3);//蓝IN2置1（导通）

                AD9910_AmpWrite(11255); // 设置输出幅度
                
                //PD11 PD13 PD15 设为0 0 0，增益0.1dB
                GPIO_ResetBits(GPIOE,GPIO_Pin_11);
                GPIO_ResetBits(GPIOE,GPIO_Pin_13);
                GPIO_ResetBits(GPIOE,GPIO_Pin_15);

                delay_ms(1000);
                freq_final=log_sweep(start_freq, end_freq, num_steps);//对数扫频,得到最大幅值对应的频率
                //计算增益带宽积
                gainBandwidthProduct = freq_final;//计算增益带宽积
                //发送给串口屏
                sprintf((char*)buf,"page0.t10.txt=\"%d\"",gainBandwidthProduct);
                HMISends((char *)buf); //发送Ri的数据给page0页面的t10文本控件
                HMISendb(0xff);//结束符
                delay_ms(1000);

                //转换压摆率测试模式，片上DAC输出方波，等待FPGA计算压摆率完毕，接受后发送给串口屏
                //继电器控制，切换到压摆率测试模式
                GPIO_SetBits(GPIOG,GPIO_Pin_2);//橙IN2置1（导通）
                GPIO_ResetBits(GPIOG,GPIO_Pin_3);//蓝IN2置0（断开）
                GPIO_SetBits(GPIOG,GPIO_Pin_4);//蓝IN1置1（导通）

                //DAC输出方波
                AD9910_RAM_WAVE_Set(SQUARE_WAVE);//设置输出方波
                //调整程控放大器的增益为10倍 1 1 1
                GPIO_SetBits(GPIOE,GPIO_Pin_11);
                GPIO_SetBits(GPIOE,GPIO_Pin_13);
                GPIO_SetBits(GPIOE,GPIO_Pin_15);
                delay_ms(4000);

                //拉低FPGA的某个引脚，通知FPGA开始计算压摆率
                GPIO_SetBits(GPIOB,GPIO_Pin_5);
                GPIO_ResetBits(GPIOB,GPIO_Pin_5);

                // while(uart1_flag==0){
                //        USART_SendData(USART1,1);
                // };//等待FPGA传来数据

                //这里接受FPGA传来的数据
                delay_ms(100);
                pressureSwingRate = vpp;//这里是FPGA传来的数据

                //发送给串口屏
                sprintf((char*)buf,"page0.t11.txt=\"%d\"",pressureSwingRate);
                HMISends((char *)buf); //发送Ri的数据给page0页面的t12文本控件
                HMISendb(0xff);//结束符
                delay_ms(1000);

                //转换为静态功耗测试模式，接受adc另外几个通道的数据，进行公式计算，发送给串口屏
                //继电器控制，切换到静态功耗测试模式
                //橙IN2置0（断开）
                GPIO_ResetBits(GPIOG,GPIO_Pin_2);
                //蓝IN2置0（断开）
                GPIO_ResetBits(GPIOG,GPIO_Pin_3);
                //蓝IN1置0（断开）
                GPIO_ResetBits(GPIOG,GPIO_Pin_4);
                
                for(i=0;i<10;i++){//采样10次,计算平均值
                    ADS1118Init(ADS1118_SS_ONCE,ADS1118_MODE_DC,ADS1118_DR_128,ADS1118_PULL_UP_EN_E,ADS1118_NOP_W);//初始化ADS1118
                    Uo_1 = Getdata(ADS1118_MUX_2G,ADS1118_PGA_10,ADS1118_TS_MODE_ADC,CS_0);//这个地方记得改//得到当前频率对应的幅值
                    delay_ms(100);
                    //Uo_2 = Getdata(ADS1118_MUX_3G,ADS1118_PGA_10,ADS1118_TS_MODE_ADC,CS_0);//这个地方记得改//得到当前频率对应的幅值
                    //delay_ms(100);
                    Vavgs[i] = Uo_1 ;
                }
                Vavg = 100 * (Vavgs[0] + Vavgs[1] + Vavgs[2] + Vavgs[3] + Vavgs[4] + Vavgs[5] + Vavgs[6] + Vavgs[7] + Vavgs[8] + Vavgs[9]);
                //转mV
                Vavg =1.0505 * Vavg + 28.724;
                //单位：uA
                Iavg = Vavg / 1.0795;//从Vavg计算得到的电流值

                //单位：uW
                staticPowerConsumption = 24 * Iavg*0.7;//这里是计算得到的数据
                //发送给串口屏
                sprintf((char*)buf,"page0.t13.txt=\"%d\"",staticPowerConsumption);
                HMISends((char *)buf); //发送Ri的数据给page0页面的t13文本控件
                HMISendb(0xff);//结束符
                delay_ms(1000);
                //测试完毕，重置状态
                //串口屏状态重置
                sprintf((char*)buf,"page0.sw0.val=0");
                HMISends((char *)buf); //发送Ri的数据给page0页面的t10文本控件
                HMISendb(0xff);//结束符
                delay_ms(1000);
                sprintf((char*)buf,"page0.b1.txt=\"启动\"");
                HMISends((char *)buf); //发送Ri的数据给page0页面的t10文本控件
                HMISendb(0xff);//结束符
                delay_ms(1000);

                istest = 0;
                state = 0;
            }

            //先进行扫频操作，然后计算出增益带宽积，发送给串口屏
            //转换压摆率测试模式，ad9910输出方波，等待FPGA计算压摆率完毕，接受后发送给串口屏
            //转换为静态功耗测试模式，接受adc另外几个通道的数据，进行公式计算，发送给串口屏
            //启动扫频
            
        }
		
		
	}
}

