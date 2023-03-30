#include "stm32l476xx.h"
#include "I2C.h"
#include "HT16K33.h"

void writeSevenSeg(double val, int prec, uint8_t address, int end);

void HTK_Init(uint8_t Address) {
	uint8_t setup1 = 0x21;
	uint8_t setup2 = 0xA0;
	uint8_t setup3 = 0x81;
	uint8_t setup4 = 0xEF;
	I2C_SendData(I2C1, Address, &setup1, 1);
	I2C_SendData(I2C1, Address, &setup2, 1);
	I2C_SendData(I2C1, Address, &setup3, 1);
	I2C_SendData(I2C1, Address, &setup4, 1);
}

void HTK_Blink(uint8_t Address) {
	uint8_t blink = 0x85;
	I2C_SendData(I2C1, Address, &blink, 1);
}

void HTK_Clear_Display(uint8_t Address) {
	uint8_t clear_all[17] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
														0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	HTK_SetData(clear_all, 17, Address);
}
														

void HTK_SetData(uint8_t* data, uint8_t size, uint8_t SlaveAddress) {
	I2C_SendData(I2C1, SlaveAddress, data, size);
}

void HTK_8x16_Set_Bit(uint8_t HTK_Address, int bit, int column) {
	uint8_t column_addresses[8] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E};
	uint8_t bit_masks[8] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
	uint8_t write_data[3];

	write_data[0] = column_addresses[column - 1];
	if (bit == -1) {
		write_data[1] = 0x00;
		write_data[2] = 0x00;
	} else if (bit <= 8) {
		write_data[1] = bit_masks[bit - 1];
		write_data[2] = 0x00;
	} else if (bit <= 16) {
		write_data[2] = bit_masks[bit - 9];
		write_data[1] = 0xFF;
	}
	
	HTK_SetData(write_data, 3, HTK_Address);
}

void HTK_8x16_Scroll_Values(int* values, int new_value, int size) {
	for (int i=size-1; i > 0; i--) {
		values[i] = values[i-1];
	}
	values[0] = new_value;		
}

void HTK_Set_Bar_Red(int bar, uint8_t Address) {
	if (bar > 0)
		HTK_Clear_Bar_Green(bar, Address);
	else
		bar = bar * -1;
	
	uint8_t data_loc = 0x00;
	uint8_t data[2];
	uint8_t curr_data[6];
	
	I2C_SendData(I2C1, Address, &data_loc, 1);
	I2C_ReceiveData(I2C1, Address, curr_data, 6);
	
	bar = bar - 1;
	int addr_lo = bar & 3;
	int addr_mid = bar >> 2;
	if(addr_mid >= 3){
		addr_mid -= 3;
		addr_lo += 4;
	}
	data[0] = addr_mid << 1;
	data[1] = (uint8_t) (1<<addr_lo) | curr_data[data[0]];
	HTK_SetData(data, 2, Address); 
}

void HTK_Set_Bar_Green(int bar, uint8_t Address) {
	if (bar > 0)
		HTK_Clear_Bar_Red(bar, Address);
	else
		bar = bar * -1;
	
	uint8_t data_loc = 0x00;
	uint8_t data[2];
	uint8_t curr_data[6];
	
	I2C_SendData(I2C1, Address, &data_loc, 1);
	I2C_ReceiveData(I2C1, Address, curr_data, 6);

	bar = bar - 1;
	int addr_lo = bar & 3;
	int addr_mid = bar >> 2;
	if(addr_mid >= 3){
		addr_mid -= 3;
		addr_lo += 4;
	}
	data[0] = (addr_mid << 1) + 1;
	data[1] = (uint8_t) (1<<addr_lo) | curr_data[data[0]];
	HTK_SetData(data, 2, Address); 
}

void HTK_Clear_Bar_Red(int bar, uint8_t Address) {
	uint8_t data_loc = 0x00;
	uint8_t data[2];
	uint8_t curr_data[6];
	
	I2C_SendData(I2C1, Address, &data_loc, 1);
	I2C_ReceiveData(I2C1, Address, curr_data, 6);

	
	bar = bar - 1;
	int addr_lo = bar & 3;
	int addr_mid = bar >> 2;
	if(addr_mid >= 3){
		addr_mid -= 3;
		addr_lo += 4;
	}
	data[0] = addr_mid << 1;
	data[1] = (uint8_t) ~(1<<addr_lo) & curr_data[data[0]];
	HTK_SetData(data, 2, Address); 
}

