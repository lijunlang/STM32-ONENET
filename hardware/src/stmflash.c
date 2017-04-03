#include "stmflash.h"

#include "stm32f10x.h"









//读很简单，吧地址作为指针去读取里边的内容即可，实际上与不涉及到flash的保护的操作，仅仅是取地址里边的值而已
void Flash_Read(unsigned int addr, char *rBuf, unsigned short len)
{

	unsigned short lenCount = 0;
	unsigned char charBuf = 0;
	
	for(; lenCount < len; lenCount++)
	{
		charBuf = *(volatile unsigned char *)addr; //转为uchar类型
		if(charBuf == 0xff) //如果读到了0xFF，则代表数据读完了，因为各个人的ssid和password长度不同
			break;
		
		rBuf[lenCount] = (char)charBuf; //转换一下，赋值
		
		addr += 2; //传入的地址必须是2的倍数
	}

}

//判断是否需要擦除
_Bool Flash_NeedErase(void) //1-需要擦除(有数据)		0-不需要擦除(无数据)
{

	unsigned short rCount = 0;
	
	unsigned int addr = SSID_ADDR; //页起始地址
	
	for(; rCount < 1024; rCount++) //读出2KB数据		读一次至少是半字，所以这里是1024
	{
		if(*(volatile unsigned short *)addr != 0xffff)
			return 1;
		
		addr += 2;
	}
	
	return 0;

}

//写比较复杂点，注意三点，1.解锁。2.只有地址里边的值为0xFFFF时能够被写入其他数据，所以重写ssid和pswd前需要擦除一次。3.写完上锁。
void Flash_Write(unsigned int addr, char *wBuf, unsigned short len)
{
	
	unsigned short lenCount = 0;
	
	FLASH_Unlock();	//解锁

	//之前在这里sb了，我是分两次写入ssid和pswd的，结果没注释这里，ssid被擦除了，调试了大半天。。。我去
//	if(Flash_NeedErase()) //需要擦除
//	{
//		FLASH_ErasePage(SSID_ADDRESS);
//	}
	
	for(; lenCount < len; lenCount++)
	{
		//FLASH_ProgramOptionByteData(addr, wBuf[lenCount]); //写入 //这种方式写入我没调出来，反正本质上都是写半字，索性用下面的方式
		FLASH_ProgramHalfWord(addr, (unsigned short)wBuf[lenCount]); //写入

		addr += 2; //地址必须是2的倍数
	}
	
	FLASH_Lock(); //上锁

}
