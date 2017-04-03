/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	oled.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		OLED初始化，显示控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//硬件驱动
#include "oled.h"
#include "i2c.h"
#include "delay.h"
#include "selfcheck.h"
#include "hwtimer.h"

//字库
#include "oled_zk.h"

//C库
#include <stdarg.h>







/*
************************************************************
*	函数名称：	OLED_SendByte
*
*	函数功能：	OLED发送一个字节
*
*	入口参数：	byte：需要发送的字节
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_SendByte(unsigned char byte)
{

	unsigned char i = 0;
	
	for(; i < 8; i++)		
	{
		if(byte & 0x80)
			SDA_H;
		else
			SDA_L;
		
		DelayUs(iicInfo.speed);
		
		SCL_H;
		DelayUs(iicInfo.speed);
		SCL_L;
		
		byte <<= 1;
	}

}

/*
************************************************************
*	函数名称：	OLED_WriteData
*
*	函数功能：	OLED写入一个数据
*
*	入口参数：	byte：需要写入的数据
*
*	返回参数：	写入结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
_Bool OLED_WriteData(unsigned char byte)
{
	
	IIC_Start();
	
	OLED_SendByte(OLED_ADDRESS);
	if(IIC_WaitAck(5000))	//等待应答
	{
		IIC_Stop();
		return 1;
	}
	
	OLED_SendByte(0x40);	//write data
	if(IIC_WaitAck(5000))	//等待应答
	{
		IIC_Stop();
		return 1;
	}
	
	OLED_SendByte(byte);
	if(IIC_WaitAck(5000))	//等待应答
	{
		IIC_Stop();
		return 1;
	}
	
	IIC_Stop();
	
	return 0;

}

/*
************************************************************
*	函数名称：	OLED_WriteCom
*
*	函数功能：	OLED写入一个命令
*
*	入口参数：	cmd：需要写入的命令
*
*	返回参数：	写入结果
*
*	说明：		0-成功		1-失败
************************************************************
*/
_Bool OLED_WriteCom(unsigned char cmd)
{
	
	IIC_Start();
	
	OLED_SendByte(OLED_ADDRESS);	//设备地址
	if(IIC_WaitAck(5000))			//等待应答
	{
		IIC_Stop();
		return 1;
	}
	
	OLED_SendByte(0x00);
	if(IIC_WaitAck(5000))			//等待应答
	{
		IIC_Stop();
		return 1;
	}
	
	OLED_SendByte(cmd);
	if(IIC_WaitAck(5000))			//等待应答
	{
		IIC_Stop();
		return 1;
	}
	
	IIC_Stop();
	
	return 0;
	
}

