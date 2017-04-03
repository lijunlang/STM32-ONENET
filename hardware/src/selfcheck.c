/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	selfcheck.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		LED初始化，亮灭LED
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "selfcheck.h"
#include "i2c.h"
#include "usart.h"
#include "delay.h"
#include "oled.h"




CHECK_INFO checkInfo = {DEV_ERR, DEV_ERR, DEV_ERR, DEV_ERR, DEV_ERR};



/*
************************************************************
*	函数名称：	Check_PowerOn
*
*	函数功能：	外接设备检测
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		IIC设备可以读取寄存器来查看响应情况
*				主要检查sht20、adxl345、gy30、eeprom
************************************************************
*/
void Check_PowerOn(void)
{

	unsigned char value = 0;
	
	//检测SH20
	I2C_ReadByte(0X40, 0XE7, &value);					//读取用户寄存器
	if(value)
	{
		UsartPrintf(USART_DEBUG, " SHT20 :Ok\r\n");
		checkInfo.SHT20_OK = DEV_OK;
	}
	else
		UsartPrintf(USART_DEBUG, " SHT20 :Error\r\n");
	DelayXms(1);
	
	//检测ADXL345
	I2C_ReadByte(0x53, 0x00, &value);
	if(value == 229)
	{
		UsartPrintf(USART_DEBUG, "ADXL345 :Ok\r\n");
		checkInfo.ADXL345_OK = DEV_OK;
	}
	else
		UsartPrintf(USART_DEBUG, "ADXL345 :Error\r\n");
	DelayXms(1);
	
	//检测EEPROM
	if(!I2C_ReadByte(0x50, 255, &value))
	{
		UsartPrintf(USART_DEBUG, "EEPROM :Ok\r\n");
		checkInfo.EEPROM_OK = DEV_OK;
	}
	else
		UsartPrintf(USART_DEBUG, "EEPROM :Error\r\n");
	DelayXms(1);
	
	//检测OLED
	if(!OLED_WriteCom(0xAE))								//关闭显示，判断ACK
	{
		UsartPrintf(USART_DEBUG, "OLED :Ok\r\n");
		checkInfo.OLED_OK = DEV_OK;
	}
	else
		UsartPrintf(USART_DEBUG, "OLED :Error\r\n");

}
