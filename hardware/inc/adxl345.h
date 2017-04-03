#ifndef _ADXL345_H_
#define _ADXL345_H_


    //0 ±16g，13位模式
#define DATA_FORMAT_REG  0x31
    //0x08 测量模式
#define POWER_CTL                0x2d
    //0x80 使能DATA_READY中断,需要吗，需要禁止的吧。
#define INT_ENABLE       0x2e
#define BW_RATE 0x2c
#define OFSX 0x1e
#define OFSY 0x1f
#define OFSZ 0x20

#define ADXL345_ADDRESS 0x53



typedef struct
{

	unsigned char incidence_X;
	unsigned char incidence_Y;
	unsigned char incidence_Z;
	
	float incidence_Xf;
	float incidence_Yf;
	float incidence_Zf;

} ADXL345_INFO;

extern ADXL345_INFO adxlInfo;




void ADXL345_Init(void);

void ADXL345_GetValue(void);


#endif
