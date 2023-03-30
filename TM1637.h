#ifndef TM1637_H
#define TM1637_H

#include "stm32l476xx.h"
#include "SysTimer.h"

static uint8_t digits[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};

void TM1637_Init(void);

void Set_DIO_Input(void);
void Set_DIO_Output(void);

void Set_CLK_Input(void);
void Set_CLK_Output(void);

void StartTM(void);
void StopTM(void);

void WriteByte(uint8_t word);

void WriteNumTM(int num);

#endif
