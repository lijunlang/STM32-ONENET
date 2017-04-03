#ifndef _NET_IO_H_
#define _NET_IO_H_







typedef struct
{
	
	unsigned short dataLen;			//接收数据长度
	unsigned short dataLenPre;		//上一次的长度数据，用于比较
	
	unsigned char buf[256];			//接收缓存

} NET_IO_INFO;

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

#define NET_IO		USART2

extern NET_IO_INFO netIOInfo;







void NET_IO_Init(void);

void NET_IO_Send(unsigned char *str, unsigned short len);

_Bool NET_IO_WaitRecive(void);

void NET_IO_ClearRecive(void);


#endif
