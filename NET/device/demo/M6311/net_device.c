/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	net_device.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-03-02
	*
	*	版本： 		V1.1
	*
	*	说明： 		网络设备应用层
	*
	*	修改记录：	V1.1：1.平台IP和PORT通过参数传入的方式确定，解决了不同协议网络设备驱动不通用的问题。
	*					  2.增加基站定位功能，在net_device.h里通过宏M6311_LOCATION来确实是否使用。
	*					  3.NET_DEVICE_SendCmd新增参数“mode”，决定是否清除本次命令的返回值。
	************************************************************
	************************************************************
	************************************************************
**/

#include "stm32f10x.h"	//单片机头文件

#include "net_device.h"	//网络设备应用层
#include "net_io.h"		//网络设备数据IO层

//硬件驱动
#include "delay.h"
#include "led.h"
#include "usart.h"

//C库
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




NET_DEVICE_INFO netDeviceInfo = {0, 0, 7, 0, 0, 0};


GPS_INFO gps;


void M6311_Location(void)
{
	
	char *dataPtr, *ptr;
	
	char mcc[8]; 																				//移动国家号码
	char mnc[8];																				//移动网络号码
	char lac[16];																				//位置区域码
	char cell[16];																				//小区号
	
	char sendBuf[128];

	while(NET_DEVICE_SendCmd("AT+IPSTART=\"TCP\",\"183.230.40.83\",80\r\n", "CONNECT OK", 1))	//定位端ip
		RTOS_TimeDly(100);
	
	while(NET_DEVICE_SendCmd("AT+CCED=0,2\r\n", "OK", 0))										//查找到
		RTOS_TimeDly(100);
	
	dataPtr = strstr((char *)netIOInfo.buf, "+CCED:");
	
	if(dataPtr != NULL)
	{
		dataPtr += 7;
		memset(mcc, 0, sizeof(mcc));
		ptr = mcc;
		while(*dataPtr >= '0' && *dataPtr <= '9')
			*ptr++ = *dataPtr++;
		
		dataPtr++;
		memset(mnc, 0, sizeof(mnc));
		ptr = mnc;
		while(*dataPtr >= '0' && *dataPtr <= '9')
			*ptr++ = *dataPtr++;
		
		dataPtr++;
		memset(lac, 0, sizeof(lac));
		ptr = lac;
		while(*dataPtr >= '0' && *dataPtr <= '9')
			*ptr++ = *dataPtr++;
		
		dataPtr++;
		memset(cell, 0, sizeof(cell));
		ptr = cell;
		while(*dataPtr >= '0' && *dataPtr <= '9')
			*ptr++ = *dataPtr++;
		
		//UsartPrintf(USART_DEBUG, "mcc = %s, mcc = %s, lac = %s, cell = %s\r\n", mcc, mnc, lac, cell);
		
		memset(sendBuf, 0, sizeof(sendBuf));
		
		snprintf(sendBuf, sizeof(sendBuf), "GET http://api.lbs.heclouds.com/api/gsmlbs?mcc=%s&mnc=%s&cell=%s&lac=%s&apikey=B985C9CC0017EAD1EA11126F37CB1BA1\r\nHTTP/1.1\r\nHost:api.lbs.heclouds.com\r\n",
		
			mcc, mnc, cell, lac);
		
		NET_DEVICE_SendData((unsigned char *)sendBuf, strlen(sendBuf));
		
		dataPtr = (char *)NET_DEVICE_GetIPD(200);
		if(dataPtr != NULL)
		{
			dataPtr = strstr(dataPtr, "\"lng\" :");
			dataPtr += 8;
			ptr = gps.lon;
			while((*dataPtr >= '0' && *dataPtr <= '9') || *dataPtr == '.')
				*ptr++ = *dataPtr++;
			
			dataPtr = strstr(dataPtr, "\"lat\" :");
			dataPtr += 8;
			ptr = gps.lat;
			while((*dataPtr >= '0' && *dataPtr <= '9') || *dataPtr == '.')
				*ptr++ = *dataPtr++;
			
			UsartPrintf(USART_DEBUG, "lon = %s, lat = %s\r\n", gps.lon, gps.lat);
		}
	}
	else
		UsartPrintf(USART_DEBUG, "dataptr is null\r\n");
	
	NET_DEVICE_SendCmd("AT+IPCLOSE\r\n", "OK", 1);							//连接前先关闭一次
	UsartPrintf(USART_DEBUG, "Tips:	CIPCLOSE\r\n");
	RTOS_TimeDly(100);														//等待

}

