#ifndef _OLED_H_
#define _OLED_H_







extern const unsigned char wen[];
extern const unsigned char du[];
extern const unsigned char shi[];
extern const unsigned char san[];
extern const unsigned char zhou[];
extern const unsigned char zhuang[];
extern const unsigned char tai[];
extern const unsigned char wei[];
extern const unsigned char lian[];
extern const unsigned char jie[];
extern const unsigned char zhong[];
extern const unsigned char yi[];
extern const unsigned char duan[];
extern const unsigned char kai[];


#define OLED_ADDRESS	0x78



void OLED_Init(void);

_Bool OLED_WriteCom(unsigned char cmd);

void OLED_Address(unsigned char page, unsigned char column);

void OLED_ClearScreen(void);

void OLED_ClearAt(unsigned char x);

void OLED_Dis12864_Pic(const unsigned char *dp);

void OLED_DisChar16x16(unsigned short page, unsigned short column, const unsigned char *dp);

void OLED_DisString6x8(unsigned char x, unsigned char y, char *fmt, ...);

void OLED_DisString8x16(unsigned char x, unsigned char y, char *fmt, ...);


#endif
