#include "AD9833.h"
//输出时，上升沿更新，低电平延迟
//输入时，高电平延迟，下降沿读取



void AD9833_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能 GPIOE 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    
    // 配置 CS1 和 CS2 引脚
    GPIO_InitStructure.GPIO_Pin = AD9833_CS1_PIN | AD9833_CS2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 无上下拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 50MHz 速度
    GPIO_Init(AD9833_PORT, &GPIO_InitStructure);
    
    // 配置 SCLK 引脚
    GPIO_InitStructure.GPIO_Pin = AD9833_SCLK_PIN;
    GPIO_Init(AD9833_PORT, &GPIO_InitStructure);
    
    // 配置 PICO 引脚
    GPIO_InitStructure.GPIO_Pin = AD9833_PICO_PIN;
    GPIO_Init(AD9833_PORT, &GPIO_InitStructure);
    
    // 设置默认状态
    GPIO_SetBits(AD9833_PORT, AD9833_CS1_PIN | AD9833_CS2_PIN); // CS1,CS2高
    GPIO_ResetBits(AD9833_PORT, AD9833_SCLK_PIN);              // SCLK低
    GPIO_ResetBits(AD9833_PORT, AD9833_PICO_PIN);              // PICO低
}
static double ref_clk = 25000000.0; //参考时钟频率
/*******************************************************************************
* 函数名: AD9833_CS_N 静态函数
* 输入参数:	cs1:是否使能通道1片选 cs2:是否使能通道2片选
* 输出参数:	无
* 描述:		使能片选
*******************************************************************************/
static void AD9833_CS_N(bool cs1, bool cs2){
	if(cs1)
		CS1_N;
	if(cs2)
		CS2_N;
	return;
}
/*******************************************************************************
* 函数名: AD9833_CS_P 静态函数
* 输入参数:	cs1:是否使能通道1片选 cs2:是否使能通道2片选
* 输出参数:	无
* 描述:	取消片选
*******************************************************************************/
static void AD9833_CS_P(bool cs1, bool cs2){
	if(cs1)
		CS1_P;
	if(cs2)
		CS2_P;
	return;
}
/*******************************************************************************
* 函数名: SoftSPI_Send 静态函数
* 输入参数:	data:发送的数据
* 输出参数:	无
* 描述:		使用软件SPI发送数据
* 注意:		不改变片选的情况下发送数据
*******************************************************************************/
static void SoftSPI_Send(uint16_t data){//不改变片选的情况下发送数据
	uint16_t mask = 0x8000;
	for(uint8_t i = 0;i<16;i++){
		if(mask&data)
			PICO_P;
		else
			PICO_N;
		WAIT;
		SCLK_N;
		WAIT;
		SCLK_P;
		mask >>= 1;
	}
}
/*******************************************************************************
* 函数名: AD9833_Init
* 输入参数:	无
* 输出参数:	无
* 描述:		初始化AD9833
* 注意:		默认复位，参考时钟频率为25MHz
*******************************************************************************/
void AD9833_Init(void){
	AD9833_Reset(1,1);
}
/*******************************************************************************
 *  函数名: AD9833_Reset
 *  输入参数:	channel1_en:是否使能通道1 channel2_en:是否使能通道2
 *  输出参数:	无
 *  描述:		复位AD9833
 ******************************************************************************/
void AD9833_Reset(bool channel1_en, bool channel2_en){
	uint16_t data = COMMAND_RESET|AD9833_SEL_CTR; //复位命令
	AD9833_CS_N(channel1_en, channel2_en); //控制片选
	SoftSPI_Send(data); //复位
	AD9833_CS_P(channel1_en, channel2_en); //片选拉高
}
/*******************************************************************************
 *  函数名: AD9833_SetRefClk
 *  输入参数:	freq:参考时钟频率
 *  输出参数:	无
 *  描述:		设置参考时钟频率 仅仅是为了修改这个文件中的参考频率进行计算 AD9833内部没有相关的寄存器
 ******************************************************************************/
void AD9833_SetRefClk(double freq){
	if(freq > 0.0)
		ref_clk = freq; //设置参考时钟频率
	else
	
	ref_clk = 25000000.0; //默认25MHz
}
/*******************************************************************************
 *  函数名: AD9833_WriteFreqReg
 *  输入参数:	word:要写入的频率寄存器值 channel1_en:是否使能通道1 channel2_en:是否使能通道2
 *  输出参数:	无
 *  描述:		写入频率寄存器 频率寄存器28bits 根据输出的参考时钟频率进行频率控制字的修改 可以获得稳定的输出频率
 ******************************************************************************/
void AD9833_WriteFreqReg(uint32_t word, bool channel1_en, bool channel2_en){
	uint16_t data = 0;
	data = COMMAND_B28;
	AD9833_CS_N(channel1_en, channel2_en); //控制片选
	SoftSPI_Send(data);//控制
	AD9833_CS_P(channel1_en, channel2_en); //片选拉高
	WAIT;
	data = word&0x00003fff; //取低14位
	data |= AD9833_SEL_FREQ0; //设置为频率寄存器0
	AD9833_CS_N(channel1_en, channel2_en); //控制片选
	SoftSPI_Send(data);
	AD9833_CS_P(channel1_en, channel2_en); //片选拉高
	WAIT;
	data = (word>>14)&0x00003fff; //取高14位
	data |= AD9833_SEL_FREQ0; //设置为频率寄存器0
	AD9833_CS_N(channel1_en, channel2_en); //控制片选
	SoftSPI_Send(data);
	AD9833_CS_P(channel1_en, channel2_en); //片选拉高
}
/*******************************************************************************
 *  函数名: AD9833_SetFreq
 *  输入参数:	freq:要设置的频率 channel1_en:是否使能通道1 channel2_en:是否使能通道2
 *  输出参数:	无
 *  描述:		设置频率 根据参考时钟频率计算寄存器值，并设置
 ******************************************************************************/
