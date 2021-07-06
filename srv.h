#pragma once
#include "mytype.h"  

extern volatile unsigned sysTimer;
extern "C"
	void	printf	(const char * sh,...);

u32		sprintf	(char* str,const char * sh,...);
void	Delay_us(unsigned);
void	Delay	(unsigned);