//==========================================================
//	函数名称：	NET_DEVICE_IO_Init
//
//	函数功能：	初始化网络设备IO层
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		初始化网络设备的控制引脚、数据收发功能等
//==========================================================
void NET_DEVICE_IO_Init(void)
{
	
	GPIO_InitTypeDef gpioInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	//
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_1;			//GPIOA1-复位
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitStruct);
	
	gpioInitStruct.GPIO_Pin = GPIO_Pin_4;			//GPIOC4-PowerKey
	GPIO_Init(GPIOC, &gpioInitStruct);
	
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IPD;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_7;			//GPIOA7-status
	GPIO_Init(GPIOA, &gpioInitStruct);
	
	NET_DEVICE_PWRK_ON;
	
	NET_IO_Init();									//网络设备数据IO层初始化
	
	netDeviceInfo.reboot = 0;
	
	while(!NET_DEVICE_STATUS)
		RTOS_TimeDly(2);

}

#if(NET_DEVICE_TRANS == 1)
//==========================================================
//	函数名称：	M6311_QuitTrans
//
//	函数功能：	退出透传模式
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		连续发送三个‘+’，然后关闭透传模式
//==========================================================
void M6311_QuitTrans(void)
{

	while((NET_IO->SR & 0X40) == 0);	//等待发送空
	NET_IO->DR = '+';
	RTOS_TimeDly(3); 					//大于串口组帧时间(10ms)
	
	while((NET_IO->SR & 0X40) == 0);	//等待发送空
	NET_IO->DR = '+';        
	RTOS_TimeDly(3); 					//大于串口组帧时间(10ms)
	
	while((NET_IO->SR & 0X40) == 0);	//等待发送空
	NET_IO->DR = '+';        
	RTOS_TimeDly(20);					//等待100ms
	
	NET_DEVICE_SendCmd("AT+CMMODE=0\r\n","OK", 1);	//关闭透传模式

}

#endif

//==========================================================
//	函数名称：	NET_DEVICE_Exist
//
//	函数功能：	网络设备存在检查
//
//	入口参数：	无
//
//	返回参数：	返回结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Exist(void)
{

	return NET_DEVICE_SendCmd("AT\r\n", "OK", 1);

}