void HTK_Clear_Bar_Green(int bar, uint8_t Address) {
	uint8_t data_loc = 0x00;
	uint8_t data[2];
	uint8_t curr_data[6];
	
	I2C_SendData(I2C1, Address, &data_loc, 1);
	I2C_ReceiveData(I2C1, Address, curr_data, 6);
	
	bar = bar - 1;
	int addr_lo = bar & 3;
	int addr_mid = bar >> 2;
	if(addr_mid >= 3){
		addr_mid -= 3;
		addr_lo += 4;
	}
	data[0] = (addr_mid << 1) + 1;
	data[1] = (uint8_t) ~(1<<addr_lo) & curr_data[data[0]];
	HTK_SetData(data, 2, Address);
}

void HTK_Set_Bar_Orange(int bar, uint8_t Address) {
	HTK_Set_Bar_Green(-bar, Address);
	HTK_Set_Bar_Red(-bar, Address);
}

void HTK_Clear_Bar(int bar, uint8_t Address) {
	HTK_Clear_Bar_Green(bar, Address);
	HTK_Clear_Bar_Red(bar, Address);
}

int HTK_Color_Bars_Lux(double Lux, int curr_bar, uint8_t address) {
	int bar;
	
	if (Lux < 125) {
		bar = 1;
	} else if (Lux < 1000) {
		bar = (int)(Lux / 125.0);
	} else if (Lux < 10000) {
		bar = 8 + (((int)Lux-999) / 113);
	} else if (Lux < 50000) {
		bar = 16 + (((int)Lux - 9999) / 5000);
	} else {
		bar = 24;
	}
	
	if (bar > curr_bar) {
		for (int i=1; i <= bar+1; i++) {
			if (i <= 8) {
				HTK_Set_Bar_Green(i, address);
			} else if (i <= 16) {
				HTK_Set_Bar_Orange(i, address);
			} else if (i <= 24) {
				HTK_Set_Bar_Red(i, address);
			}
		}
	} else if (bar < curr_bar) {
		for (int i=curr_bar; i > bar; i--) {
			HTK_Clear_Bar(i, address);
		}
	}
	
	return bar+1;
}

int HTK_Color_Bars_UVI(double UVI, int curr_bar, uint8_t address) {
	int bar;
	
	if (UVI < 0.25) {
		bar = 1;
	}	else if (UVI <= 2) {
		bar = (int)(UVI/0.25);
	} else if (UVI <= 6) {
		bar = 8 + (int)((UVI-2.0) / 0.5);
	} else if (UVI <= 10) {
		bar = 16 + (((int)UVI-6) / 0.5);
	} else {
		bar = 24;
	}
	
	if (bar > curr_bar) {
		for (int i=1; i <= bar+1; i++) {
			if (i <= 8) {
				HTK_Set_Bar_Green(i, address);
			} else if (i <= 16) {
				HTK_Set_Bar_Orange(i, address);
			} else if (i <= 24) {
				HTK_Set_Bar_Red(i, address);
			}
		}
	} else if (bar < curr_bar) {
		for (int i=curr_bar; i > bar; i--) {
			HTK_Clear_Bar(i, address);
		}
	}
	
	return bar+1;
}		

int HTK_Color_Bars_HR(double hr, int curr_bar, uint8_t address) {
	int bar;
	
	if (hr < 8) {
		bar = 1;
	}	else if (hr <= 32) {
		bar = (int)hr / 8;
	} else if (hr <= 52) {
		bar = 4 + (int)(hr-32)/5;
	} else if (hr <= 156) {
		bar = 8 + ((int)hr - 52)/13;
	} else if (hr <= 180) {
		bar = 16 + ((int)hr - 156)/6;
	} else if (hr <= 190) {
		bar = 20 + ((int)hr - 180)/3;
	} else {
		bar = 24;
	}
	
	if (bar > curr_bar) {
		for (int i=1; i <= bar+1; i++) {
			if (i<=4 || i >= 21) {
				HTK_Set_Bar_Red(i, address);
			} else if (i<=8 || i>= 17) {
				HTK_Set_Bar_Orange(i, address);
			} else if (i <= 16) {
				HTK_Set_Bar_Green(i, address);
			}
		}
	} else if (bar < curr_bar) {
		for (int i=curr_bar; i > bar; i--) {
			HTK_Clear_Bar(i, address);
		}
	}
	
	return bar+1;
}	
	
