 //#include "stm32f0xx.h"
#include "stm32f030x8.h"
#include "lcd_i2c.h"  
#include "srv.h"  

/*
 //--- diplay 16 x 2 ----
	PB10 - SCL I2C2
	PB11 - SDA I2C2
 -------------- analog inputs --------
  PA1 <- current sensor   
  PA2 <- output voltage sensor  
  PA3 <- power control resistor
  
 ---------------- dig inputs  ---------
  PA0 <- digital input 
  PA5 <- Pusk 
  PA6 <- Stop
---------------- dig out ---------
  PA7 -> gpio output - управление выходным ключом 
  PA4 -> onboard LED 
*/


volatile unsigned sysTimer;

extern "C" void SysTick_Handler() 
	{  sysTimer++; 
	   if ((sysTimer % 1000) == 0) 
		   GPIOA->ODR ^= BIT(4);
	}
extern  int TmpC;
extern  uns PowCtrl,LoadCurrent,LoadVoltage;
int		AdcLoopF();
void  	ConfigureADC();
extern volatile uns Reference;

void	ConfigureGPIO()
{  
  // Select output mode (01) on GPIOC pin 8 and 9 => LED3 & LED4 onboard STM32F0 disco
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
   //------------------------
  // PA 1,3,6  - as ADC input PA4 - gpio output 
   
   GPIOA->MODER &= ~(GPIO_MODER_MODER0| GPIO_MODER_MODER5 | GPIO_MODER_MODER6);  // reset bits = gpio input
   GPIOA->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER7_0	|		// // LED & key
				  GPIO_MODER_MODER1   | GPIO_MODER_MODER2   | GPIO_MODER_MODER3; // 3 -  analog mode   - TTa - 3.3 V tolerant I/O directly connected to ADCtype
  // PA 0,5,6     - as dig up input = 0 

   GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR5_0 | GPIO_PUPDR_PUPDR6_0; // 01 - pull up

   
 //	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;  // ~3
 // GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_0; // 01 - pull up

}

 //-----------------------------------------------------------------
bool	IsDeltaT(unsigned* FromTime,unsigned dt)
 //-----------------------------------------------------------------
 {
   if (sysTimer- *FromTime < dt)  return false;
   *FromTime += dt;
   return true;
  }
unsigned	abs(int val) { if (val < 0) val *= -1; return val;}

 //-----------------------------------------------------------------
int		main()
 //-----------------------------------------------------------------	
 { u32 Tindic=0; uns prevPow=0;
  SysTick_Config(SystemCoreClock / 1000);
  ConfigureGPIO();
  LCDI2C_init(0x38,16,2);
  LCDI2C_clear();
  LCDI2C_command(192);
  ConfigureADC();
  //LCDI2C_backlight();
	//CreateBigNum();
  
  //byte cx,cy; cx = cy = 0;
  while (1)
  {
		AdcLoopF();
		if (IsDeltaT( &Tindic,500 ) || abs(PowCtrl -prevPow ) > 100)
		{
		 //GPIOC->ODR ^= BIT(LED3_PIN);
		 LCDI2C_setCursor(0,0);
	     printf("рег/ток/напряж",PowCtrl);
		 prevPow = PowCtrl;
		 LCDI2C_setCursor(0,1);
		 //unsigned V = 	
		 printf(" %u %u %u ",PowCtrl,LoadCurrent,LoadVoltage);
		}
	    
		//LCDI2C_write_String(txt);
		//LCDI2C_setCursor(0,1); LCDI2C_write_String("Ток");
		//for (int cnt = 1; cnt <= 9; cnt++) {
      	//Delay(1000);
      	//printBigNum(cnt, 5, 0);
    }
  }

//Мощность 204 238