/*
************************************************************
*	函数名称：	OLED_Init
*
*	函数功能：	OLED初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_Init(void)
{
#if 1
	OLED_WriteCom(0xAE); //关闭显示
	OLED_WriteCom(0x20); //Set Memory Addressing Mode	
	OLED_WriteCom(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_WriteCom(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	OLED_WriteCom(0xa1); //0xa0，X轴正常显示；0xa1，X轴镜像显示
	OLED_WriteCom(0xc8); //0xc0，Y轴正常显示；0xc8，Y轴镜像显示
	OLED_WriteCom(0x00); //设置列地址低4位
	OLED_WriteCom(0x10); //设置列地址高4位
	OLED_WriteCom(0x40); //设置起始线地址
	OLED_WriteCom(0x81); //设置对比度值
	OLED_WriteCom(0x7f); //------
	OLED_WriteCom(0xa6); //0xa6,正常显示模式;0xa7，
	OLED_WriteCom(0xa8); //--set multiplex ratio(1 to 64)
	OLED_WriteCom(0x3F); //------
	OLED_WriteCom(0xa4); //0xa4,显示跟随RAM的改变而改变;0xa5,显示内容忽略RAM的内容
	OLED_WriteCom(0xd3); //设置显示偏移
	OLED_WriteCom(0x00); //------
	OLED_WriteCom(0xd5); //设置内部显示时钟频率
	OLED_WriteCom(0xf0); //------
	OLED_WriteCom(0xd9); //--set pre-charge period//
	OLED_WriteCom(0x22); //------
	OLED_WriteCom(0xda); //--set com pins hardware configuration//
	OLED_WriteCom(0x12); //------
	OLED_WriteCom(0xdb); //--set vcomh//
	OLED_WriteCom(0x20); //------
	OLED_WriteCom(0x8d); //--set DC-DC enable//
	OLED_WriteCom(0x14); //------
	OLED_WriteCom(0xaf); //打开显示
#else
	OLED_WriteCom(0xAE);   //display off
	OLED_WriteCom(0x00);	//Set Memory Addressing Mode	
	OLED_WriteCom(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_WriteCom(0x40);	//Set Page Start Address for Page Addressing Mode,0-7
	OLED_WriteCom(0xb0);	//Set COM Output Scan Direction
	OLED_WriteCom(0x81);//---set low column address
	OLED_WriteCom(0xff);//---set high column address
	OLED_WriteCom(0xa1);//--set start line address
	OLED_WriteCom(0xa6);//--set contrast control register
	OLED_WriteCom(0xa8);
	OLED_WriteCom(0x3f);//--set segment re-map 0 to 127
	OLED_WriteCom(0xad);//--set normal display
	OLED_WriteCom(0x8b);//--set multiplex ratio(1 to 64)
	OLED_WriteCom(0x33);//
	OLED_WriteCom(0xc8);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	OLED_WriteCom(0xd3);//-set display offset
	OLED_WriteCom(0x00);//-not offset
	OLED_WriteCom(0xd5);//--set display clock divide ratio/oscillator frequency
	OLED_WriteCom(0x80);//--set divide ratio
	OLED_WriteCom(0xd9);//--set pre-charge period
	OLED_WriteCom(0x1f); //
	OLED_WriteCom(0xda);//--set com pins hardware configuration
	OLED_WriteCom(0x12);
	OLED_WriteCom(0xdb);//--set vcomh
	OLED_WriteCom(0x40);//0x20,0.77xVcc
//	IIC_Write_Command(0x8d);//--set DC-DC enable
//	IIC_Write_Command(0x14);//
	OLED_WriteCom(0xaf);//--turn on oled panel
#endif

}

/*
************************************************************
*	函数名称：	OLED_Address
*
*	函数功能：	设置OLED显示地址
*
*	入口参数：	x：行地址
*				y：列地址
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_Address(unsigned char x, unsigned char y)
{

	OLED_WriteCom(0xb0 + x);					//设置行地址
	DelayUs(iicInfo.speed);
	OLED_WriteCom(((y & 0xf0) >> 4) | 0x10);	//设置列地址的高4位
	DelayUs(iicInfo.speed);
	OLED_WriteCom(y & 0x0f);					//设置列地址的低4位
	DelayUs(iicInfo.speed);
	
}

/*
************************************************************
*	函数名称：	OLED_ClearScreen
*
*	函数功能：	OLED全屏清除
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_ClearScreen(void)
{
	
	unsigned char i = 0, j = 0;
	
	if(checkInfo.OLED_OK)
	{
		TIM_Cmd(OS_TIMER, DISABLE);
		
		IIC_SpeedCtl(1);
		
		for(; i < 8; i++)
		{
			OLED_WriteCom(0xb0 + i);
			OLED_WriteCom(0x10);
			OLED_WriteCom(0x00);
			
			for(j = 0; j < 132; j++)
			{
				OLED_WriteData(0x00);
			}
		}
		
		TIM_Cmd(OS_TIMER, ENABLE);
	}
	
}

/*
************************************************************
*	函数名称：	OLED_ClearAt
*
*	函数功能：	OLED清除指定行
*
*	入口参数：	x：需要清除的行
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_ClearAt(unsigned char x)
{

	unsigned char i = 0;
	
	if(checkInfo.OLED_OK)
	{
		
		OLED_WriteCom(0xb0 + x);
		OLED_WriteCom(0x10);
		OLED_WriteCom(0x00);
			
		for(; i < 132; i++)
		{
			OLED_WriteData(0x00);
		}
	}

}

/*
************************************************************
*	函数名称：	OLED_Dis12864_Pic
*
*	函数功能：	显示一幅128*64的图片
*
*	入口参数：	dp：图片数据指针
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void OLED_Dis12864_Pic(const unsigned char *dp)
{
	
	unsigned char i = 0, j = 0;
	
	if(checkInfo.OLED_OK)
	{
		
		for(; j < 8; j++)
		{
			OLED_Address(j, 0);
			
			for (i = 0; i < 128; i++)
			{	
				OLED_WriteData(*dp++); //写数据到LCD,每写完一个8位的数据后列地址自动加1
			}
		}
	}
	
}

/*
************************************************************
*	函数名称：	OLED_DisChar16x16
*
*	函数功能：	显示16x16的点阵数据
*
*	入口参数：	dp：图片数据指针
*
*	返回参数：	无
*
*	说明：		显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标
************************************************************
*/
void OLED_DisChar16x16(unsigned short x, unsigned short y, const unsigned char *dp)
{
	
	unsigned short i = 0, j = 0;
	
	if(checkInfo.OLED_OK)
	{
		IIC_SpeedCtl(20);					//i2c速度控制
		
		for(j = 2; j > 0; j--)
		{
			OLED_Address(x, y);
			
			for (i = 0; i < 16; i++)
			{
				OLED_WriteData(*dp++);		//写数据到OLED,每写完一个8位的数据后列地址自动加1
			}
			
			x++;
		}
	}
	
}