int HTK_Color_Bars_BT(double temp, int curr_bar, uint8_t address) {
	int bar;
	
	if (temp < 20) {
		bar = 1;
	}	else if (temp <= 80) {
		bar = (int)temp / 20;
	} else if (temp <= 91) {
		bar = 4 + (int)((temp-78)/3);
	} else if (temp <= 99) {
		bar = 8 + ((int)temp - 91);
	} else if (temp <= 99.99) {
		bar = 16 + (int)((temp - 99)/0.19);
	} else if (temp <= 103) {
		bar = 20 + (int)temp - 99;
	} else {
		bar = 24;
	}
	
	if (bar > curr_bar) {
		for (int i=1; i <= bar+1; i++) {
			if (i<=4) {
				HTK_Set_Bar_Red(i, address);
			} else if (i<=8) {
				HTK_Set_Bar_Orange(i, address);
			} else if (i <= 16) {
				HTK_Set_Bar_Green(i, address);
			} else if (i <= 20) {
				HTK_Set_Bar_Orange(i, address);
			} else {
				HTK_Set_Bar_Red(i, address);
			}
		}
	} else if (bar < curr_bar) {
		for (int i=curr_bar+1; i > bar; i--) {
			HTK_Clear_Bar(i, address);
		}
	}
	
	return bar;
}	

void SetBit(int bit, int value, uint8_t* data) {
	uint8_t column_masks[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	
	for (int i = 0; i < 8; i++) {
		if (bit < 8) {
			data[1+(2*i)] &= ~column_masks[bit];
			if ((value-1) == i) {
				data[1+(2*i)] |= column_masks[bit];
			}	
		} else {
			data[2+(2*i)] &= ~column_masks[bit-8];
			if ((value-1) == i) {
				data[2+(2*i)] |= column_masks[bit-8];
			}
		} 
	}	
}



void ScrollValues(int* data, int newData) {
	for (int i = 16; i > 1; i--) {
		data[i-1] = data[i-2];
	}
	data[0] = newData;
}

void HTK_Set_SevenSeg(int digit, int value, int dec, uint8_t address) {
	uint8_t digits[13] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, 0x39, 0x71, 0x00};
	uint8_t data[2] = {0x00, 0x00};
	
	data[0] = 0x08 - (uint8_t)(2*digit);
	if (digit <= 2) data[0] += 0x02;
	data[1] = digits[value];
	if (dec == 1) data[1] = data[1] | 0x80;
	HTK_SetData(data, 2, address);	

}

void HTK_WriteNum_SevenSeg(int num, uint8_t address) {
	if(num > 0) {
		writeSevenSeg(num, 0, address, 0);
	}
}

void HTK_Write_Temp_SevenSeg(double temp, int C, uint8_t address) {
	writeSevenSeg(temp, temp >= 100 ? 1 : 2, address, C == 1 ? 10 : 11);
}

void HTK_Write_Humidity_SevenSeg(double humidity, uint8_t address) {
	if (humidity >= 100) {
		writeSevenSeg(100, 1, address, 0);
	} else {
		writeSevenSeg(humidity, 2, address, 0);
	}	
}

void HTK_Write_AQI(double aqi, uint8_t address) {
	writeSevenSeg(aqi, aqi < 100 ? 2 : 1, address, 0);
}

void writeSevenSeg(double val, int prec, uint8_t address, int end) {
	int i;
	int dig[4];
	double frac = val - (int) val;
	int inte;
	for(i = 0; i < prec; i++) {
		frac = frac * 10;
		inte = (int) frac;
		dig[prec - i - 1] = inte;
		frac -= inte;
	}
	inte = (int) val;
	for(i = 0; i < 4 - prec; i++) {
		if(inte == 0){
			dig[i + prec] = 12;
		} else {
			dig[i + prec] = inte % 10;
			inte /= 10;
		}
	}
	if(end > 0) {
		dig[0] = end;
	}
	for(i = 0; i < 4; i++){
		HTK_Set_SevenSeg(i + 1, dig[i], prec == i ? 1 : 0, address);
	}
}
