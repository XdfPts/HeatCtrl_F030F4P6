#ifndef I2C_H	
#define I2C_H
#include "mytype.h"

//#define I2C_BUS I2C2


// ��������������� ����������
typedef enum _I2C_Direction {I2C_Transmitter=0, I2C_Receiver=1} I2C_Direction;

/*
������� �������� ��� ��������� ������ ���������
����� �� ������������ � ���� ���������� �����, � ��� ��� ��������
��������:
I2C->CR2 |= 1<<I2C_OFFSET_CR2_NBYTES;	// ����� �������, ��� ��������� 1 ����
I2C->CR2 |= 1<<16;						// ����� ��������� ��������, �� �� 1 ����, �� �� 16
*/

#define I2C_OFFSET_TIMINGR_SCLL		0
#define I2C_OFFSET_TIMINGR_SCLH		8
#define I2C_OFFSET_TIMINGR_SDADEL	16
#define I2C_OFFSET_TIMINGR_SCLDEL	20
#define I2C_OFFSET_TIMINGR_PRESC	28
#define I2C_OFFSET_CR2_NBYTES		16





void	I2C_Initialization ();
void	I2C_Start ();
byte	I2C_Stop  ();
byte	I2C_Write_Transaction (byte Adress,  byte *Data, byte Size);
byte	I2C_Write_Sym 				(byte Adress,  byte Sym);
byte	I2C_Read_Transaction  (byte Adress,  byte *Data, byte Size);


#endif



