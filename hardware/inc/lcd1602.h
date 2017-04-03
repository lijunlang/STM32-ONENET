#ifndef _LCD1602_H_
#define _LCD1602_H_







void Lcd1602_Init(void);

void Lcd1602_WriteData(unsigned char byte);

void Lcd1602_WriteCom_Busy(unsigned char byte);

void Lcd1602_Clear(unsigned char pos);

void Lcd1602_DisString(unsigned char pos, char *fmt,...);


#endif
