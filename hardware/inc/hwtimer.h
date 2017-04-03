#ifndef _HWTIMER_H_
#define _HWTIMER_H_

#include "stm32f10x.h"





typedef struct
{

	unsigned int timer6Out;

} TIM_INFO;

extern TIM_INFO timInfo;


#define OS_TIMER	TIM6

#define NET_TIME	60			//设定时间--单位秒
extern unsigned short timerCount;



void Timer1_8_Init(TIM_TypeDef * TIMx, unsigned short arr, unsigned short psc);

void Timer6_7_Init(TIM_TypeDef * TIMx, unsigned short arr, unsigned short psc);

void RTOS_TimerInit(void);


#endif
