#include "stm32f0xx.h"
#include "i2c.h"
#include "srv.h"

/*
  //-- for DSCf030  PB10 - SCL I2C2  PB11 - SDA I2C2
  for F030F4P6  
  PA9   - SCL
  PA10  - SCA
*/
#define GPIO_I2C GPIOA
#define I2C_BUS I2C1

void  Configure_GPIO_I2C()
 {
  
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  // Enable the peripheral clock of GPIOA 
	
  GPIO_I2C->OTYPER |= GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_9;         // Open drain for I2C signals 
  GPIO_I2C->AFR[1] = (GPIO_I2C->AFR[1] &~ (GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2)) \
                  | (4 << (2 * 4)) | (4 << (1 * 4));              // AF=4 for I2C signals 9 & 10  
  GPIO_I2C->MODER = (GPIO_I2C->MODER & ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10)) \
                 | (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER9_1); // Select AF mode (b 10) on 9 and 10
  
}


void I2C_Initialization ()
 {
	
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;		// явно вкл. clk I2C
	RCC->CFGR3 |= RCC_CFGR3_I2C1SW; 		// System clock (SYSCLK) selected as I2C1 clock
	I2C_BUS->CR1 &= ~I2C_CR1_PE;			// Отключаю I2C
	Configure_GPIO_I2C();
	
	/* Частота тактирования модуля I2C = 48 МГц
	// Частота шины I2C = 100 kHz
	// Настраиваю тайминги
	I2C_BUS->TIMINGR |= \
	(0xB  << I2C_OFFSET_TIMINGR_PRESC)|\
	(0x13 << I2C_OFFSET_TIMINGR_SCLL)|\
	(0xF  << I2C_OFFSET_TIMINGR_SCLH)|\
	(0x4  << I2C_OFFSET_TIMINGR_SCLDEL)|\
	(0x2  << I2C_OFFSET_TIMINGR_SDADEL); */
	//I2C_BUS->TIMINGR = (uint32_t)0x00B01A4B; 

	I2C_BUS->TIMINGR = 0x10805E89;; 		// AN
  	I2C_BUS->CR1     = I2C_CR1_PE; 			// Включаю I2C
 }

 //---------------------------------------------------------------
byte	I2C_Write_Sym (byte Adress,  byte Sym)
 //---------------------------------------------------------------
{
    //int busyCnt = 0;
	// Старт
  	I2C_BUS->CR2 &= ~I2C_CR2_RD_WRN; // master write
	I2C_BUS->CR2 =  I2C_CR2_AUTOEND | (1<<16) | (Adress <<1);    // 1 byte 
	for(int WrWaitCnt = 0;WrWaitCnt < 0x8000;WrWaitCnt++)
	{
 	if((I2C_BUS->ISR & I2C_ISR_TXE) == 0) 
	{
		 //if(I2C_BUS->ISR & I2C_ISR_BUSY)  busyCnt ++;
		 //if( busyCnt >  
		 if(I2C_BUS->ISR & I2C_ISR_NACKF) break; // from for -> ret
		 if(I2C_BUS->ISR & I2C_ISR_ARLO)  break; // arbitration lost - 
		 continue;
	}
	I2C_BUS->TXDR = Sym; 
   	I2C_BUS->CR2 |= I2C_CR2_START; 
	return 0;
	}
	//I2C_BUS->CR1  &= ~I2C_CR1_PE;
	//Delay_us(1200);
	//I2C_BUS->CR1  |= I2C_CR1_PE;
    return 1;
}