//==========================================================
//	函数名称：	NET_DEVICE_Init
//
//	函数功能：	网络设备初始化
//
//	入口参数：	无
//
//	返回参数：	返回初始化结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_Init(char *ip, char *port)
{
	
	char cfgBuffer[32];
	
	switch(netDeviceInfo.initStep)
	{
		case 0:
			
			NET_IO_Send((unsigned char *)"AT+CMVERSION\r\n", strlen("AT+CMVERSION\r\n"));
			RTOS_TimeDly(30);
			UsartPrintf(USART_DEBUG, "\r\n**************************\r\n%s\r\n**************************\r\n", netIOInfo.buf);
			NET_DEVICE_ClrData();
			netDeviceInfo.initStep++;
		
		break;
		
		case 1:
			Led4_Set(LED_ON);
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+SSYS?\r\n");
			if(!NET_DEVICE_SendCmd("AT+SSYS?\r\n","OK", 1)) 					//切换sim卡   0-内置卡		1-外置卡	这里使用外置卡
				netDeviceInfo.initStep++;
		break;
		
		case 2:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+SIM1\r\n");
			if(!NET_DEVICE_SendCmd("AT+SIM1\r\n","OK", 1)) 						//检测外置卡是否存在		返回+SIM1: EXSIT
				netDeviceInfo.initStep++;
		break;
		
		case 3:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CPIN?\r\n");
			if(!NET_DEVICE_SendCmd("AT+CPIN?\r\n", "+CPIN: READY", 1))			//确保SIM卡PIN码解锁，返回READY，表示解锁成功
				netDeviceInfo.initStep++;
		break;
		
		case 4: //自动判断卡类型
		{
			char resBuf[5] = {0, 0, 0, 0, 0};
			char text[2] = {0, 0};
			
			strcpy(resBuf, "0,");
			sprintf(text, "%d", netDeviceInfo.cardType);
			strcat(resBuf, text);
			
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CREG?  %d\r\n",
										netDeviceInfo.cardType);
			if(!NET_DEVICE_SendCmd("AT+CREG?\r\n", resBuf, 1)) 				//确认网络搜索成功,OK
				netDeviceInfo.initStep++;
			else 															//如果失败则检测返回的内容
			{
				if(netIOInfo.buf[11] != 48)
					netDeviceInfo.cardType = netIOInfo.buf[11] - 48;
				
				NET_DEVICE_ClrData();
			}
		}
		break;
			
		case 5:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CSQ\r\n");
			if(!NET_DEVICE_SendCmd("AT+CSQ\r\n","OK", 1))					//查询信号强度,OK
				netDeviceInfo.initStep++;
		break;
			
		case 6:
				UsartPrintf(USART_DEBUG, "STA Tips:	AT+CGREG?\r\n");		//检查网络注册状态
				if(!NET_DEVICE_SendCmd("AT+CGREG?\r\n","OK", 1))
					netDeviceInfo.initStep++;
		break;
		
		case 7:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CGACT=1,1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CGACT=1,1\r\n","OK", 1)) 			//激活
				netDeviceInfo.initStep++;
		break;
		
		case 8:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CGATT=1\r\n");			//附着GPRS业务
			if(!NET_DEVICE_SendCmd("AT+CGATT=1\r\n","OK", 1))
				netDeviceInfo.initStep++;
		break;
			
		case 9:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CMMUX=0\r\n");
			if(!NET_DEVICE_SendCmd("AT+CMMUX=0\r\n","OK", 1)) 				//必须为单连接，不然平台IP都连不上
				netDeviceInfo.initStep++;
		break;
			
#if(NET_DEVICE_TRANS == 1)
			
		case 10:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CMMODE=1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CMMODE=1\r\n","OK", 1))					//配置透传
				netDeviceInfo.initStep++;
		break;
		
		case 11:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CMTCFG=1,1024,1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CMTCFG=1,1024,1\r\n","OK", 1)) 		//配置透传，最大长度2000字节，间隔是100ms。配置成hex模式
				netDeviceInfo.initStep++;
		break;
			
		case 12:
			memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
			strcpy(cfgBuffer, "AT+IPSTART=\"TCP\",\"");
			strcat(cfgBuffer, ip);
			strcat(cfgBuffer, "\",");
			strcat(cfgBuffer, port);
			strcat(cfgBuffer, "\r\n");
			UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);
			
			UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);
			if(!NET_DEVICE_SendCmd(cfgBuffer,"CONNECT", 1)) 				//连接平台
			{
				Led4_Set(LED_OFF);
				netDeviceInfo.initStep++;
			}
		break;
			
#else
			
		case 10:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CMHEAD=1\r\n");
			if(!NET_DEVICE_SendCmd("AT+CMHEAD=1\r\n","OK", 1))				//显示IP头
				netDeviceInfo.initStep++;
		break;
		
		case 11:
#if(M6311_LOCATION == 1)
			M6311_Location();
#endif
			memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
			strcpy(cfgBuffer, "AT+IPSTART=\"TCP\",\"");
			strcat(cfgBuffer, ip);
			strcat(cfgBuffer, "\",");
			strcat(cfgBuffer, port);
			strcat(cfgBuffer, "\r\n");
			UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);
			
			if(!NET_DEVICE_SendCmd(cfgBuffer,"CONNECT", 1)) 				//连接平台
			{
				Led4_Set(LED_OFF);
				netDeviceInfo.initStep++;
			}
		break;
			
		case 12:
			UsartPrintf(USART_DEBUG, "STA Tips:	AT+CMSTATE\r\n");
			if(!NET_DEVICE_SendCmd("AT+CMSTATE\r\n", "CONNECTED", 1))		//检查连接状态
				netDeviceInfo.initStep++;
			else
			{
				NET_DEVICE_SendCmd("AT+IPCLOSE\r\n", "OK", 1);
				RTOS_TimeDly(20);
				NET_DEVICE_ReConfig(11);
			}
		break;
