 #include "stm32f030x6.h"
 //#include "stm32f0xx.h"
 #include "mytype.h"
/*
 -------------- analog inputs --------
  PA1 <- current sensor   
  PA2 <- output voltage sensor  
  PA3 <- power control resistor
 */

 //#include "adc_aq.h"
void	Delay(unsigned);

u32		abs(int);


#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7BA)) //Internal voltage reference raw value at 30 degrees C, VDDA=3.3V
//Temperature sensor raw value at 30 degrees C, VDDA=3.3V
#define TEMP30_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
//Temperature sensor raw value at 110 degrees C, VDDA=3.3V
#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
//Internal voltage reference raw value at 30 degrees C, VDDA=3.3V

struct  adcChVal
 {
	volatile uns	measure[16];		// выборки
	volatile uns	avr;				// среднее
	volatile uns	filtr;				// среднее
	volatile unsigned Akk;
	volatile byte	indx,cnt;			// index
	volatile byte	ready;

	void	app(uns val)  
	{Akk += val; 
	 measure[indx++ & 15] = val;
	 if (indx > 16) ready = 1;
	 if (++cnt == 0) 
	 { avr = Akk / 256; Akk = 0; 
	   
	   
	 }
	}
	uns		Filtr() { for(int i =0;i<DIM(measure);i++) 
						{ if ( filtr == 0) filtr  = avr;
						 unsigned v = measure[i];
						 if ( v != filtr ) { if ( v < filtr) filtr--; else filtr++;}
						 
					    }
					 return filtr;
					}	
	void	reset() {Akk = indx = cnt = ready = filtr = 0;}
 };
adcChVal Current_s,
		 pow_reg,
		 OutVoltg_s,
		 TempSens,
		 Calibr;
 
int TmpC;
uns PowCtrl,LoadCurrent,LoadVoltage;
volatile unsigned AdcErrCnt;
 
volatile unsigned short PowerLevel,  // потенциометр регулировки мощности
						Current,	 // датчик тока
						OutVoltage,	 // вых. напряжение
						TemprSensor, // измерение температурного датчика
						Reference;   // калибровка АЦП - измерение образцового источника 
volatile byte 		OverCnt,adcCnt;
unsigned			AdcCount;
unsigned 			VDD_CALIB;
unsigned 			T110;
unsigned 			T30;
volatile  bool		AdcLockFlag;

  //----------------------------------------------------------
extern "C" void	ADC1_IRQHandler()
  //----------------------------------------------------------
  {
  if ((ADC1->ISR & ADC_ISR_OVR) != 0)  /* Check OVR has triggered the IT */
  {
    ADC1->ISR |= ADC_ISR_OVR; // Clear 
    ADC1->CR  |= ADC_CR_ADSTP; /* Stop the sequence conversion */
	AdcLockFlag = true;
  }
  if (AdcLockFlag ) return;
	
  if ((ADC1->ISR & ADC_ISR_EOC) != 0)  /* checks EOC has triggered the IT */
   {
		  uns adcDr = ADC1->DR; 
		  switch(adcCnt++)
			{
				case 0: 	Current_s.app(adcDr);	// PA1
							break;	
				case 1: 	OutVoltg_s.app(adcDr);  // PA2
							break;	
				case 2: 	pow_reg.app(adcDr);		// PA3
							break;	
				case 3: 	TempSens.app(adcDr);  	// 16
							break;	
		 		case 4:		Calibr.app(adcDr); 	  	// 17
							break;	
				default:
							AdcErrCnt++;OverCnt++;
			}
  }
	if (ADC1->ISR & ADC_ISR_EOSEQ) 			/* Check conversion is completed */   
	 {
	   adcCnt=0;						 
	   ADC1->ISR = ADC_ISR_EOSEQ;
	   //ADC1->CR |= ADC_CR_ADSTART;		// to much speed - use TIM3 TRGO instead
	   AdcCount++;
	}
}
void		ConfigureTIM3()  
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; 		// (1) Enable the peripheral clock of the TIM3  
  TIM3->CR2 |= TIM_CR2_MMS_1; 				// (2) Configure MMS=010 to output a rising edge at each update event 
  TIM3->PSC = 959; 							// (3) Select PCLK/960 i.e. 48MHz/960=50kHz 
  TIM3->ARR = (uint16_t)50; 				// (4) 50 - Set one update event each 1 ms 
  TIM3->CR1 |= TIM_CR1_CEN; 				// (5) Enable TIM3 
}

