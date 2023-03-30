#ifndef BME688_H
#define BME688_H

#include "I2C.h"

void BME688_Init();

// Must add delay after taking measurement before reading values
void BME688_Take_Measurement();

double BME688_Get_Temp_C();

double BME688_Get_Humidity();

double BME688_Get_Pressure();

double BME688_Get_Gas();

double BME688_Calc_AQI(double gas_r);

#endif