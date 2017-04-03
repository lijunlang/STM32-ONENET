#ifndef _SELFCHECK_H_
#define _SELFCHECK_H_







typedef struct
{

	unsigned short SHT20_OK : 1;		//温湿度传感器正常标志位
	unsigned short ADXL345_OK : 1;		//三轴传感器正常标志位
	unsigned short EEPROM_OK : 1;		//存储器正常标志位
	unsigned short OLED_OK : 1;			//OLE正常标志位
	
	unsigned short NET_DEVICE_OK : 1;	//网络设备正常标志位

} CHECK_INFO;

#define DEV_OK		1
#define DEV_ERR		0

extern CHECK_INFO checkInfo;



void Check_PowerOn(void);


#endif
