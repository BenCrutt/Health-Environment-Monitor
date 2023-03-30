#ifndef LTR390_H
#define LTR390_H

#include "stm32l476xx.h"
#include <stdio.h>
#include "I2C.h"
#include "HT16K33.h"

void LTR390_Init();

void LTR390_Init_ALS();

void LTR390_Init_UVS();

int LTR390_Get_ALS_Data();

int LTR390_Get_UVS_Data();

double LTR390_Calc_Lux(int ALS_Out);

double LTR390_Calc_UVI(int UVS_Out);

void Write_Lux_Bargraph(double Lux, uint8_t HTK_Address);


#endif