#endif
			
		default:
		break;
	}

	if(netDeviceInfo.initStep == 13)
		return 0;
	else
		return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_Reset
//
//	函数功能：	网络设备复位
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_Reset(void)
{
	
#if(NET_DEVICE_TRANS == 1)
	M6311_QuitTrans();
	UsartPrintf(USART_DEBUG, "Tips:	QuitTrans\r\n");
#endif
	
	UsartPrintf(USART_DEBUG, "Tips:	M6311_Reset\r\n");

	//复位模块	如果不复位的话，在死机重启时，6311是无法初始化成功的
	NET_DEVICE_RST_ON;		//复位
	RTOS_TimeDly(50);
	
	NET_DEVICE_RST_OFF;		//结束复位
	RTOS_TimeDly(200);

}

//==========================================================
//	函数名称：	NET_DEVICE_ReLink
//
//	函数功能：	重连平台
//
//	入口参数：	无
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_ReLink(char *ip, char *port)
{

#if(NET_DEVICE_TRANS == 1)
	return 1;
#else
	char cfgBuffer[32];
	
	NET_DEVICE_SendCmd("AT+IPCLOSE\r\n", "OK", 1);							//连接前先关闭一次
	UsartPrintf(USART_DEBUG, "Tips:	CIPCLOSE\r\n");
	RTOS_TimeDly(100);														//等待
	
	memset(cfgBuffer, 0, sizeof(cfgBuffer));
			
	strcpy(cfgBuffer, "AT+IPSTART=\"TCP\",\"");
	strcat(cfgBuffer, ip);
	strcat(cfgBuffer, "\",");
	strcat(cfgBuffer, port);
	strcat(cfgBuffer, "\r\n");
	UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);
	
	UsartPrintf(USART_DEBUG, "STA Tips:	%s", cfgBuffer);
	if(!NET_DEVICE_SendCmd(cfgBuffer,"CONNECT", 1))							//重连平台
		return 0;
	else
		return 1;
#endif

}

//==========================================================
//	函数名称：	NET_DEVICE_SendCmd
//
//	函数功能：	向网络设备发送一条命令，并等待正确的响应
//
//	入口参数：	cmd：需要发送的命令
//				res：需要检索的响应
//				mode：1-清除接收		0-不清除(能获取返回信息)
//
//	返回参数：	返回连接结果
//
//	说明：		0-成功		1-失败
//==========================================================
_Bool NET_DEVICE_SendCmd(char *cmd, char *res, _Bool mode) //
{

	unsigned char timeOut = 200;
	
	NET_IO_Send((unsigned char *)cmd, strlen((const char *)cmd));	//写命令到网络设备
	
	while(timeOut--)												//等待
	{
		if(NET_IO_WaitRecive() == REV_OK)							//如果收到数据
		{
			if(strstr((const char *)netIOInfo.buf, res) != NULL)	//如果检索到关键词
			{
				if(mode)
					NET_IO_ClearRecive();							//清空缓存
				
				return 0;
			}
		}
		
		RTOS_TimeDly(2);											//挂起等待
	}
	
	return 1;

}

//==========================================================
//	函数名称：	NET_DEVICE_SendData
//
//	函数功能：	使网络设备发送数据到平台
//
//	入口参数：	data：需要发送的数据
//				len：数据长度
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_SendData(unsigned char *data, unsigned short len)
{
	
#if(NET_DEVICE_TRANS == 1)
	NET_IO_Send(data, len);							//发送设备连接请求数据
#else
	char cmdBuf[30];
	
	RTOS_TimeDly(10);								//等待一下

	NET_IO_ClearRecive();							//清空接收缓存
	sprintf(cmdBuf, "AT+IPSEND=%d\r\n", len);		//发送命令
	if(!NET_DEVICE_SendCmd(cmdBuf, ">", 1)) 		//收到‘>’时可以发送数据
	{
		NET_IO_Send(data, len);  					//发送设备连接请求数据
	}
#endif

}

