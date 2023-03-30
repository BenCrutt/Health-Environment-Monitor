#include "MAX30205.h"
#include "I2C.h"

double MAX30205_Get_Temp() {
	uint8_t MAX30205_Address = 0b1001000 << 1;
	uint8_t temp_max = 0x00;
	uint8_t max_temp[4];
	I2C_SendData(I2C1, MAX30205_Address, &temp_max, 1);
	I2C_ReceiveData(I2C1, MAX30205_Address, max_temp, 2);
	double temperature_c = (double)max_temp[0] + ((double)max_temp[1] / 256.0);
	return temperature_c;
}