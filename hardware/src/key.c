/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	key.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		按键IO初始化，按键功能判断
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//按键头文件
#include "key.h"

//硬件驱动
#include "delay.h"




/*
************************************************************
*	函数名称：	Key_Init
*
*	函数功能：	按键IO初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		SW2-PD2		SW3-PC11	SW4-PC12	SW5-PC13	
*				按下为低电平		释放为高电平
************************************************************
*/
void Key_Init(void)
{

	GPIO_InitTypeDef gpioInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);	//打开GPIOC、GPIOD的时钟
	
	gpioInitStructure.GPIO_Mode = GPIO_Mode_IPU;									//设置为上拉输入模式
	gpioInitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;			//初始化Pin11、12、13
	gpioInitStructure.GPIO_Speed = GPIO_Speed_50MHz;								//承载的最大频率
	GPIO_Init(GPIOC, &gpioInitStructure);											//初始化GPIOC
	
	gpioInitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &gpioInitStructure);											//初始化GPIOD

}

/*
************************************************************
*	函数名称：	KeyScan
*
*	函数功能：	按键电平扫描
*
*	入口参数：	GPIOX：需要扫描的GPIO组	NUM：该GPIO组内的编号
*
*	返回参数：	IO电平状态
*
*	说明：		
************************************************************
*/
_Bool KeyScan(GPIO_TypeDef* GPIOX, unsigned int NUM)
{
	
	if(GPIOX == GPIOC)
	{
		if(!GPIO_ReadInputDataBit(GPIOC, NUM))	//按下  为低
		{
			return KEYDOWN;
		}
		else									//弹起  为高
		{
			return KEYUP;
		}
	}
	else if(GPIOX == GPIOD)
	{
		if(!GPIO_ReadInputDataBit(GPIOD, NUM))	//按下  为低
		{
			return KEYDOWN;
		}
		else									//弹起  为高
		{
			return KEYUP;
		}
	}
	
	return KEYUP;								//默认返回按键释放
	
}

/*
************************************************************
*	函数名称：	Keyboard
*
*	函数功能：	按键功能检测
*
*	入口参数：	
*
*	返回参数：	按键动作
*
*	说明：		分单击、双击、长安
************************************************************
*/
unsigned char Keyboard(void)
{
	
	static unsigned char keyStatus = 0;
	
	if(KeyScan(GPIOC, KEY0) == KEYDOWN)
	{
		keyStatus |= 0x01;							//标记KEY0按下
		
		while(KeyScan(GPIOC, KEY0) != KEYUP)		//等待释放
			return KEYNONE;
	}
	else
	{
		if(keyStatus & 0x01)
		{
			keyStatus &= 0xfe;						//标记KEY0释放
			return KEY0DOWN;
		}
	}
	
	if(KeyScan(GPIOC, KEY1) == KEYDOWN)
	{
		keyStatus |= 0x02;							//标记KEY1按下
		
		while(KeyScan(GPIOC, KEY1) != KEYUP)		//等待释放
			return KEYNONE;
	}
	else
	{
		if(keyStatus & 0x02)
		{
			keyStatus &= 0xfd;						//标记KEY1释放
			return KEY1DOWN;
		}
	}
	
	if(KeyScan(GPIOC, KEY2) == KEYDOWN)
	{
		keyStatus |= 0x04;							//标记KEY2按下
		
		while(KeyScan(GPIOC, KEY2) != KEYUP)		//等待释放
			return KEYNONE;
	}
	else
	{
		if(keyStatus & 0x04)
		{
			keyStatus &= 0xfb;						//标记KEY2释放
			return KEY2DOWN;
		}
	}
	
	if(KeyScan(GPIOD, KEY3) == KEYDOWN)
	{
		keyStatus |= 0x08;							//标记KEY3按下
		
		while(KeyScan(GPIOD, KEY3) != KEYUP)		//等待释放
			return KEYNONE;
	}
	else
	{
		if(keyStatus & 0x08)
		{
			keyStatus &= 0xf7;						//标记KEY3释放
			return KEY3DOWN;
		}
	}
	
	return KEYNONE;
	
}
