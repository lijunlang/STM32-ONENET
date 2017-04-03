#ifndef _BEEP_H_
#define _BEEP_H_







typedef struct
{

	_Bool Beep_Status;

} BEEP_INFO;

#define BEEP_ON		1
#define BEEP_OFF	0

extern BEEP_INFO beepInfo;




void Beep_Init(void);

void Beep_Set(_Bool status);


#endif