//----------------------------------------------------------
void	CalibrateADC()
 //----------------------------------------------------------
{
  
  if (ADC1->CR & ADC_CR_ADEN) 
	  {
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN) {}
  }
  
  ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN; 
  ADC1->CR |= ADC_CR_ADCAL; 		 	// Launch the calibration by setting ADCAL
  
  while ((ADC1->CR & ADC_CR_ADCAL) != 0) {} // Wait until ADCAL

	
  //ADC1->CR |= ADC_CR_ADEN;								  /* enable device */
  //while(!(ADC1->ISR & ADC_ISR_ADRDY));
}
//------------------------------------------------------------------------------
void  ConfigureADC()
//------------------------------------------------------------------------------
{
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;   	   			 
  RCC->CR 	   = (RCC->CR & (~RCC_CFGR_PPRE)); 		
  ConfigureTIM3();
	
  ADC1->CFGR2  = ADC_CFGR2_CKMODE_1;							  				// 10 - hi bits => PCLK_4
  //ADC1->CFGR1 |= ADC_CFGR1_WAIT;   								  			// Select the WAIT mode 
  ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0 | (ADC_CFGR1_EXTSEL_1 | ADC_CFGR1_EXTSEL_0); // Select the external trigger TRG3 - on falling edge and external trigger on TIM3_TRGO 
	

  ADC1->SMPR  |= (ADC_SMPR_SMP_0  | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2) ; 				// measure time 240 clk 
  ADC->CCR 	  |= (ADC_CCR_VREFEN  | ADC_CCR_TSEN); 

  ADC1->CHSELR = ADC_CHSELR_CHSEL1 | ADC_CHSELR_CHSEL3 | ADC_CHSELR_CHSEL2
								   | ADC_CHSELR_CHSEL17 | ADC_CHSELR_CHSEL16 ;		// ch0 & REFINT & TempSensor
  	
  CalibrateADC();

  	if ((ADC1->ISR & ADC_ISR_ADRDY) != 0) ADC1->ISR |= ADC_ISR_ADRDY;
  	ADC1->CR |= ADC_CR_ADEN; /* (3) */
  	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) {}

	VDD_CALIB = *VREFINT_CAL_ADDR;
	T110	  = *TEMP110_CAL_ADDR;
	T30	   	  = *TEMP30_CAL_ADDR;

	ADC1->IER = ADC_IER_EOCIE | ADC_IER_EOSEQIE | ADC_IER_OVRIE; 
	ADC1->CR |= ADC_CR_ADSTART; 	/* start the ADC conversions */

		
	NVIC_EnableIRQ	(ADC1_COMP_IRQn); 
	NVIC_SetPriority(ADC1_COMP_IRQn,0); 

}
 //------------------------------------------------------------------
int		CalcTemper	(int TempVal)
 //-----------------------------------------------------------------
 {
	int	register temperature; 		  // will contain the temperature in % degrees Celsius 
	if (Reference < 1000) return -100;
    temperature = TempVal * VDD_CALIB / Reference - T30;
    //temperature *= (int32_t)(110000 - 30000);
    temperature = temperature * 80000 / (int32_t)(T110 - T30);  // 80000 = 110000 - 30000
    temperature += 30000;
		temperature /= 1000;
    return	temperature;
 }
//-----------------------------------------------------------------
 uns		getVoltage(uns adcCode)
 //-----------------------------------------------------------------
 {  unsigned V = (adcCode * VDD_CALIB) / Reference;
	 V = (V * 3300)/4095;
    return V;
 }
 //-----------------------------------------------------------------
int		AdcLoopF()
 //-----------------------------------------------------------------
 {
	 if (AdcLockFlag)
	 {
		if ((ADC1->CR  & ADC_CR_ADSTP) == 0) // cleared by hardware
		{
			ADC1->CR |= ADC_CR_ADSTART;
		    AdcLockFlag = 0;
			AdcErrCnt++;
		}
	 }	
	if (Calibr.ready)
	{
		Calibr.ready = 0;
		Reference	= Calibr.Filtr();
		TmpC   		= CalcTemper(TempSens.avr);
		PowCtrl		= getVoltage ( pow_reg.Filtr() );
		LoadVoltage = getVoltage ( OutVoltg_s.Filtr() ); 
		LoadCurrent = getVoltage ( Current_s.Filtr() ); 
		
	}	
	return 0;
 }

