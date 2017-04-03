#ifndef _RELAY_H_
#define _RELAY_H_



typedef struct
{
	_Bool J1Sta;
	
	_Bool J2Sta;

} RELAY_STATUS;

extern RELAY_STATUS relayStatus;

typedef enum
{

	RELAY_OFF = 0,
	RELAY_ON

} RELAY_ENUM;








void Relay_Init(void);


void J1_Set(RELAY_ENUM status);

void J2_Set(RELAY_ENUM status);


#endif
