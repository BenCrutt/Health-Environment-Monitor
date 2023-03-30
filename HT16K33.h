#ifndef HT16K33_H
#define HT16K33_H
#include "stm32l476xx.h"

void HTK_SetData(uint8_t* data, uint8_t size, uint8_t SlaveAddress);

void SetBit(int bit, int value, uint8_t* data);

void ScrollValues(int* data, int newData);

void HTK_Init(uint8_t Address);

void HTK_Clear_Display(uint8_t Address);

void HTK_Set_Bar_Red(int bar, uint8_t Address);

void HTK_Set_Bar_Green(int bar, uint8_t Address);

void HTK_Clear_Bar_Green(int bar, uint8_t Address);

void HTK_Clear_Bar_Red(int bar, uint8_t Address);

void HTK_Set_Bar_Orange(int bar, uint8_t Address);

void HTK_Clear_Bar(int bar, uint8_t Address);
	
void HTK_8x16_Set_Bit(uint8_t HTK_Address, int bit, int column);

void HTK_8x16_Scroll_Values(int* values, int new_value, int size);

int HTK_Color_Bars_Lux(double Lux, int curr_bar, uint8_t address);

void HTK_Set_SevenSeg(int digit, int value, int dec, uint8_t address);

void HTK_Write_Temp_SevenSeg(double temp, int C, uint8_t address);

void HTK_Write_Humidity_SevenSeg(double humidity, uint8_t address);

void HTK_Write_AQI(double aqi, uint8_t address);

int HTK_Color_Bars_UVI(double UVI, int curr_bar, uint8_t address);

int HTK_Color_Bars_HR(double hr, int curr_bar, uint8_t address);

int HTK_Color_Bars_BT(double temp, int curr_bar, uint8_t address);

void HTK_WriteNum_SevenSeg(int num, uint8_t address);

void HTK_Blink(uint8_t Address);

#endif