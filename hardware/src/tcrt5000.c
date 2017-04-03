/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	tcrt500.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2017-03-07
	*
	*	版本： 		V1.0
	*
	*	说明： 		红外对管距离检测驱动
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "adc.h"
#include "tcrt5000.h"



TCRT5000_INFO t5000Info;




/*
************************************************************
*	函数名称：	TCRT5000_Init
*
*	函数功能：	初始化控制脚和ADC1_ch13
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void TCRT5000_Init(void)
{

	GPIO_InitTypeDef gpioInitStrcut;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_5;
	gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitStrcut);
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_AIN;
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC, &gpioInitStrcut);
	
	ADC_ChInit(ADC1, 0);
	
	TCRT5000_Set(TCRT5000_ON);

}

/*
************************************************************
*	函数名称：	TCRT5000_GetValue
*
*	函数功能：	获取红外接收管的AD值
*
*	入口参数：	times：采集次数，平均值
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void TCRT5000_GetValue(unsigned char times)
{

	t5000Info.voltag = ADC_GetValueTimes(ADC1, ADC_Channel_13, times);

}

/*
************************************************************
*	函数名称：	TCRT5000_Set
*
*	函数功能：	控制脚
*
*	入口参数：	status：TCRT5000_ON-打开距离检测		TCRT5000_OFF：关闭距离检测
*
*	返回参数：	无
*
*	说明：		TCRT5000_OFF的时候电压测量值低于200
************************************************************
*/
void TCRT5000_Set(TCRT5000_ENUM status)
{

	GPIO_WriteBit(GPIOA, GPIO_Pin_5, status != TCRT5000_ON ? Bit_SET : Bit_RESET);
	
	t5000Info.status = status;

}