void AD9833_SetFreq(double freq, bool channel1_en, bool channel2_en){
	uint32_t word = (uint32_t)(freq/ref_clk*0x10000000); //计算寄存器值
 	AD9833_WriteFreqReg(word, channel1_en, channel2_en);
}
/*******************************************************************************
 *  函数名: AD9833_AddPhaseReg
 *  输入参数:	data:要增加的相位寄存器值 channel1_en:是否使能通道1 channel2_en:是否使能通道2
 *  输出参数:	无
 *  描述:		将相位寄存器的值加上data 相位寄存器12bits
 *  注意:		是加上相位data 不是设置为相位data
 ******************************************************************************/
void AD9833_AddPhaseReg(uint16_t data, bool channel1_en, bool channel2_en){
	
	SoftSPI_Send(0);
	data &= 0x0FFF; //取低12位
	data |= AD9833_SEL_PHASE0; //设置为相位寄存器0
	AD9833_CS_N(channel1_en, channel2_en); //控制片选
	SoftSPI_Send(data);
	AD9833_CS_P(channel1_en, channel2_en); //片选拉高
}

/*******************************************************************************
 *  函数名: AD9833_AddPhase
 *  输入参数:	phase:要增加的相位值 channel1_en:是否使能通道1 channel2_en:是否使能通道2
 *  输出参数:	无
 *  描述:		将输出信号的的值加上phase
 *  注意:		是加上相位phase 不是设置为相位phase phase的范围是0-360度
 ******************************************************************************/
void AD9833_AddPhase(double phase, bool channel1_en, bool channel2_en){
	while(phase>360)
		phase -= 360;
	while(phase<=0)
		phase += 360;
	uint16_t data = (uint16_t)(phase/360*4096) | AD9833_SEL_PHASE0; //设置相位寄存器0
	AD9833_AddPhaseReg(data, channel1_en, channel2_en);
}
/*******************************************************************************
 *  函数名: AD9833_SetWaveform
 *  输入参数:	waveform:波形类型 channel1_en:是否使能通道1 channel2_en:是否使能通道2
 *  输出参数:	无
 *  描述:		设置波形类型 waveform的值为0-2 分别对应正弦波、三角波、方波
 *  注意:		如果不支持的波形类型，则默认为正弦波
 ******************************************************************************/
void AD9833_SetWaveform(uint8_t waveform,bool channel1_en, bool channel2_en){
	uint16_t data = 0;
	switch(waveform){
		case 0: //正弦波
		data = 0; //默认正弦波
		break;
		case 1: //三角波
		data = COMMAND_MODE|AD9833_SEL_CTR;
		break;
		case 2: //方波
		data = COMMAND_OPBITEN|COMMAND_DIV2|AD9833_SEL_CTR;
		break;
		default: //默认正弦波
		data = 0;
		break;
	}
	AD9833_CS_N(channel1_en, channel2_en); //控制片选
	SoftSPI_Send(data);
	AD9833_CS_P(channel1_en, channel2_en); //片选拉高
}
/*******************************************************************************
 *  函数名: AD9833_ALLInit
 *  输入参数:	freq_0:通道1频率 phase_0:通道1相位 wave_0:通道1波形
 *            freq_1:通道2频率 phase_1:通道2相位 wave_1:通道2波形
 *  输出参数:	无
 *  描述:		初始化AD9833，设置参考时钟频率、通道1和通道2的频率、相位和波形
 ******************************************************************************/
void AD9833_ALLInit(double freq_0,double phase_0, uint8_t wave_0,
					double freq_1,double phase_1, uint8_t wave_1){
 AD9833_Reset(1,1);
 AD9833_SetRefClk(25000000.0); //设置参考时钟频率
 AD9833_SetFreq(freq_0, true, false); //设置通道1频率
 AD9833_AddPhase(phase_0, true, false); //设置通道1相位
 AD9833_SetWaveform(wave_0, true, false); //设置通道1波形
 AD9833_SetFreq(freq_1, false, true); //设置通道2频率
 AD9833_AddPhase(phase_1, false, true); //设置通道2相位
 AD9833_SetWaveform(wave_1, false, true); //设置通道2波形
}
/*******************************************************************************
 *  函数名: AD9833_Sleep
 *  输入参数:	sleep1_choice:休眠1选择 sleep2_chioce:休眠2选择
 *  输出参数:	无
 *  描述:		设置AD9833的休眠模式 sleep1_choice和sleep2_chioce的值为0-2
 *				分别对应不同的休眠模式
 * 				0:不休眠 1:DAC休眠 2:9833的内部时钟休眠，相位累加器不工作 3:全部休眠
 ******************************************************************************/
void AD9833_Sleep(uint8_t sleep1_choice,uint8_t sleep2_chioce){
	sleep1_choice = ((sleep1_choice%4)<<6)|AD9833_SEL_CTR;
	sleep2_chioce = ((sleep2_chioce%4)<<6)|AD9833_SEL_CTR;
	AD9833_CS_N(true, false); //控制片选
	SoftSPI_Send(sleep1_choice); //发送休眠1命令
	AD9833_CS_P(true, false); //片选拉高
	AD9833_CS_N(false, true); //控制片选
	SoftSPI_Send(sleep2_chioce); //发送休眠2命令
	AD9833_CS_P(false, true); //片选拉高
	return;
}
