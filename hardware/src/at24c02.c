/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	AT24C02.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		保存、读取设备信息
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "at24c02.h"
#include "i2c.h"
#include "delay.h"




/*
************************************************************
*	函数名称：	AT24C02_WriteByte
*
*	函数功能：	写一个字节到EEPROM
*
*	入口参数：	regAddr：寄存器地址
*				byte：需要写入的数据
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void AT24C02_WriteByte(unsigned char regAddr, unsigned char byte)
{

	I2C_WriteByte(AT24C02_ADDRESS, regAddr, &byte);

}

/*
************************************************************
*	函数名称：	AT24C02_WriteBytes
*
*	函数功能：	写多个字节到EEPROM
*
*	入口参数：	regAddr：寄存器地址
*				byte：需要写入的数据缓存区
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void AT24C02_WriteBytes(unsigned char regAddr, unsigned char *byte, unsigned char len)
{
	
	unsigned char count = 0;
	
	for(; count < len; count++)
	{
		I2C_WriteByte(AT24C02_ADDRESS, regAddr, byte);
		
		regAddr++;											//地址自增
		byte++;												//偏移到下个数据
		
		DelayXms(5);										//必要延时，数据量一旦大了，比如十几字节时，1ms的延时有时无法保证全部字节正确写入
	}

}

/*
************************************************************
*	函数名称：	AT24C02_ReadByte
*
*	函数功能：	从EEPROM读一个字节
*
*	入口参数：	regAddr：寄存器地址
*				byte：需要读取的数据的缓存地址
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void AT24C02_ReadByte(unsigned char regAddr, unsigned char *byte)
{

	I2C_ReadByte(AT24C02_ADDRESS, regAddr, byte);

}

/*
************************************************************
*	函数名称：	AT24C02_ReadBytes
*
*	函数功能：	从EEPROM读多个字节
*
*	入口参数：	regAddr：寄存器地址
*				byte：需要写入的数据缓存区
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void AT24C02_ReadBytes(unsigned char regAddr, unsigned char *byte, unsigned char len)
{

	I2C_ReadBytes(AT24C02_ADDRESS, regAddr, byte, len);

}

/*
************************************************************
*	函数名称：	AT24C02_Clear
*
*	函数功能：	指定地址开始写入多个相同数据
*
*	入口参数：	startAddr：寄存器起始地址
*				byte：需要写入的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：		写入指定位置指定长度的指定数据
************************************************************
*/
_Bool AT24C02_Clear(unsigned char startAddr, unsigned char byte, unsigned short len)
{
	
	unsigned short count = 0;

	if(startAddr + len > 256)									//超出限制
		return 1;
	
	for(; count < len; count++)
	{
		I2C_WriteByte(AT24C02_ADDRESS, startAddr, &byte);
		
		startAddr++;											//地址自增
		
		DelayXms(2);											//必要延时，数据量一旦大了，比如十几字节时，1ms的延时有时无法保证全部字节正确写入
	}
	
	return 0;

}
