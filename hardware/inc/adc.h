#ifndef _ADC_H_
#define _ADC_H_


#include "stm32f10x.h"




void ADC_ChInit(ADC_TypeDef * ADCx, _Bool temp);

unsigned short ADC_GetValue(ADC_TypeDef * ADCx, unsigned char ch);

float ADC_GetValueTimes(ADC_TypeDef * ADCx, unsigned char ch, unsigned char times);

float ADC_GetTemperature(void);


#endif
