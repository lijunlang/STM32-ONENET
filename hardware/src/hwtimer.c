/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	hwtimer.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		单片机定时器初始化
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//协议层
#include "onenet.h"

//网络设备
#include "net_device.h"

//硬件驱动
#include "hwtimer.h"
#include "selfcheck.h"


unsigned short timerCount = 0;	//时间计数--单位秒


TIM_INFO timInfo = {0};



/*
************************************************************
*	函数名称：	Timer1_8_Init
*
*	函数功能：	Timer1或8的PWM配置
*
*	入口参数：	TIMx：TIM1 或者 TIM8
*				arr：重载值
*				psc分频值
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Timer1_8_Init(TIM_TypeDef * TIMx, unsigned short arr, unsigned short psc)
{
	
	GPIO_InitTypeDef gpioInitStruct;
	TIM_TimeBaseInitTypeDef timerInitStruct;
	TIM_OCInitTypeDef timerOCInitStruct;

	if(TIMx == TIM1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	}
	else
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	}
	
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpioInitStruct);	
	
	timerInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStruct.TIM_Period = arr;
	timerInitStruct.TIM_Prescaler = psc;
	TIM_TimeBaseInit(TIMx, &timerInitStruct);
	
	timerOCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;				//选择定时器模式:TIM脉冲宽度调制模式2
 	timerOCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	timerOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;		//输出极性:TIM输出比较极性低
	timerOCInitStruct.TIM_Pulse = 0;
	TIM_OC2Init(TIMx, &timerOCInitStruct);
	TIM_OC3Init(TIMx, &timerOCInitStruct);
	
	TIM_CtrlPWMOutputs(TIMx, ENABLE);							//MOE 主输出使能	
	
	TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);			//使能TIMx在CCR1上的预装载寄存器
	TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);			//使能TIMx在CCR1上的预装载寄存器
 
	TIM_ARRPreloadConfig(TIMx, ENABLE);							//ARPE使能
	
	TIM_Cmd(TIMx, ENABLE);										//使能TIMx

}

/*
************************************************************
*	函数名称：	Timer6_7_Init
*
*	函数功能：	Timer6或7的定时配置
*
*	入口参数：	TIMx：TIM6 或者 TIM7
*				arr：重载值
*				psc分频值
*
*	返回参数：	无
*
*	说明：		timer6和timer7只具有更新中断功能
************************************************************
*/
void Timer6_7_Init(TIM_TypeDef * TIMx, unsigned short arr, unsigned short psc)
{

	TIM_TimeBaseInitTypeDef timerInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	
	if(TIMx == TIM6)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
		
		nvicInitStruct.NVIC_IRQChannel = TIM6_IRQn;
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
		
		nvicInitStruct.NVIC_IRQChannel = TIM7_IRQn;
	}
	
	timerInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStruct.TIM_Period = arr;
	timerInitStruct.TIM_Prescaler = psc;
	
	TIM_TimeBaseInit(TIMx, &timerInitStruct);
	
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);					//使能更新中断
	
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&nvicInitStruct);
	
	TIM_Cmd(TIMx, ENABLE); //使能定时器

}

/*
************************************************************
*	函数名称：	TIM6_IRQHandler
*
*	函数功能：	RTOS的心跳定时中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void TIM6_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
	{
		//do something...
		if(++timInfo.timer6Out >= 4294967290UL)
			timInfo.timer6Out = 0;
		
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}

}

/*
************************************************************
*	函数名称：	TIM7_IRQHandler
*
*	函数功能：	Timer7更新中断服务函数
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void TIM7_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
	{
		if(oneNetInfo.netWork == 0)											//如果网络断开
		{
			if(++timerCount >= NET_TIME) 									//如果网络断开超时
			{	
				checkInfo.NET_DEVICE_OK = 0;								//置设备未检测标志
				
				NET_DEVICE_ReConfig(0);										//设备初始化步骤设置为开始状态
				
				oneNetInfo.netWork = 0;
			}
		}
		else
		{
			timerCount = 0;													//清除计数
		}
		
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	}

}
