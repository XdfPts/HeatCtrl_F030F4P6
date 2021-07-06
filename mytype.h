#ifndef _MYTYPE_H
#define _MYTYPE_H

	#define BIT(bn) (1 << (bn))
	
//  #define SetPinHi(pin) (GPIOA->BSRR = BIT(pin))
//	#define SetPinLo(pin) (GPIOA->BRR  = BIT(pin))
//	#define SETBIT(pin)   (GPIOA->ODR |= BIT(pin))

	#define getBit(pin) (GPIOA->IDR & BIT((pin)))



  //#define sbi(pin) 		GPIO->BSRR = BIT(pin)
  //#define cbi(pin) 		GPIO->BRR  = BIT(pin)

  #define byte unsigned char
  #define uns  unsigned short
  #define u32  unsigned

  typedef void (*putnF) (unsigned char);
  typedef u32  (*prn)   (const char*,...);
  typedef unsigned char uint8_t;

  #define DIM(arr) (sizeof(arr)/sizeof(arr[0]))
  #define sz(var) sizeof(var)
  #define uconst unsigned const
  #define cchar const char

#endif
