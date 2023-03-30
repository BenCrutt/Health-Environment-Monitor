#include "BME688.h"

void BME688_Init() {
		uint8_t SlaveAddress = 0b1110110 << 1; // BME688 Address
		
		// BME688
		uint8_t recData[3];
		
		uint8_t get_osrs_h = 0x72;
		I2C_SendData(I2C1, SlaveAddress, &get_osrs_h, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		uint8_t set_osrs_h[2] = {0x72, (0x01|(recData[0]&0xF8))}; // Set osrs_H
		I2C_SendData(I2C1, SlaveAddress, set_osrs_h, 2);
		
		uint8_t get_osrs_t_p = 0x74;
		I2C_SendData(I2C1, SlaveAddress, &get_osrs_t_p, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		uint8_t set_osrs_t[2] = {0x74, (0x54|(recData[0]&0x03))}; // osrs_t and osrs_p
		I2C_SendData(I2C1, SlaveAddress, set_osrs_t, 2);
		
		uint8_t set_gas_wait_0[2] = {0x64, 0x59};
		I2C_SendData(I2C1, SlaveAddress, set_gas_wait_0, 2);
		
		// Calculating res_heat_x for 300C
		uint8_t get_par_g1 = 0xED;
		I2C_SendData(I2C1, SlaveAddress, &get_par_g1, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_g1 = recData[0];
		
		uint8_t get_par_g2 = 0xEB;
		I2C_SendData(I2C1, SlaveAddress, &get_par_g2, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_g2 = recData[0] + (recData[1]<<8);
		
		uint8_t get_par_g3 = 0xEE;
		I2C_SendData(I2C1, SlaveAddress, &get_par_g2, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_g3 = recData[0];
		
		uint8_t get_res_heat_range = 0x02;
		I2C_SendData(I2C1, SlaveAddress, &get_res_heat_range, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double res_heat_range = ((recData[0]&0x30)>>4);
		
		uint8_t get_res_heat_val = 0x00;
		I2C_SendData(I2C1, SlaveAddress, &get_res_heat_val, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double res_heat_val = recData[0];
		
		double var1 = (par_g1 / 16.0) + 49.0;
		double var2 = ((par_g2 / 32768.0) * 0.0005) + 0.00235;
		double var3 = par_g3 / 1024.0;
		double var4 = var1 * (1.0 + (var2 * 300.0));
		double var5 = var4 + (var3 * 68.0);
		uint8_t res_heat_x = (uint8_t)(3.4 * ((var5 * (4.0 / (4.0 + res_heat_range)) * (1.0/(1.0+(res_heat_val * 0.002)))) - 25.0));
		
		uint8_t set_res_heat_0[2] = {0x5A, res_heat_x};
		I2C_SendData(I2C1, SlaveAddress, set_res_heat_0, 2);
		
		uint8_t read_nb_conv = 0x71;
		I2C_SendData(I2C1, SlaveAddress, &read_nb_conv, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		uint8_t nb_conv_write[2] = {0x71, ((recData[0]&0xF0)|0x20)};
		I2C_SendData(I2C1, SlaveAddress, &nb_conv_write, 2);
		
}

// Must add delay after taking measurement before reading values
void BME688_Take_Measurement() {
		uint8_t SlaveAddress = 0b1110110 << 1; // BME688 Address
		uint8_t recData[3];
	
		uint8_t read_mode = 0x74;
		I2C_SendData(I2C1, SlaveAddress, &read_mode, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1); 
		uint8_t sending = recData[0] & ~0x03;
		sending |= 0x01;
		uint8_t take_measurement[2] = {0x74, sending};
		I2C_SendData(I2C1, SlaveAddress, take_measurement, 2);
}

double BME688_Get_Temp_C() {
		uint8_t SlaveAddress = 0b1110110 << 1; // BME688 Address
		uint8_t recData[3];
		
		uint8_t getTemp = 0x22;
		I2C_SendData(I2C1, SlaveAddress, &getTemp, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 3);
		int t_adc = (recData[1]<<4) + (recData[0]<<12) + (recData[2]>>4);
		double adc = (recData[1]<<4) + (recData[0]<<12) + (recData[2]>>4);
		
		uint8_t get_par_t1 = 0xE9;
		I2C_SendData(I2C1, SlaveAddress, &get_par_t1, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		int par_t1 = recData[0] + (recData[1]<<8);
		double par_t1f = recData[0] + (recData[1]<<8);
		
		uint8_t get_par_t2 = 0x8A;
		I2C_SendData(I2C1, SlaveAddress, &get_par_t2, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		int par_t2 = recData[0] + (recData[1]<<8);
		double par_t2f = recData[0] + (recData[1]<<8);
		
		uint8_t get_par_t3 = 0x8C;
		I2C_SendData(I2C1, SlaveAddress, &get_par_t3, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		int par_t3 = recData[0];
		double par_t3f = recData[0];
		
		double var1f = ((adc / 16384.0) - ((par_t1f / 1024.0)))*par_t2f;
		double var2f = (((adc / 131072.0) - (par_t1f / 8192.0))*((adc / 131072.0) - (par_t1f / 8192.0)) * (par_t3*16.0));
		double t_finef = var1f + var2f;
		double final_temp = t_finef / 5120.0;
		
		return final_temp;
}

double BME688_Get_Humidity() {
		double temp_comp = BME688_Get_Temp_C();
	
		uint8_t SlaveAddress = 0b1110110 << 1; // BME688 Address
		uint8_t recData[3];
		
		uint8_t getHum = 0x25;
		I2C_SendData(I2C1, SlaveAddress, &getHum, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double h_adc = (recData[0]<<8) + recData[1];
		
		uint8_t get_par_h1 = 0xE2;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h1, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_h1 = (recData[0]&0x0F) + (recData[1]<<4);
		
		uint8_t get_par_h2 = 0xE1;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h2, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_h2 = (recData[0]<<4) + ((recData[1]&0xF0)>>4);
		
		uint8_t get_par_h3 = 0xE4;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h3, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_h3 = recData[0];
		
		uint8_t get_par_h4 = 0xE5;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h4, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_h4 = recData[0];
		
		uint8_t get_par_h5 = 0xE6;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h5, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_h5 = recData[0];
		
		uint8_t get_par_h6 = 0xE7;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h6, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_h6 = recData[0];
		
		uint8_t get_par_h7 = 0xE8;
		I2C_SendData(I2C1, SlaveAddress, &get_par_h7, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_h7 = recData[0];
		
		double var1 = h_adc - (par_h1*16.0) + ((par_h3/2.0)*temp_comp);
		double var2 = var1*((par_h2/262144.0)*(1.0+((par_h4 / 16384.0)*temp_comp)+((par_h5 / 1048576.0)*temp_comp*temp_comp)));
		double var3 = par_h6 / 16384.0;
		double var4 = par_h7 / 2097152.0;
		double hum_comp = var2 + ((var3 + (var4*temp_comp))*var2*var2);
		
		return hum_comp;
}

double BME688_Get_Pressure() {
		double temp_comp = BME688_Get_Temp_C();
		double t_fine = temp_comp * 6500.0;
	
		uint8_t SlaveAddress = 0b1110110 << 1; // BME688 Address
		uint8_t recData[3];
		
		uint8_t getPr = 0x1F;
		I2C_SendData(I2C1, SlaveAddress, &getPr, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 3);
		double p_adc = (recData[0]<<12) + (recData[1]<<4) + (recData[2]>>4);

		uint8_t get_par_p1 = 0x8E;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p1, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_p1 = (recData[1]<<8) + recData[0];
	
		uint8_t get_par_p2 = 0x90;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p2, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_p2 = (recData[1]<<8) + recData[0];
	
		uint8_t get_par_p3 = 0x92;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p3, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_p3 = recData[0];
		
		uint8_t get_par_p4 = 0x94;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p4, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_p4 = (recData[1]<<8) + recData[0];
		
		uint8_t get_par_p5 = 0x96;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p5, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_p5 = (recData[1]<<8) + recData[0];
		
		uint8_t get_par_p6 = 0x99;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p6, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_p6 = recData[0];
		
		uint8_t get_par_p7 = 0x98;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p7, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_p7 = recData[0];
		
		uint8_t get_par_p8 = 0x9C;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p8, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_p8 = (recData[1]<<8) + recData[0];

		uint8_t get_par_p9 = 0x9E;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p9, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double par_p9 = (recData[1]<<8) + recData[0];
		
		uint8_t get_par_p10 = 0xA0;
		I2C_SendData(I2C1, SlaveAddress, &get_par_p10, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double par_p10 = recData[0];
		
		double var1 = (t_fine / 2.0) - 64000.0;
		double var2 = var1 * var1 * (par_p6 / 131072.0);
		var2 = var2 + (var1 * par_p5 * 2.0);
		var2 = (var2 / 4.0) + (par_p4 * 65536.0);
		
		double var1_p = var1;
		//if (var1_p < 0) var1_p = var1_p*(-1);
		var1 = (((par_p3 * var1_p * var1) / 16384.0) + (par_p2 * var1_p)) / 524288.0;
		
		var1 = (1.0 + (var1 / 32768.0)) * par_p1;
		double press_comp = 1048576.0 - p_adc;
		
		double var2_p = var2;
		//if (var2_p < 0) var2_p = var2_p*(-1);
		press_comp = ((press_comp - (var2_p / 4096.0)) * 6250.0) / var1;
		
		var1 = (par_p9 * press_comp * press_comp) / 2147483648.0;
		var2 = press_comp * (par_p8 / 32768.0);
		double var3 = (press_comp / 256.0) * (press_comp / 256.0) * (press_comp / 256.0) * (par_p10 / 131072.0);
		
		var1_p = var1;
		//if (var1_p < 0) var1_p = var1_p*(-1);
		var2_p = var2;
		//if (var2_p < 0) var2_p = var2_p*(-1);
		double var3_p = var3;
		//if (var3_p < 0) var3_p = var3_p*(-1);
		press_comp = press_comp + (var1_p + var2_p + var3_p + (par_p7 * 128.0)) / 16.0;
		
		double prelim_pressure = press_comp - 101325.0;
		press_comp = 101325.0 + (prelim_pressure / 10.0);
		return press_comp;
}

double BME688_Get_Gas() {
		uint8_t SlaveAddress = 0b1110110 << 1; // BME688 Address
		uint8_t recData[2];
	
		uint8_t check = 0x2D;
		I2C_SendData(I2C1, SlaveAddress, &check, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		//printf("%u\n", (recData[0]&0x30));	// Gas status
		uint8_t get_gas_range = 0x2D;
		I2C_SendData(I2C1, SlaveAddress, &get_gas_range, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 1);
		double gas_range = (recData[0]&0x0F);
	
		uint8_t get_gas_adc = 0x2C;
		I2C_SendData(I2C1, SlaveAddress, &get_gas_adc, 1);
		I2C_ReceiveData(I2C1, SlaveAddress, recData, 2);
		double gas_adc = (recData[0]<<2) + ((recData[1]&0xC0)>>6);
	
		uint32_t var1 = UINT32_C(262144) >> (int)gas_range;
		int32_t var2 = (int32_t)gas_adc - INT32_C(512);
		var2 *= INT32_C(3);
		var2 = INT32_C(4096) + var2;
		double gas_res = 1000000.0f * (float)var1 / (float)var2;
	
		return gas_res;
}

double BME688_Calc_AQI(double gas_r) {
	double aqi;
	double difference;
	
	if (gas_r >= 18000.0) {
		difference = gas_r - 18000.0;
		aqi = 10 - (difference / 200);
	} else if (gas_r >= 13000) {
		difference = gas_r - 13000.0;
		aqi = 50 - (difference / 125);
	} else if (gas_r >= 10000.0) {
		difference = gas_r - 10000.0;
		aqi = 100 - (difference / 60);
	} else if (gas_r >= 7500) {
		difference = gas_r - 7500.0;
		aqi = 150 - (difference / 50);
	} else if (gas_r >= 5000.0) {
		difference = gas_r - 5000.0;
		aqi = 200 - (difference / 50);
	} else if (gas_r >= 2500.0) {
		difference = gas_r - 2500.0;
		aqi = 300 - (difference / 25);
	} else if (gas_r >= 0) {
		aqi = 500 - gas_r /12.5;
	}
	
	return aqi;
}