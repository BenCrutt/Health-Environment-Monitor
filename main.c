/*
 * ECE 153B - Winter 2023
 *
 * Name(s):
 * Section:
 * Lab: 4B
 */

#include "Common.h"
#include "stm32l476xx.h"
#include "I2C.h"
#include "SysClock.h"
#include "UART.h"
#include "SysTimer.h"
#include "LED.h"
#include "TM1637.h"
#include "HT16K33.h"
#include "LTR390.h"
#include "MAX30205.h"
#include "BME688.h"
#include "EXTI.h"
#include "ADC.h"
#include <string.h>
#include <stdio.h>

volatile int MODE;
int warnings;

#define TCA_Address 0xEE
#define HTK_Bar_Addr 0xE6
#define HTK_Address_0 0xE0
#define HTK_Address_1 0xE2
#define HTK_Address_2 0xE4
#define HTK_Address_3 0xE8
static const uint8_t desel = 0x00;
static const uint8_t sel_0 = 0x01;
static const uint8_t sel_1 = 0x02;
static const uint8_t sel_2 = 0x04;
static const uint8_t sel_3 = 0x08;
static const uint8_t sel_4 = 0x10;
static const uint8_t sel_5 = 0x20;
static const uint8_t sel_6 = 0x40;
static const uint8_t sel_7 = 0x80;

static const uint8_t weather_mode_display[17] = {0x00, 0x00, 0xFF, 0x7E, 0x81, 0x7E, 0x81, 0x66, 0x99, 0x66, 0x99, 0x66, 0x99, 0x66, 0x99, 0x00, 0xFF};
static const uint8_t health_mode_display[17] = {0x00, 0xFF, 0x00, 0x81, 0x7E, 0x81, 0x7E, 0xE7, 0x18, 0xE7, 0x18, 0x81, 0x7E, 0x81, 0x7E, 0xFF, 0x00};
static const uint8_t estimation_mode_display[17] = {0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xFF, 0xDB, 0xFF, 0xDB, 0xFF, 0xDB, 0xFF, 0x00};

static double BME688_Temp_C;
static double BME688_Humidity;
static double BME688_Pressure;
static double BME688_Gas;
static double AQI;

static double estimate_temp;
static double estimate_aqi;
static double estimate_humidity;
static double estimate_pressure;
static double estimate_lux;
static double estimate_uvi;

void EXTI4_Init(void);
void EXTI4_IRQHandler(void);
void Init_USARTx(int x);

void EXTI4_Init(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	// Initialize User Button
	GPIOC->MODER &= ~GPIO_MODER_MODE4; // Clear mode bits setting mode to input
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD4; // No Pull-up, No pull-down

	// Configure SYSCFG EXTI
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI4;
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PC;

	// Configure EXTI Trigger
	EXTI->RTSR1 |= EXTI_RTSR1_RT4;
	// EXTI->FTSR1 |= EXTI_FTSR1_FT4;

	// Enable EXTI
	EXTI->IMR1 |= EXTI_IMR1_IM4;

	// Configure and Enable in NVIC
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_SetPriority(EXTI4_IRQn, 0);
}

// [TODO] Write Interrupt Handlers (look in startup_stm32l476xx.s to find the
// interrupt handler names that you should use)

void EXTI4_IRQHandler(void)
{

	int i = 0;
	int pressed = 0;
	for (int j = 0; j < 10000; j++)
		;
	if ((GPIOC->IDR & GPIO_IDR_ID4) == GPIO_IDR_ID4)
		pressed = 1;

	while ((GPIOC->IDR & GPIO_IDR_ID4) == GPIO_IDR_ID4)
		;

	if (pressed == 1)
	{
		if (MODE == 1)
			MODE = 2;
		else if (MODE == 2)
			MODE = 2;
		else
			MODE = 1;
	}

	// Clear interrupt pending bit
	EXTI->PR1 |= EXTI_PR1_PIF4;
}

// Initializes USARTx
// USART2: UART Communication with Termite
// USART1: Bluetooth Communication with Phone
void Init_USARTx(int x)
{
	if (x == 1)
	{
		UART1_Init();
		UART1_GPIO_Init();
		USART_Init(USART1);
	}
	else if (x == 2)
	{
		UART2_Init();
		UART2_GPIO_Init();
		USART_Init(USART2);
	}
	else
	{
		// Do nothing...
	}
}