/*
************************************************************
*	函数名称：	OLED_DisString6x8
*
*	函数功能：	显示6x8的点阵数据
*
*	入口参数：	x：显示行
*				y：显示列
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		能显示7行
************************************************************
*/
void OLED_DisString6x8(unsigned char x, unsigned char y, char *fmt, ...)
{

	unsigned char i = 0, ch = 0;
	unsigned char OledPrintfBuf[300];
	
	va_list ap;
	unsigned char *pStr = OledPrintfBuf;
	
	va_start(ap,fmt);
	vsprintf((char *)OledPrintfBuf, fmt, ap);
	va_end(ap);
	
	if(checkInfo.OLED_OK)
	{
		y += 2;
		IIC_SpeedCtl(20);							//i2c速度控制
		
		while(*pStr != '\0')
		{
			ch = *pStr - 32;
			
			if(y > 126)
			{
				y = 2;
				x++;
			}
			
			OLED_Address(x, y);
			for(i = 0; i < 6; i++)
				OLED_WriteData(F6x8[ch][i]);
			
			y += 6;
			pStr++;
		}
	}

}

/*
************************************************************
*	函数名称：	OLED_DisString8x16
*
*	函数功能：	显示8x16的点阵数据
*
*	入口参数：	x：显示行
*				y：显示列
*				fmt：不定长参
*
*	返回参数：	无
*
*	说明：		能显示4行
************************************************************
*/
void OLED_DisString8x16(unsigned char x, unsigned char y, char *fmt, ...)
{

	unsigned char i = 0, ch = 0;
	unsigned char OledPrintfBuf[300];
	
	va_list ap;
	unsigned char *pStr = OledPrintfBuf;
	
	va_start(ap,fmt);
	vsprintf((char *)OledPrintfBuf, fmt, ap);
	va_end(ap);
	
	if(checkInfo.OLED_OK)
	{
		y += 2;
		IIC_SpeedCtl(20);							//i2c速度控制
		
		while(*pStr != '\0')
		{
			ch = *pStr - 32;
			
			if(y > 128)
			{
				y = 2;
				x += 2;
			}
			
			OLED_Address(x, y);
			for(i = 0; i < 8; i++)
				OLED_WriteData(F8X16[(ch << 4) + i]);
			
			OLED_Address(x + 1, y);
			for(i = 0; i < 8; i++)
				OLED_WriteData(F8X16[(ch << 4) + i + 8]);
			
			y += 8;
			pStr++;
		}
	}

}
