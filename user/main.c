/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2017-01-011
	*
	*	版本： 		V1.0
	*
	*	说明： 		接入onenet，上传数据和命令控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络协议层
#include "onenet.h"
#include "fault.h"

//网络设备
#include "net_device.h"

//硬件驱动
#include "led.h"
#include "delay.h"
#include "key.h"
#include "lcd1602.h"
#include "usart.h"
#include "hwtimer.h"
#include "i2c.h"
#include "gy30.h"
#include "adxl345.h"
#include "sht20.h"
#include "iwdg.h"
#include "at24c02.h"
#include "selfcheck.h"
#include "beep.h"
#include "oled.h"
#include "info.h"
#include "tcrt5000.h"
#include "relay.h"

//C库
#include <string.h>


//数据流
DATA_STREAM dataStream[] = {
								{"Red_Led", &ledStatus.Led4Sta, TYPE_BOOL, 1},
								{"Green_Led", &ledStatus.Led5Sta, TYPE_BOOL, 1},
								{"Yellow_Led", &ledStatus.Led6Sta, TYPE_BOOL, 1},
								{"Blue_Led", &ledStatus.Led7Sta, TYPE_BOOL, 1},
								{"beep", &beepInfo.Beep_Status, TYPE_BOOL, 1},
								{"temperature", &sht20Info.tempreture, TYPE_FLOAT, 1},
								{"humidity", &sht20Info.humidity, TYPE_FLOAT, 1},
								{"Xg", &adxlInfo.incidence_Xf, TYPE_FLOAT, 1},
								{"Yg", &adxlInfo.incidence_Yf, TYPE_FLOAT, 1},
								{"Zg", &adxlInfo.incidence_Zf, TYPE_FLOAT, 1},
								{"errType", &faultTypeReport, TYPE_UCHAR, 1},
								{"Relay1", &relayStatus.J1Sta, TYPE_BOOL, 1},
								{"Relay2", &relayStatus.J2Sta, TYPE_BOOL, 1},
								
							};
unsigned char dataStreamLen = sizeof(dataStream) / sizeof(dataStream[0]);





/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);								//中断控制器分组设置

	Delay_Init();																//systick初始化
	
	Led_Init();																	//LED初始化
	
	Key_Init();																	//按键初始化
	
	//Beep_Init();																//蜂鸣器初始化
	
	TCRT5000_Init();															//TCRT5000初始化
	
	Relay_Init();																//继电器初始化
	
	IIC_Init();																	//软件IIC总线初始化
	
	//GY30_Init();																//GY-30初始化
	
	Lcd1602_Init();																//LCD1602初始化
	
	Usart1_Init(115200); 														//初始化串口   115200bps
	
	Lcd1602_DisString(0x80, "Check Power On");									//提示进行开机检测
	Check_PowerOn(); 															//上电自检
	Lcd1602_Clear(0x80);														//清第一行显示
	
	if(checkInfo.ADXL345_OK == DEV_OK) 											//如果检测到ADXL345则初始化
		ADXL345_Init();
	
	if(checkInfo.OLED_OK == DEV_OK)												//如果检测到OLED则初始化
	{
		OLED_Init();
		OLED_ClearScreen();														//清屏
	}

	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET) 								//如果是看门狗复位则提示
	{
		UsartPrintf(USART_DEBUG, "WARN:	IWDG Reboot\r\n");
		
		RCC_ClearFlag();														//清除看门狗复位标志位
		
		faultTypeReport = faultType = FAULT_REBOOT; 							//标记为重启错误
		
		netDeviceInfo.reboot = 1;
	}
	else
	{
		UsartPrintf(USART_DEBUG, "2.DEVID: %s,     APIKEY: %s\r\n"
								, oneNetInfo.devID, oneNetInfo.apiKey);
		
		netDeviceInfo.reboot = 0;
	}
	
	//Iwdg_Init(4, 1250); 														//64分频，每秒625次，重载1250次，2s
	
	Lcd1602_Clear(0xff);														//清屏
	
	Timer6_7_Init(TIM6, 49, 35999);												//72MHz，36000分频-500us，50重载值。则中断周期为500us * 50 = 25ms
	Timer6_7_Init(TIM7, 1999, 35999);											//72MHz，36000分频-500us，2000重载值。则中断周期为500us * 2000 = 1s
																				//定时检查网络状态标志位
	
	UsartPrintf(USART_DEBUG, "3.Hardware init OK\r\n");							//提示初始化完成

}

