/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	gy30.c
	*
	*	作者： 		MDZZ
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		光照传感器初始化，读取光照强度
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "gy30.h"
#include "i2c.h"
#include "delay.h"
#include "led.h"



GY30_INFO gy30Info;


/*
************************************************************
*	函数名称：	GY30_Init
*
*	函数功能：	GY30初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void GY30_Init(void)
{

	DelayUs(5);
	
	I2C_WriteByte(BH1750FVI_ADDR, BH1750_ON, (void *)0);			//power on
    DelayUs(2);
	
	I2C_WriteByte(BH1750FVI_ADDR, BH1750_RSET, (void *)0);			//clear
    DelayUs(40);
	
	I2C_WriteByte(BH1750FVI_ADDR, BH1750_Con_High_RM, (void *)0);	//连续H分辨率模式，至少120ms，之后自动断电模式
    DelayUs(40);

}

/*
************************************************************
*	函数名称：	GY30_GetValue
*
*	函数功能：	读取光照强度
*
*	入口参数：	无
*
*	返回参数：	读取结果
*
*	说明：		0-成功		1-失败
				单位LX，勒克斯
************************************************************
*/
_Bool GY30_GetValue(void)
{
	
    unsigned char addr;
    unsigned char data[2];
    unsigned short result = 0;
    float result_lx = 0;

	
	addr = BH1750FVI_ADDR << 1;								//地址左移
	
    IIC_Start();
	
    IIC_SendByte(addr);
	if(IIC_WaitAck(50))										//等待应答
		return 1;
	
	IIC_Start();

    IIC_SendByte(addr + 1);
	if(IIC_WaitAck(50))										//等待应答
		return 1;
	
	data[0] = IIC_RecvByte();
	IIC_Ack();
	data[1] = IIC_RecvByte();
	IIC_NAck();
	
	IIC_Stop();
	
    result = (unsigned short)((data[0] << 8) + data[1]);	//合成数据，即光照数据
    result_lx = (float)result / 1.2;						//除以1.2为光照强度

    gy30Info.lightVal = (unsigned short)result_lx;
	
	return 0;
	
}
