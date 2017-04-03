#ifndef _STMFLASH_H_
#define _STMFLASH_H_







#define SSID_ADDR		0x0807F800 //账号保存地址	 							地址必须是2的倍数
#define PSWD_ADDR		0x0807F8A0 //密码保存地址	长度偏移160字节(80个半字)	地址必须是2的倍数

#define DEVID_ADDR      0x0807F000 //devid保存地址	 							地址必须是2的倍数
#define AKEY_ADDR       0x0807F0A0 //apikey保存地址	长度偏移160字节(80个半字)	地址必须是2的倍数

#define W_G_ADDR		0x0807E800 //启动时的选择方式							地址必须是2的倍数



_Bool Flash_NeedErase(void);

void Flash_Read(unsigned int addr, char *rBuf, unsigned short len);

void Flash_Write(unsigned int addr, char *wBuf, unsigned short len);


#endif
