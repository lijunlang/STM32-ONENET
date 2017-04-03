#ifndef _GY30_H_
#define _GY30_H_







#define BH1750FVI_ADDR 0x23

#define BH1750_DOWN   0x0
#define BH1750_ON   0x01
#define BH1750_RSET 0x07
//commended
#define BH1750_Con_High_RM  0x10
//commended
#define BH1750_Con_High_RM2  0x11
#define BH1750_Con_Low_RM  0x13
//auto power down
#define BH1750_Once_High_RM  0x20
#define BH1750_Once_High_RM2  0x21
#define BH1750_Once_Low_RM  0x23



typedef struct
{

	unsigned short lightVal;
	
	unsigned char initStep; //≥ı ºªØ≤Ω÷Ë

} GY30_INFO;

extern GY30_INFO gy30Info;





void GY30_Init(void);

_Bool GY30_GetValue(void);


#endif