int main(void)
{
	int i;

	uint32_t adcOUT;

	MODE = 0;

	System_Clock_Init(); // System Clock = 80 MHz

	// Initialize I2C
	I2C_GPIO_Init();
	I2C_Initialization();
	EXTI4_Init();

	// Initialize UART -- change the argument depending on the part you are working on
	Init_USARTx(2);

	SysTick_Init();
	LED_Init();
	// TM1637_Init();

	// BME688
	BME688_Init();
	LTR390_Init();
	ADC_Init();

	// TMP117
	uint8_t data[17] = {0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	// HTK_Init(HTK_Address);

	// HTK_Clear_Display(HTK_Address);
	int ALS;
	double Lux;
	int bar;
	int UVS;
	double UVI;
	// HTK_Set_Bar_Green(1, HTK_Address);

	HTK_Init(HTK_Address_0);
	HTK_Clear_Display(HTK_Address_0);
	HTK_Init(HTK_Address_1);
	HTK_Clear_Display(HTK_Address_1);
	HTK_Init(HTK_Address_2);
	HTK_Clear_Display(HTK_Address_2);
	HTK_Init(HTK_Address_3);
	HTK_Clear_Display(HTK_Address_3);
	int vals[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int values[32] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	int output;
	uint32_t max_adc = 0;
	uint32_t curr_max = 0;
	double hr = 0;
	double hr_period;
	int avg_count = 0;
	double avg = 0;

	uint8_t data16[17];
	data16[0] = 0x00;

	TM1637_Init();
	// HTK_Init(0b1110011 << 1);

	for (i = 0; i < 8; i++)
	{
		uint8_t sel = 1 << i;
		I2C_SendData(I2C1, TCA_Address, &sel, 1);
		HTK_Init(HTK_Bar_Addr);
		HTK_Clear_Display(HTK_Bar_Addr);
	}
	delay(10);

	// I2C_SendData(I2C1, TCA_Address_1, &sel_2, 1);

	// I2C_SendData(I2C1, TCA_Address_1, &desel, 1);
	// HTK_Init(HTK_Bar_Addr);
	// HTK_Clear_Display(HTK_Bar_Addr);
	int prev_bar_Lux;
	int prev_bar_UVI;
	int prev_bar_hr;
	int prev_bar_temp;

	int temp_wait = 9;
	double MAX30205_Temp_C;
	printf("\n");
	while (1)
	{
		// Determine Slave Address
		//
		// Note the "<< 1" must be present because bit 0 is treated as a don't care in 7-bit addressing mode
		if (MODE == 0)
		{
			printf("ENTERING CONDITION MODE\n\n");
			I2C_SendData(I2C1, TCA_Address, &sel_4, 1);
			HTK_SetData(weather_mode_display, 17, HTK_Bar_Addr);

			for (int l = 0; l < 32; l++)
				values[l] = -1;

			for (int j = 0; j < 32; j++) {
				if (j < 8)
					HTK_8x16_Set_Bit(HTK_Address_3, values[j], (j & 0x7) + 1);
				else if (j < 16)
					HTK_8x16_Set_Bit(HTK_Address_2, values[j], (j & 0x7) + 1);
				else if (j < 24)
					HTK_8x16_Set_Bit(HTK_Address_1, values[j], (j & 0x7) + 1);
				else if (j < 32)
					HTK_8x16_Set_Bit(HTK_Address_0, values[j], (j & 0x7) + 1);
			}

			prev_bar_Lux = 1;
			prev_bar_UVI = 1;

			I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			HTK_Set_Bar_Green(1, HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			HTK_Set_Bar_Green(1, HTK_Bar_Addr);

			while (MODE == 0)
			{
				BME688_Take_Measurement();
				LTR390_Init_ALS();

				delay(250);

				ALS = LTR390_Get_ALS_Data();
				Lux = LTR390_Calc_Lux(ALS);
				LTR390_Init_UVS();

				delay(250);

				UVS = LTR390_Get_UVS_Data();
				UVI = LTR390_Calc_UVI(UVS);

				BME688_Temp_C = BME688_Get_Temp_C();
				BME688_Humidity = BME688_Get_Humidity();
				BME688_Pressure = BME688_Get_Pressure();
				BME688_Gas = BME688_Get_Gas();

				AQI = BME688_Calc_AQI(BME688_Gas);

				WriteNumTM((int)BME688_Pressure / 100);

				I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
				prev_bar_Lux = HTK_Color_Bars_Lux(Lux, prev_bar_Lux, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_2, 1);
				HTK_Write_Temp_SevenSeg(BME688_Temp_C, 1, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_3, 1);
				HTK_Write_Humidity_SevenSeg(BME688_Humidity, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
				HTK_Write_AQI(AQI, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
				prev_bar_UVI = HTK_Color_Bars_UVI(UVI, prev_bar_UVI, HTK_Bar_Addr);

				HTK_8x16_Scroll_Values(&values, 16 - (int)AQI / 13, 32);

				for (int j = 0; j < 32; j++)
				{
					if (j < 8)
						HTK_8x16_Set_Bit(HTK_Address_3, values[j], j + 1);
					else if (j < 16)
						HTK_8x16_Set_Bit(HTK_Address_2, values[j], j - 7);
					else if (j < 24)
						HTK_8x16_Set_Bit(HTK_Address_1, values[j], j - 15);
					else if (j < 32)
						HTK_8x16_Set_Bit(HTK_Address_0, values[j], j - 23);
				}

				I2C_SendData(I2C1, TCA_Address, &sel_6, 1);
				ScrollValues(&vals, (((int)(BME688_Temp_C * 10.0)) % 8) + 1);
				for (int j = 0; j < 16; j++)
				{
					SetBit(j, vals[j], &data16);
				}
				HTK_SetData(data16, 17, HTK_Bar_Addr);

				// I2C_SendData(I2C1, TCA_Address, &sel_5, 1);
				// HTK_SetData(data16, 17, HTK_Bar_Addr);
			}
		}
		else if (MODE == 1)
		{
			printf("ENTERING HEALTH MODE\n\n");
			
			I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_2, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_3, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_6, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_5, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_4, 1);
			HTK_SetData(health_mode_display, 17, HTK_Bar_Addr);

			WriteNumTM(0);

			I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
			HTK_Write_AQI(hr, HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
			HTK_Set_Bar_Red(1, HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
			HTK_Set_Bar_Red(1, HTK_Bar_Addr);

			prev_bar_hr = 1;
			prev_bar_temp = 1;

			while (MODE == 1)
			{
				for (int x = 0; x < 30000; x++)
				{
					// Trigger ADC
					ADC1->CR |= ADC_CR_ADSTART;

					// Wait for end of regular conversion of master ADC
					while ((ADC123_COMMON->CSR & ADC_CSR_EOC_MST) == 0)
						;

					// Get result
					adcOUT = ADC1->DR;

					if (adcOUT > max_adc)
						max_adc = adcOUT;

					output = (max_adc / 30) - 87;
					if (output < 1)
						output = 1;
					if (output > 16)
						output = 16;

					if (output > 11)
					{
						if (msTicks - curr_max > 300)
						{
							hr_period = ((double)msTicks - (double)curr_max) / 1000.0;
							hr = (60.0 / hr_period);
							avg += hr;
							avg_count += 1;
							if (avg_count == 5)
							{
								avg = avg / 5.0;
								I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
								HTK_Write_AQI(avg, HTK_Bar_Addr);
								I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
								prev_bar_hr = HTK_Color_Bars_HR(avg, prev_bar_hr, HTK_Bar_Addr);
								avg = 0;
								avg_count = 0;
							}
							curr_max = msTicks;
						}
					}
				}

				if (temp_wait == 10)
				{
					temp_wait = 0;
					MAX30205_Temp_C = MAX30205_Get_Temp();
					I2C_SendData(I2C1, TCA_Address, &sel_2, 1);
					HTK_Write_Temp_SevenSeg((MAX30205_Temp_C * 1.8) + 32.0, 0, HTK_Bar_Addr);

					I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
					prev_bar_temp = HTK_Color_Bars_BT((int)(MAX30205_Temp_C * 1.8) + 32, prev_bar_temp, HTK_Bar_Addr);

					I2C_SendData(I2C1, TCA_Address, &sel_6, 1);
					ScrollValues(&vals, ((int)(MAX30205_Temp_C * 1.8) + 32 - 70) / 5 + 1);
					for (int j = 0; j < 16; j++)
					{
						SetBit(j, vals[j], &data16);
					}
					HTK_SetData(data16, 17, HTK_Bar_Addr);
				}
				else
				{
					temp_wait += 1;
				}

				HTK_8x16_Scroll_Values(&values, output, 32);
				for (int j = 0; j < 32; j++)
				{
					if (j < 8)
						HTK_8x16_Set_Bit(HTK_Address_3, values[j], j + 1);
					else if (j < 16)
						HTK_8x16_Set_Bit(HTK_Address_2, values[j], j - 7);
					else if (j < 24)
						HTK_8x16_Set_Bit(HTK_Address_1, values[j], j - 15);
					else if (j < 32)
						HTK_8x16_Set_Bit(HTK_Address_0, values[j], j - 23);
				}

				max_adc = 0;
			}
		}
		else if (MODE == 2)
		{
			warnings = 0;
			printf("ENTERING ESTIMATION MODE\n");
			printf("Heart rate and body temperature analysis beginning shortly. \nPlease attach electrodes and thermometer.\n\n");
			I2C_SendData(I2C1, TCA_Address, &sel_4, 1);
			HTK_SetData(estimation_mode_display, 17, HTK_Bar_Addr);

			WriteNumTM(0);

			I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_2, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_3, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_6, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_5, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
			HTK_Clear_Display(HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_3, 1);

			// Give some time before collecting heart rate
			for (int i = 3; i >= 0; i--)
			{
				delay(1000);
				HTK_WriteNum_SevenSeg(i, HTK_Bar_Addr); // Countdown
			}
			delay(1000);

			// Show ECG for 6 HR collections, average last 4 of them (~30s)
			I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
			HTK_Write_AQI(hr, HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
			HTK_Set_Bar_Red(1, HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
			HTK_Set_Bar_Red(1, HTK_Bar_Addr);

			prev_bar_hr = 1;
			prev_bar_temp = 1;
			int reading_hr = 1;
			double hr_average = 0;
			int hr_avg_count = 0;

			while (reading_hr == 1)
			{
				for (int x = 0; x < 30000; x++)
				{
					// Trigger ADC
					ADC1->CR |= ADC_CR_ADSTART;

					// Wait for end of regular conversion of master ADC
					while ((ADC123_COMMON->CSR & ADC_CSR_EOC_MST) == 0)
						;

					// Get result
					adcOUT = ADC1->DR;

					if (adcOUT > max_adc)
						max_adc = adcOUT;

					output = (max_adc / 30) - 87;
					if (output < 1)
						output = 1;
					if (output > 16)
						output = 16;

					if (output > 11)
					{
						if (msTicks - curr_max > 300)
						{
							hr_period = ((double)msTicks - (double)curr_max) / 1000.0;
							hr = (60.0 / hr_period);
							avg += hr;
							avg_count += 1;
							if (avg_count == 5)
							{
								avg = avg / 5.0;
								I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
								HTK_Write_AQI(avg, HTK_Bar_Addr);
								I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
								prev_bar_hr = HTK_Color_Bars_HR(avg, prev_bar_hr, HTK_Bar_Addr);

								hr_avg_count += 1;
								if (hr_avg_count >= 2)
								{
									hr_average += avg;
								}
								if (hr_avg_count == 6)
								{
									reading_hr = 0;
								}

								avg = 0;
								avg_count = 0;
							}
							curr_max = msTicks;
						}
					}
				}

				if (temp_wait == 10)
				{
					temp_wait = 0;
					MAX30205_Temp_C = MAX30205_Get_Temp();
					I2C_SendData(I2C1, TCA_Address, &sel_2, 1);
					HTK_Write_Temp_SevenSeg((MAX30205_Temp_C * 1.8) + 32.0, 0, HTK_Bar_Addr);

					I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
					prev_bar_temp = HTK_Color_Bars_BT((int)(MAX30205_Temp_C * 1.8) + 32, prev_bar_temp, HTK_Bar_Addr);

					I2C_SendData(I2C1, TCA_Address, &sel_6, 1);
					ScrollValues(&vals, ((int)(MAX30205_Temp_C * 1.8) + 32 - 70) / 5 + 1);
					for (int j = 0; j < 16; j++)
					{
						SetBit(j, vals[j], &data16);
					}
					HTK_SetData(data16, 17, HTK_Bar_Addr);
				}
				else
				{
					temp_wait += 1;
				}

				HTK_8x16_Scroll_Values(&values, output, 32);
				for (int j = 0; j < 32; j++)
				{
					if (j < 8)
						HTK_8x16_Set_Bit(HTK_Address_3, values[j], j + 1);
					else if (j < 16)
						HTK_8x16_Set_Bit(HTK_Address_2, values[j], j - 7);
					else if (j < 24)
						HTK_8x16_Set_Bit(HTK_Address_1, values[j], j - 15);
					else if (j < 32)
						HTK_8x16_Set_Bit(HTK_Address_0, values[j], j - 23);
				}

				max_adc = 0;
			}

			hr_average = hr_average / 5.0;
			I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
			HTK_Write_AQI(hr_average, HTK_Bar_Addr);
			printf("Average heart rate: %f BPM\n", hr_average);
			printf("Body temperature: %f degrees Farenheit\n", (MAX30205_Temp_C * 1.8) + 32.0);

				printf("Weather data analysis beginning\n\n");
			I2C_SendData(I2C1, TCA_Address, &sel_3, 1);
			// Give some time before collecting heart rate
			for (int i = 3; i >= 0; i--)
			{
				delay(1000);
				HTK_WriteNum_SevenSeg(i, HTK_Bar_Addr); // Countdown
			}
			// MAX30205_Temp_C has body temp, is already printed

			// Allow condition data to stabilize for 20 seconds, then collect data (20s)
			for (int l = 0; l < 32; l++)
				values[l] = -1;

			for (int j = 0; j < 32; j++)
			{
				if (j < 8)
					HTK_8x16_Set_Bit(HTK_Address_3, values[j], j + 1);
				else if (j < 16)
					HTK_8x16_Set_Bit(HTK_Address_2, values[j], j - 7);
				else if (j < 24)
					HTK_8x16_Set_Bit(HTK_Address_1, values[j], j - 15);
				else if (j < 32)
					HTK_8x16_Set_Bit(HTK_Address_0, values[j], j - 23);
			}

			prev_bar_Lux = 1;
			prev_bar_UVI = 1;

			I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			HTK_Set_Bar_Green(1, HTK_Bar_Addr);

			I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
			HTK_Clear_Display(HTK_Bar_Addr);
			HTK_Set_Bar_Green(1, HTK_Bar_Addr);

			for (int i = 0; i < 40; i++)
			{
				BME688_Take_Measurement();
				LTR390_Init_ALS();

				delay(150);

				ALS = LTR390_Get_ALS_Data();
				Lux = LTR390_Calc_Lux(ALS);
				LTR390_Init_UVS();

				delay(150);

				UVS = LTR390_Get_UVS_Data();
				UVI = LTR390_Calc_UVI(UVS);

				BME688_Temp_C = BME688_Get_Temp_C();
				BME688_Humidity = BME688_Get_Humidity();
				BME688_Pressure = BME688_Get_Pressure();
				BME688_Gas = BME688_Get_Gas();

				AQI = BME688_Calc_AQI(BME688_Gas);

				WriteNumTM((int)BME688_Pressure / 100);

				I2C_SendData(I2C1, TCA_Address, &sel_1, 1);
				prev_bar_Lux = HTK_Color_Bars_Lux(Lux, prev_bar_Lux, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_2, 1);
				HTK_Write_Temp_SevenSeg(BME688_Temp_C, 1, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_3, 1);
				HTK_Write_Humidity_SevenSeg(BME688_Humidity, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_7, 1);
				HTK_Write_AQI(AQI, HTK_Bar_Addr);

				I2C_SendData(I2C1, TCA_Address, &sel_0, 1);
				prev_bar_UVI = HTK_Color_Bars_UVI(UVI, prev_bar_UVI, HTK_Bar_Addr);

				HTK_8x16_Scroll_Values(&values, 16 - (int)AQI / 13, 32);

				for (int j = 0; j < 32; j++)
				{
					if (j < 8)
						HTK_8x16_Set_Bit(HTK_Address_3, values[j], j + 1);
					else if (j < 16)
						HTK_8x16_Set_Bit(HTK_Address_2, values[j], j - 7);
					else if (j < 24)
						HTK_8x16_Set_Bit(HTK_Address_1, values[j], j - 15);
					else if (j < 32)
						HTK_8x16_Set_Bit(HTK_Address_0, values[j], j - 23);
				}

				I2C_SendData(I2C1, TCA_Address, &sel_6, 1);
				ScrollValues(&vals, (((int)(BME688_Temp_C * 10.0)) % 8) + 1);
				for (int j = 0; j < 16; j++)
				{
					SetBit(j, vals[j], &data16);
				}
				HTK_SetData(data16, 17, HTK_Bar_Addr);
			}

			// Store values for later analysis and output
			estimate_temp = BME688_Temp_C;
			estimate_aqi = AQI;
			estimate_humidity = BME688_Humidity;
			estimate_pressure = BME688_Pressure;
			estimate_lux = Lux;
			estimate_uvi = UVI;
			
			printf("Air temperature: %f degrees Celcius\n", estimate_temp);
			printf("Relative humidity: %f%%\n", estimate_humidity);
			printf("Barometric pressure: %f hPa\n", estimate_pressure);
			printf("Air Quality Index: %f...", estimate_aqi);
			if (estimate_aqi <= 50) {
				printf("Air quality today is excellent\n");
			} else if (estimate_aqi <= 100) {
				printf("Air quality today is good\n");
			}
			printf("Ambient light intensity: %f Lux\n", estimate_lux);
			printf("UV Index: %f\n\n", estimate_uvi);

			// Calculate warnings
			printf("WARNINGS:\n");
			
			// Body temp
			if (((MAX30205_Temp_C * 1.8) + 32.0) >= 100.0) {
				printf("HIGH TEMPERATURE ALERT: Possible fever detected\n");
				warnings += 1;
			}
			
			// HR
			if (hr_average >= 180) {
				printf("HEART RATE ALERT: High heart rate detected\n");
				warnings += 1;
			} else if (hr_average <= 45) {
				printf("HEART RATE ALERT: Low heart rate detected\n");
				warnings += 1;
			}
			
			// Air temp
			if (estimate_temp >= 30) {
				printf("HIGH TEMPERATURE ALERT: Exercise caution during activities outdoor\n");
				warnings += 1;
			} else if (estimate_temp <= 10) {
				printf("LOW TEMPERATURE ALERT: Wear a jacket!\n");
			}
			
			// Humidity
			if (estimate_humidity <= 20) {
				printf("LOW HUMIDITY ALERT: Stay hydrated!\n");
				warnings += 1;
			}
			
			// Pressure
			if (estimate_pressure <= 95000) {
				printf("LOW PRESSURE ALERT: Possible inclement weather detected\n");
				warnings += 1;
			}
			
			// Air quality:
			if (estimate_aqi >= 200) {
				printf("AQI ALERT: Air quality today is poor; mask use highly recommended\n");
				warnings += 1;
			} else if (estimate_aqi >= 100) {
				printf("AQI ALERT: Air quality today is mediocre; mask use recommended\n");
				warnings += 1;
			}
			
			// Lux
			if (Lux >= 10000) {
				printf("BRIGHTNESS ALERT: Sunglasses highly recommended\n");
				warnings += 1;
			}			
			
			// UVI
			if (estimate_uvi >= 4) {
				printf("UVI ALERT: Sunscreen highly recommended\n\n");
				warnings += 1;
			}
			
			if (warnings >= 1) {
				printf("Total warnings: %d\n\n", warnings);	
			} else {
				printf("No warnings generated!\n\n");
			}

			// Flash MODE light for 5 seconds to indicate Termite output
			I2C_SendData(I2C1, TCA_Address, &sel_4, 1);
			HTK_Blink(HTK_Bar_Addr);
			delay(6000);
			HTK_Init(HTK_Bar_Addr);

			// Go to condition mode
			MODE = 0;
		}
	}
}
