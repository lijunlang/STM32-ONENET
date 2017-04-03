/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	lcd1602.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.1
	*
	*	说明： 		LCD1602初始化，显示
	*
	*	修改记录：	V1.1：更改了EN脚接口
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "lcd1602.h"
#include "delay.h"

//C库
#include <stdarg.h>





//数据、命令控制
#define RS_H	GPIO_SetBits(GPIOC, GPIO_Pin_6)
#define RS_L	GPIO_ResetBits(GPIOC, GPIO_Pin_6)

//读写控制
#define RW_H	GPIO_SetBits(GPIOA, GPIO_Pin_11)
#define RW_L	GPIO_ResetBits(GPIOA, GPIO_Pin_11)

//使能控制
#define EN_H	GPIO_SetBits(GPIOB, GPIO_Pin_4)
#define EN_L	GPIO_ResetBits(GPIOB, GPIO_Pin_4)







/*
************************************************************
*	函数名称：	Lcd1602_SendByte
*
*	函数功能：	向LCD1602写一个字节
*
*	入口参数：	byte：需要写入的数据
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Lcd1602_SendByte(unsigned char byte)
{
	
	unsigned short value = 0;
	
	value = GPIO_ReadOutputData(GPIOB);					//读取GPIOB的数据
	value &= ~(0x001F << 5);							//清除bit5~8
	value |= ((unsigned short)byte & 0x001F) << 5;		//将要写入的数据取低5位并左移5位
	GPIO_Write(GPIOB, value);							//写入GPIOB
	
	value = GPIO_ReadOutputData(GPIOC);					//读取GPIOC的数据
	value &= ~(0x0007 << 0);							//清除bit0~2
	value |= ((unsigned short)byte & 0x00E0) >> 5;		//将要写入的数据取高3位并右移5位
	GPIO_Write(GPIOC, value);							//写入GPIOC
	
	DelayUs(10);

}

/*
************************************************************
*	函数名称：	Lcd1602_WriteCom
*
*	函数功能：	向LCD1602写命令
*
*	入口参数：	byte：需要写入的命令
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Lcd1602_WriteCom(unsigned char byte)
{

	RS_L;						//RS拉低，命令模式
	RW_L;						//RW拉低，写模式
	
	Lcd1602_SendByte(byte);		//发送一个字节
	
	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	函数名称：	Lcd1602_WriteCom_Busy
*
*	函数功能：	向LCD1602写命令
*
*	入口参数：	byte：需要写入的命令
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Lcd1602_WriteCom_Busy(unsigned char byte)
{
	
	DelayXms(10);

	RS_L;
	RW_L;
	
	Lcd1602_SendByte(byte);
	
	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	函数名称：	Lcd1602_WriteData
*
*	函数功能：	向LCD1602写一个数据
*
*	入口参数：	byte：需要写入的数据
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Lcd1602_WriteData(unsigned char byte)
{

	RS_H;						//RS拉高，数据模式
	RW_L;						//RW拉低，写模式
	
	Lcd1602_SendByte(byte);		//发送一个字节

	EN_H;
	DelayUs(20);
	EN_L;
	DelayUs(5);

}

/*
************************************************************
*	函数名称：	Lcd1602_Init
*
*	函数功能：	LCD1602初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		RW-PA11		RS-PC6		EN-PC3
*				DATA0~4-PB5~9		DATA5~7-PC0~2
************************************************************
*/
void Lcd1602_Init(void)
{

	GPIO_InitTypeDef gpioInitStrcut;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);				//禁止JTAG功能
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioInitStrcut);
	
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_6;
	GPIO_Init(GPIOC, &gpioInitStrcut);
	
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOA, &gpioInitStrcut);
	
	DelayXms(15);
    Lcd1602_WriteCom(0x38);
    DelayXms(5);
    Lcd1602_WriteCom(0x38);
    DelayXms(5);
    Lcd1602_WriteCom(0x38);
    Lcd1602_WriteCom_Busy(0x38);
    Lcd1602_WriteCom_Busy(0x08);
    Lcd1602_WriteCom_Busy(0x01);
    Lcd1602_WriteCom_Busy(0x06);
    Lcd1602_WriteCom_Busy(0x0c);
	
    EN_L;

}

/*
************************************************************
*	函数名称：	Lcd1602_Clear
*
*	函数功能：	LCD1602清除指定行
*
*	入口参数：	pos：指定的行
*
*	返回参数：	无
*
*	说明：		0x80-第一行		0xC0-第二行		0xFF-两行
************************************************************
*/
void Lcd1602_Clear(unsigned char pos)
{

	switch(pos)
	{
		case 0x80:
			
			Lcd1602_DisString(0x80, "                ");
		
		break;
		
		case 0xC0:
			
			Lcd1602_DisString(0xC0, "                ");
		
		break;
		
		case 0xFF:
			
			Lcd1602_WriteCom_Busy(0x01);
		
		break;
	}

}

/*
************************************************************
*	函数名称：	Lcd1602_DisString
*
*	函数功能：	设置LCD1602显示的内容
*
*	入口参数：	pos：要显示的行
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void Lcd1602_DisString(unsigned char pos, char *fmt,...)
{

	unsigned char LcdPrintfBuf[33];
	unsigned char count = 0;
	unsigned char remain = 0;					//一行剩余空间
	va_list ap;
	unsigned char *pStr = LcdPrintfBuf;
	
	va_start(ap,fmt);
	vsprintf((char *)LcdPrintfBuf, fmt, ap);
	va_end(ap);
	
	remain = 0x8f - pos;						//这里解决bug，以前只管写16次，这次要根据开始的位置来计算
	
	Lcd1602_WriteCom_Busy(pos);
	
	while(*pStr != 0)
	{
		Lcd1602_WriteData(*pStr++);
		
		if(++count > remain && pos <= 0x8f)
		{
			count = 0;
			Lcd1602_WriteCom_Busy(0xC0);
			DelayXms(1);
		}
	}

}
