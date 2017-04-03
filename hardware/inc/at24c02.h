#ifndef _AT24C02_H_
#define _AT24C02_H_







#define AT24C02_ADDRESS		0x50 //AT24C02µÿ÷∑




void AT24C02_WriteByte(unsigned char regAddr, unsigned char byte);

void AT24C02_WriteBytes(unsigned char regAddr, unsigned char *byte, unsigned char len);

void AT24C02_ReadByte(unsigned char regAddr, unsigned char *byte);

void AT24C02_ReadBytes(unsigned char regAddr, unsigned char *byte, unsigned char len);

_Bool AT24C02_Clear(unsigned char startAddr, unsigned char byte, unsigned short len);


#endif
