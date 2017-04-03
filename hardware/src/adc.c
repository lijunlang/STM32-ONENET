#include "adc.h"








void ADC_ChInit(ADC_TypeDef * ADCx, _Bool temp)
{

	ADC_InitTypeDef adcInitStruct;
	
	if(ADCx == ADC1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	else if(ADCx == ADC2)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	else
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	
	ADC_DeInit(ADCx); //复位ADCx,将外设 ADCx 的全部寄存器重设为缺省值
	
	adcInitStruct.ADC_ContinuousConvMode = DISABLE; //模数转换工作在单次转换模式
	adcInitStruct.ADC_DataAlign = ADC_DataAlign_Right; //ADC数据右对齐
	adcInitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //转换由软件而不是外部触发启动
	adcInitStruct.ADC_Mode = ADC_Mode_Independent; //ADC工作模式:ADC1和ADC2工作在独立模式
	adcInitStruct.ADC_NbrOfChannel = 1; //顺序进行规则转换的ADC通道的数目
	adcInitStruct.ADC_ScanConvMode = DISABLE; //模数转换工作在单通道模式
	ADC_Init(ADCx, &adcInitStruct);	//根据adcInitStruct中指定的参数初始化外设ADCx的寄存器
	
	if(ADCx == ADC1 && temp)
		ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器//ADC1通道16
	
	ADC_Cmd(ADCx, ENABLE); //使能指定的ADC1
	
	ADC_ResetCalibration(ADCx);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADCx));	//等待复位校准结束
	
	ADC_StartCalibration(ADCx); //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADCx)); //等待校准结束

}

unsigned short ADC_GetValue(ADC_TypeDef * ADCx, unsigned char ch)
{

	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADCx, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);		//使能指定的ADC1的软件转换启动功能
	 
	while(!ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC )); //等待转换结束

	return ADC_GetConversionValue(ADCx);	//返回最近一次ADC1规则组的转换结果

}

float ADC_GetValueTimes(ADC_TypeDef * ADCx, unsigned char ch, unsigned char times)
{

	float adcValue = 0;
	unsigned char i = 0;
	
	for(; i < times; i++)
	{
		adcValue += (float)ADC_GetValue(ADC1, ch);
	}
	
	return adcValue / times;

}

float ADC_GetTemperature(void)
{

	float temp = ADC_GetValueTimes(ADC1, ADC_Channel_16, 10); //获取原始AD数据
	
	temp = temp * 3.3 / 4096; //转换为电压值
	
	return (1.43 - temp) / 0.0043 + 25; //计算出当前温度值

}
