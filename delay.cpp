//#include "stm32f0xx.h"
#include "stm32f030x8.h"
#include "srv.h"  


//---------------------------------------------
u32		GetTime_us()
 //---------------------------------------------
 {
   return sysTimer * 1000 + 48000 - SysTick->VAL;
 }
 //-------------------------------------
void	Delay_us(unsigned us )
 //-------------------------------------
 {
	unsigned st  = sysTimer;
	unsigned stu = 48000 - SysTick->VAL; // from last ms
  	unsigned ticks=us*48 + stu;
	for(;;)
	{
	  unsigned  ct = (sysTimer - st)*48000 + 48000 - SysTick->VAL;
	  if ( ct > ticks) break;
		
	}
}
 //-------------------------------------
void	Delay(unsigned ms)
 //-------------------------------------
{ 
	unsigned st = sysTimer+ms;
	for(;;) if(sysTimer > st) break;
}