//==========================================================
//	函数名称：	NET_DEVICE_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	timeOut等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如M6311的返回格式为	"<IPDATA: x>\r\nyyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *NET_DEVICE_GetIPD(unsigned short timeOut)
{
	
#if(NET_DEVICE_TRANS == 0)
	unsigned char byte = 0, count = 0;
	char sByte[5];
	char *ptrIPD;
#endif
	
	do
	{
		if(NET_IO_WaitRecive() == REV_OK)								//如果接收完成
		{
#if(NET_DEVICE_TRANS == 0)
			ptrIPD = strstr((char *)netIOInfo.buf, "IPDATA:");			//搜索“IPDATA”头
			if(ptrIPD == NULL)											//如果没找到，可能是IPDATA头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				//UsartPrintf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strstr(ptrIPD, ":");ptrIPD++;					//找到':'  ，然后偏移到下一个字符，代表数据长度的第一个数字
				
				while(*ptrIPD != '\r')									//在':'和'\r' 之间的都是数据长度的数据
				{
					sByte[count++] = *ptrIPD++;
				}
				byte = (unsigned char)atoi(sByte);						//将字符转为数值形式
				
				ptrIPD += 2;											//此时ptrIPD指针还指向':'，所以需要偏移到后两个数据
				for(count = 0; count < byte; count++)					//复制数据
				{
					netIOInfo.buf[count] = *ptrIPD++;
				}
				memset(netIOInfo.buf + byte,
						0, sizeof(netIOInfo.buf) - byte);				//将后边的数据清空
				return netIOInfo.buf;									//复制完成，返回数据指针
			}
#else
			return netIOInfo.buf;
#endif
		}
		
		RTOS_TimeDly(2);												//延时等待
	} while(timeOut--);
	
	return NULL;														//超时还未找到，返回空指针

}

//==========================================================
//	函数名称：	NET_DEVICE_ClrData
//
//	函数功能：	清空网络设备数据接收缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_ClrData(void)
{

	NET_IO_ClearRecive();		//清空缓存

}

//==========================================================
//	函数名称：	NET_DEVICE_Check
//
//	函数功能：	检查网络设备连接状态
//
//	入口参数：	无
//
//	返回参数：	返回状态
//
//	说明：		
//==========================================================
unsigned char NET_DEVICE_Check(void)
{
	
#if(NET_DEVICE_TRANS == 1)
	return 3;
#else
	
	unsigned char status = 0;
	
	NET_IO_ClearRecive();												//清空缓存
	
	if(NET_DEVICE_SendCmd("AT+SIM1\r\n", "OK", 1))						//如果sim卡丢失
	{
		UsartPrintf(USART_DEBUG, "WARN:		No Sim Card\r\n");
		status = 5;
	}
	else
		UsartPrintf(USART_DEBUG, "Tips:		Sim Card\r\n");
	
	if(!NET_DEVICE_SendCmd("AT+CMSTATE\r\n", "CLOSED", 1))				//检查连接状态
	{
		UsartPrintf(USART_DEBUG, "WARN:		CLOSED\r\n");
		status = 1;
	}
	else if(!NET_DEVICE_SendCmd("AT+CMSTATE\r\n", "CONNECTED", 1))		//检查连接状态
	{
		UsartPrintf(USART_DEBUG, "WARN:		CONNECTED\r\n");
		status = 0;
	}
	
	return status;
	
#endif

}

//==========================================================
//	函数名称：	NET_DEVICE_ReConfig
//
//	函数功能：	设备网络设备初始化的步骤
//
//	入口参数：	步骤值
//
//	返回参数：	无
//
//	说明：		该函数设置的参数在网络设备初始化里边用到
//==========================================================
void NET_DEVICE_ReConfig(unsigned char step)
{

	netDeviceInfo.initStep = step;

}

//==========================================================
//	函数名称：	NET_DEVICE_Set_DataMode
//
//	函数功能：	设置设备数据收发模式
//
//	入口参数：	模式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void NET_DEVICE_Set_DataMode(unsigned char mode)
{

	netDeviceInfo.dataType = mode;

}

//==========================================================
//	函数名称：	NET_DEVICE_Get_DataMode
//
//	函数功能：	获取设备数据收发模式
//
//	入口参数：	无
//
//	返回参数：	模式
//
//	说明：		
//==========================================================
unsigned char NET_DEVICE_Get_DataMode(void)
{

	return netDeviceInfo.dataType;

}
