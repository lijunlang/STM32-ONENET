/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	led.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2017-3-28
	*
	*	版本： 		V1.0
	*
	*	说明： 		LED初始化，开关继电器
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/
//单片机头文件
#include "stm32f10x.h"

//继电器头文件
#include "relay.h"


RELAY_STATUS relayStatus;


/*
************************************************************
*	函数名称：	Relay_Init
*
*	函数功能：	Relay初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		RELAY4-PA6	RELAY5-PA8	
				高电平关		低电平开
************************************************************
*/
void Relay_Init(void)
{
	
	GPIO_InitTypeDef gpioInitStrcut;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 ;
	gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitStrcut);

    
    J1_Set(RELAY_OFF);
    J2_Set(RELAY_OFF);


}

/*
************************************************************
*	函数名称：	J1_Set
*
*	函数功能：	继电器1控制
*
*	入口参数：	status：RELAY_ON-开	RELAY_OFF-关
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void J1_Set(RELAY_ENUM status)
{

	GPIO_WriteBit(GPIOA, GPIO_Pin_6, status != RELAY_ON ? Bit_SET : Bit_RESET);
	relayStatus.J1Sta = status;

}

/*
************************************************************
*	函数名称：	J2_Set
*
*	函数功能：	继电器2控制
*
*	入口参数：	status：RELAY_ON-开	RELAY_OFF-关
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void J2_Set(RELAY_ENUM status)
{

	GPIO_WriteBit(GPIOA, GPIO_Pin_8, status != RELAY_ON ? Bit_SET : Bit_RESET);
	relayStatus.J2Sta = status;

}