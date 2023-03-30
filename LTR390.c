#include "LTR390.h"
#include "SysTimer.h"

volatile int prevBar = 1;

void LTR390_Init() {
	uint8_t LTR390_Address = 0b1010011 << 1;
	
	uint8_t reset[2] = {0x00, 0x12};
	I2C_SendData(I2C1, LTR390_Address, reset, 2);
	
	delay(500);
}

void LTR390_Init_ALS() {
	uint8_t LTR390_Address = 0b1010011 << 1;
	
	uint8_t setALS_active[2] = {0x00, 0x02};
	I2C_SendData(I2C1, LTR390_Address, setALS_active, 2);
}

void LTR390_Init_UVS() {
	uint8_t LTR390_Address = 0b1010011 << 1;
	
	uint8_t setALS_active[2] = {0x00, 0x0A};
	I2C_SendData(I2C1, LTR390_Address, setALS_active, 2);
}

int LTR390_Get_ALS_Data() {
	uint8_t LTR390_Address = 0b1010011 << 1;
	
	uint8_t ALS_Data_Reg = 0x0D;
	I2C_SendData(I2C1, LTR390_Address, &ALS_Data_Reg, 1);
	
	// [0] = LSB, [2] = MSB
	uint8_t ALS_Data[3];
	I2C_ReceiveData(I2C1, LTR390_Address, ALS_Data, 3);
	
	int ALS_Output = (ALS_Data[2]<<16) + (ALS_Data[1]<<8) + ALS_Data[0];
	//printf("AL: %d\n", ALS_Output);
	return ALS_Output;
}

int LTR390_Get_UVS_Data() {
	uint8_t LTR390_Address = 0b1010011 << 1;
	
	uint8_t UVS_Data_Reg = 0x10;
	I2C_SendData(I2C1, LTR390_Address, &UVS_Data_Reg, 1);
	
	// [0] = LSB, [2] = MSB
	uint8_t UVS_Data[3];
	I2C_ReceiveData(I2C1, LTR390_Address, UVS_Data, 3);
	
	int UVS_Output = ((UVS_Data[2]&0x0F)<<16) + (UVS_Data[1]<<8) + UVS_Data[0];
	return UVS_Output;
}


double LTR390_Calc_Lux(int ALS_Out) {
	double numerator = 0.6 * (double)ALS_Out;
	double denominator = 3.0;
	double Lux = numerator / denominator;
	return Lux;
}

double LTR390_Calc_UVI(int UVS_Out) {
	double result = (double)UVS_Out / 230.0;
	return result;
}

void Write_Lux_Bargraph(double Lux, uint8_t HTK_Address) {
	int bar = (int)Lux / 2184;
	if (bar == 0) bar = 1;
	if (bar > 24) bar = 24;
	uint8_t setup3 = 0x81;
	
	if (bar > prevBar) {
		for (int i = prevBar+1; i <= bar; i++) {
			if (i < 9) {
				HTK_Set_Bar_Green(i, HTK_Address);
			} else if (i < 17) {
				HTK_Set_Bar_Orange(i, HTK_Address);
			} else {
				HTK_Set_Bar_Red(i, HTK_Address);
				if (bar == 24) {
					setup3 = 0x85;
					I2C_SendData(I2C1, HTK_Address, &setup3, 1);
				}
			}
		}
	} else if (bar < prevBar) {
		for (int i = prevBar; i > bar; i--) {
			HTK_Clear_Bar(i, HTK_Address);
		}
	}
	if (bar != 24) {
		setup3 = 0x81;
		I2C_SendData(I2C1, HTK_Address, &setup3, 1);
	}	
	prevBar = bar;
}
