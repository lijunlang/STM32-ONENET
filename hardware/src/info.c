/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	info.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2017-02-23
	*
	*	版本： 		V1.1
	*
	*	说明： 		V1.0：SSID、PSWD、DEVID、APIKEY保存及读取。
	*				V1.1：取消了SSID和PSWD的保存和读写，替换为了智能配网，wifi类型的网络设备可以自动保存。
	*
	*				重要：只有当外部存储器存在时，才从中读取信息
	*					  若不存在，会读取固化在代码里的信息
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "info.h"
#include "at24c02.h"
#include "delay.h"
#include "usart.h"

//协议
#include "onenet.h"

//C库
#include <string.h>
#include <stdlib.h>




/*
************************************************************
*	函数名称：	Info_Check
*
*	函数功能：	检查信息是否存在
*
*	入口参数：	无
*
*	返回参数：	检查结果
*
*	说明：		判断wifi的ssid和pswd是否存在
*				0-ok	1-无ssid	2-无pswd
*				3-无devid	4-无apikey
************************************************************
*/
unsigned char Info_Check(void)
{
	
	unsigned char rData = 0;
	
	AT24C02_ReadByte(DEVID_ADDRESS, &rData);	//读取长度值
	if(rData == 0 || rData >= 10)				//如果为0或超出
		return 1;
	
	AT24C02_ReadByte(AKEY_ADDRESS, &rData);		//读取长度值
	if(rData == 0 || rData >= 30)				//如果为0或超出
		return 2;
        
	return 0;

}

/*
************************************************************
*	函数名称：	Info_WifiLen
*
*	函数功能：	获取信息长度
*
*	入口参数：	sp：需要检查的信息-见说明
*
*	返回参数：	检查结果
*
*	说明：		获取0-ssid长度	1-pswd长度	
*				2-devid长度		3-apikey长度
************************************************************
*/
unsigned char Info_WifiLen(unsigned char sp)
{
	
	unsigned char len = 0;
    
    switch(sp)
    {
        case 0:
            AT24C02_ReadByte(DEVID_ADDRESS, &len);		//读取长度值
			if(len == 0 || len >= 10)					//如果为0或超出
				return 1;
        break;
        
        case 1:
            AT24C02_ReadByte(AKEY_ADDRESS, &len);		//读取长度值
			if(len == 0 || len >= 30)					//如果为0或超出
				return 1;
        break;
    }
	
	return len;

}

/*
************************************************************
*	函数名称：	Info_CountLen
*
*	函数功能：	计算字段长度
*
*	入口参数：	info：需要检查的字段
*
*	返回参数：	字段长度
*
*	说明：		计算串1发过来的字段长度   以"\r\n"结尾
************************************************************
*/
unsigned char Info_CountLen(char *info)
{

	unsigned char len = 0;
	char *buf = strstr(info, ":");		//找到':'
	
	buf++;								//偏移到下一个字节，代表字段信息开始
	while(1)
	{
		if(*buf == '\r')				//直到'\r'为止
			return len;
		
		buf++;
		len++;
	}

}

/*
************************************************************
*	函数名称：	Info_Read
*
*	函数功能：	读取ssid、pswd、devid、apikey
*
*	入口参数：	无
*
*	返回参数：	读取结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
_Bool Info_Read(void)
{
	
    memset(oneNetInfo.devID, 0, sizeof(oneNetInfo.devID));											//清除之前的内容
	AT24C02_ReadBytes(DEVID_ADDRESS + 1, (unsigned char *)oneNetInfo.devID, Info_WifiLen(0));		//获取devid长度  读devid
    DelayXms(10);																					//延时
                
    memset(oneNetInfo.apiKey, 0, sizeof(oneNetInfo.apiKey));										//清除之前的内容
	AT24C02_ReadBytes(AKEY_ADDRESS + 1, (unsigned char *)oneNetInfo.apiKey, Info_WifiLen(1));		//获取apikey长度  读apikey

    return 0;

}

/*
************************************************************
*	函数名称：	Info_Alter
*
*	函数功能：	更改wifi信息和项目信息
*
*	入口参数：	需要保存的字段
*
*	返回参数：	保存结果
*
*	说明：		0-不需要重新连接		1-需要重新连接
************************************************************
*/
_Bool Info_Alter(char *info)
{
    
    char *usart1Tmp;
    unsigned char usart1Count = 0;
	_Bool flag = 0;
        
	if((usart1Tmp = strstr(info, "DEVID:")) != (void *)0)								//提取devid
	{
		usart1Count = Info_CountLen(usart1Tmp);											//计算长度
        if(usart1Count > 0)
        {
            memset(oneNetInfo.devID, 0, sizeof(oneNetInfo.devID));						//清除之前的内容
            strncpy(oneNetInfo.devID, usart1Tmp + 6, usart1Count);
            UsartPrintf(USART_DEBUG, "Tips:	Save DEVID: %s\r\n", oneNetInfo.devID);

			AT24C02_WriteByte(DEVID_ADDRESS, strlen(oneNetInfo.devID));					//保存devid长度
			DelayXms(10);
			AT24C02_WriteBytes(DEVID_ADDRESS + 1,										//保存ssid
								(unsigned char *)oneNetInfo.devID,
								strlen(oneNetInfo.devID));
            
            flag = 1;
        }
	}
        
	if((usart1Tmp = strstr(info, "APIKEY:")) != (void *)0)								//提取apikey
	{
		usart1Count = Info_CountLen(usart1Tmp);											//计算长度
        if(usart1Count > 0)
        {
            memset(oneNetInfo.apiKey, 0, sizeof(oneNetInfo.apiKey));					//清除之前的内容
            strncpy(oneNetInfo.apiKey, usart1Tmp + 7, usart1Count);
            UsartPrintf(USART_DEBUG, "Tips:	Save APIKEY: %s\r\n", oneNetInfo.apiKey);

			AT24C02_WriteByte(AKEY_ADDRESS, strlen(oneNetInfo.apiKey));					//保存apikey长度
			DelayXms(10);
			AT24C02_WriteBytes(AKEY_ADDRESS + 1,										//保存ssid
								(unsigned char *)oneNetInfo.apiKey,
								strlen(oneNetInfo.apiKey));
            
            flag = 1;
        }
	}
	
	return flag;

}