/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
int main(void)
{
	
	unsigned char *dataPtr;
	unsigned int runTime = 0;
	_Bool sendFlag = 0;

	Hardware_Init();									//硬件初始化
	
	//标题显示
	OLED_DisChar16x16(0, 0, san);						//显示“三”
	OLED_DisChar16x16(0, 16, zhou);						//显示“轴”
	OLED_DisString6x8(1, 32, ":");						//显示“：”
	
	OLED_DisChar16x16(2, 0, wen);						//显示“温”
	OLED_DisChar16x16(2, 16, shi);						//显示“湿”
	OLED_DisChar16x16(2, 32, du);						//显示“度”
	OLED_DisString6x8(3, 48, ":");						//显示“：”
	
	OLED_DisChar16x16(6, 0, zhuang);					//显示“状”
	OLED_DisChar16x16(6, 16, tai);						//显示“态”
	OLED_DisString6x8(7, 32, ":");						//显示“：”
	
	NET_DEVICE_IO_Init();								//网络设备IO初始化
	NET_DEVICE_Reset();									//网络设备复位
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);			//设置为命令收发模式(例如ESP8266要区分AT的返回还是平台下发数据的返回)
	
	while(1)
	{
		
		if(oneNetInfo.netWork == 1)
		{
/******************************************************************************
			按键扫描
******************************************************************************/
			switch(Keyboard())
			{
				case KEY0DOWN:
					
					if(ledStatus.Led4Sta == LED_OFF | relayStatus.J1Sta ==RELAY_OFF)
					{
						Led4_Set(LED_ON);
						J1_Set(RELAY_ON);
					}
					else
					{
						Led4_Set(LED_OFF);
						J1_Set(RELAY_OFF);
					}
					oneNetInfo.sendData = 1;										//标记数据发送
					
				break;
				
				case KEY2DOWN:
					
					if(ledStatus.Led5Sta == LED_OFF)
						Led5_Set(LED_ON);
					else
						Led5_Set(LED_OFF);
					
					oneNetInfo.sendData = 1;
					
				break;
				
				case KEY3DOWN:
					
					if(ledStatus.Led6Sta == LED_OFF)
						Led6_Set(LED_ON);
					else
						Led6_Set(LED_OFF);
					
					oneNetInfo.sendData = 1;
					
				break;
				
				case KEY1DOWN:
					
					if(ledStatus.Led7Sta == LED_OFF | relayStatus.J2Sta ==RELAY_OFF)
					{
						Led7_Set(LED_ON);
						J2_Set(RELAY_ON);
					}
					else if(ledStatus.Led7Sta == LED_ON| relayStatus.J2Sta ==RELAY_ON)
					{
						Led7_Set(LED_OFF);
						J2_Set(RELAY_OFF);
					}

					
					oneNetInfo.sendData = 1;
					
				break;
				
				default:
				break;
			}
			
/******************************************************************************
			数据与心跳
******************************************************************************/
			if(timInfo.timer6Out - runTime >= 1000)									//25s一次(25ms中断)
			{
				runTime = timInfo.timer6Out;
				
				if(sendFlag)
				{
					TIM_Cmd(OS_TIMER, DISABLE);
					OneNet_HeartBeat();												//心跳连接
					TIM_Cmd(OS_TIMER, ENABLE);
				}
				else
				{
					TIM_Cmd(OS_TIMER, DISABLE);
					OneNet_SendData(kTypeSimpleJsonWithoutTime, dataStreamLen);		//数据发送
					TIM_Cmd(OS_TIMER, ENABLE);
				}
		
				sendFlag = !sendFlag;
			}
			
/******************************************************************************
			平台下发命令解析
******************************************************************************/
			if(oneNetInfo.netWork && NET_DEVICE_Get_DataMode() == DEVICE_DATA_MODE)	//当有网络 且 在命令接收模式时
			{
				dataPtr = NET_DEVICE_GetIPD(0);										//不等待，获取平台下发的数据
				if(dataPtr != NULL)													//如果数据指针不为空，则代表收到了数据
				{
					OneNet_App(dataPtr);											//集中处理
				}
			}
			
/******************************************************************************
			传感器
******************************************************************************/
			if(checkInfo.ADXL345_OK == DEV_OK) 										//只有设备存在时，才会读取值和显示
			{
				ADXL345_GetValue();													//采集传感器数据
				 
					Lcd1602_DisString(0x80, "X%0.1f,Y%0.1f,Z%0.1f", adxlInfo.incidence_Xf, adxlInfo.incidence_Yf, adxlInfo.incidence_Zf);
					OLED_DisString6x8(1, 40, "X%0.1f,Y%0.1f,Z%0.1f", adxlInfo.incidence_Xf, adxlInfo.incidence_Yf, adxlInfo.incidence_Zf);
				
			}
			
			if(checkInfo.SHT20_OK == DEV_OK) 										//只有设备存在时，才会读取值和显示
			{
				SHT20_GetValue();													//采集传感器数据
				Lcd1602_DisString(0xC0, "%0.1fC,%0.1f%%", sht20Info.tempreture, sht20Info.humidity);
				OLED_DisString6x8(3, 56, "%0.1fC,%0.1f%%", sht20Info.tempreture, sht20Info.humidity);
			}
			
			
			if(t5000Info.status == TCRT5000_ON)
			{
				TCRT5000_GetValue(5);
				if(t5000Info.voltag < 3500)
				{
					//Beep_Set(BEEP_ON);
					Led4_Set(LED_ON);
					DelayXms(200);
					Led5_Set(LED_ON);
					DelayXms(200);
					Led6_Set(LED_ON);
					DelayXms(200);
					Led7_Set(LED_ON);
					DelayXms(200);
					Led4_Set(LED_OFF);
					DelayXms(200);
					Led5_Set(LED_OFF);
					DelayXms(200);
					Led6_Set(LED_OFF);
					DelayXms(200);
					Led7_Set(LED_OFF);
					DelayXms(200);
				}
				else
				{
					//Beep_Set(BEEP_OFF);
				}
			}
			
/******************************************************************************
			错误处理
******************************************************************************/
			if(faultType != FAULT_NONE)												//如果错误标志被设置
			{
				UsartPrintf(USART_DEBUG, "WARN:	Fault Process\r\n");
				Fault_Process();													//进入错误处理函数
			}
			
/******************************************************************************
			数据反馈
******************************************************************************/
			if(oneNetInfo.sendData)
			{
				oneNetInfo.sendData = OneNet_SendData(kTypeSimpleJsonWithoutTime, dataStreamLen);
			}
		}
		else
		{
/******************************************************************************
			初始化网络设备、接入平台
******************************************************************************/
			if(!oneNetInfo.netWork && (checkInfo.NET_DEVICE_OK == DEV_OK))			//当没有网络 且 网络模块检测到时
			{
				OLED_DisChar16x16(6, 48, lian);
				OLED_DisChar16x16(6, 64, jie);
				OLED_DisChar16x16(6, 80, zhong);
				NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);							//设置为命令收发模式
				
				if(!NET_DEVICE_Init(oneNetInfo.ip, oneNetInfo.port))				//初始化网络设备，能连入网络
				{
					OneNet_DevLink(oneNetInfo.devID, oneNetInfo.apiKey);			//接入平台
					
					if(oneNetInfo.netWork)
					{
						//Beep_Set(BEEP_ON);											
						Led4_Set(LED_ON);															//全部灯闪烁2次提示接入成功
						Led5_Set(LED_ON);
						Led6_Set(LED_ON);
						Led7_Set(LED_ON);
						DelayXms(200);
						//Beep_Set(BEEP_OFF);
						Led4_Set(LED_OFF);
						Led5_Set(LED_OFF);
						Led6_Set(LED_OFF);
						Led7_Set(LED_OFF);
						DelayXms(200);
						Led4_Set(LED_ON);
						Led5_Set(LED_ON);
						Led6_Set(LED_ON);
						Led7_Set(LED_ON);
						DelayXms(200);
						Led4_Set(LED_OFF);
						Led5_Set(LED_OFF);
						Led6_Set(LED_OFF);
						Led7_Set(LED_OFF);
								
						OLED_DisChar16x16(6, 48, yi);
						OLED_DisChar16x16(6, 64, lian);
						OLED_DisChar16x16(6, 80, jie);
						
						runTime = timInfo.timer6Out;								//更新时间
					}
					else
					{
						Beep_Set(BEEP_ON);											//长叫提示失败
						DelayXms(500);
						Beep_Set(BEEP_OFF);
						
						OLED_DisChar16x16(6, 48, wei);
						OLED_DisChar16x16(6, 64, lian);
						OLED_DisChar16x16(6, 80, jie);
					}
				}
			}
			
/******************************************************************************
			网络设备检测
******************************************************************************/
			if(checkInfo.NET_DEVICE_OK == DEV_ERR) 									//当网络设备未做检测
			{
				NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);							//设置为命令收发模式
				
				if(timerCount >= NET_TIME) 											//如果网络连接超时
				{
					UsartPrintf(USART_DEBUG, "Tips:		Timer Check Err\r\n");
					
					NET_DEVICE_Reset();												//复位网络设备
					timerCount = 0;													//清零连接超时计数
					faultType = FAULT_NONE;											//清除错误标志
				}
				
				if(!NET_DEVICE_Exist())												//网络设备检测
				{
					UsartPrintf(USART_DEBUG, "NET Device :Ok\r\n");
					checkInfo.NET_DEVICE_OK = DEV_OK;								//检测到网络设备，标记
					NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);						//设置为数据收发模式
				}
				else
					UsartPrintf(USART_DEBUG, "NET Device :Error\r\n");
			}
		}
	}

}
