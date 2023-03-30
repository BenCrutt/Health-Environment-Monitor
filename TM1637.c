#include "TM1637.h"
#include "SysTimer.h"
#include "LED.h"

#define ms_delay 1

void TM1637_Init(void) { // A0 = CLK, A1 = DIO
	// Enable GPIOA Clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// Configure pins A0 and A1 as output mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1); // Clear bits
	GPIOA->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0); // Set to 0b01
	
	// Set these 2 pins to Fast Speed
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED1); // Clear bits
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0_1 | GPIO_OSPEEDR_OSPEED1_1); // Set to 0b10
	
	// Set output type of pins to push-pull
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);
	
	// Set pins to no pull-up, no pull-down
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1);
	
	// Set both high to begin
	GPIOA->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1);
}

void Set_DIO_Input(void) {
	// Initialize User Button
  GPIOA->MODER &= ~GPIO_MODER_MODE1; // Clear mode bits setting mode to input
}

void Set_DIO_Output(void) {	
	// Configure pin A1 as output mode
	GPIOA->MODER &= ~GPIO_MODER_MODE1; // Clear bits
	GPIOA->MODER |= GPIO_MODER_MODE1_0; // Set to 0b01
}

void Set_CLK_Input(void) {
	// Set PA0 to input mode
  GPIOA->MODER &= ~GPIO_MODER_MODE0; // Clear mode bits setting mode to input
}

void Set_CLK_Output(void) {
	// Configure pin A0 as output mode
	GPIOA->MODER &= ~GPIO_MODER_MODE0; // Clear bits
	GPIOA->MODER |= GPIO_MODER_MODE0_0; // Set to 0b01
}

// Drive DIO H->L while CLK is HIGH
void StartTM(void) {
	// CLK is high, driving data low
	Set_DIO_Output();
	
	// Bit delay
	delay(ms_delay);
}

void StopTM(void) {
	// Set DIO low
	Set_DIO_Output();
	delay(ms_delay); // Bit delay
	
	// Set CLK high
	Set_CLK_Input();
	delay(ms_delay); // Bit delay
	
	// DIO low -> high while CLK is high
	Set_DIO_Input();
	delay(ms_delay); // Bit delay
}

void WriteByte(uint8_t word) {
	uint8_t data = word;
	
	// 8 bits per word of data, LSB->MSB(?)
	for (int i = 0; i < 8; i++) {
		// Set CLK low
		Set_CLK_Output();
		delay(ms_delay); // Bit delay
		
		// Set data word bit i
		if (data & 0x01) {
			Set_DIO_Input();
		} else {
			Set_DIO_Output();
		}
		
		delay(ms_delay); // Bit delay
		
		// Set CLK high (transmit bit)
		Set_CLK_Input();
		delay(ms_delay); // Bit delay
		
		// Move on to next bit of data
		data = data >> 1;
	}
	
	// Wait for receipt of ACK
	// Set CLK low while waiting
	Set_CLK_Output();
	Set_DIO_Input(); // Receive ACK input from device
	delay(ms_delay); // Bit delay
	
	// Bring CLK high
	Set_CLK_Input();
	delay(ms_delay); // Bit delay
	
	// Read ACK
	uint32_t ack = GPIOA->IDR;
	GPIOA->IDR |= ack;
	if ((GPIOA->IDR & GPIO_IDR_ID1) == 0) {
		Set_DIO_Output(); // ACK complete
		Green_LED_Off(); // ?????
	} else {
		Set_DIO_Output();
		Green_LED_Off(); // ?????
	}
	
	delay(ms_delay); // Bit delay
	
	// Set CLK low (done with data transmission)
	Set_CLK_Output();
	delay(ms_delay); // Bit delay
}

void WriteNumTM(int num) {
	StartTM();
	WriteByte(0x40);
	StopTM();
	
	StartTM();
	WriteByte(0xC0); // Address of leftmost digit
	
	if (num >= 1000)
		WriteByte(digits[((num/1000)%10)]);
	else
		WriteByte(0x00);
	
	if (num >= 100)
		WriteByte(digits[(num/100)%10]);
	else
		WriteByte(0x00);
	
	if (num >= 10)
		WriteByte(digits[(num/10)%10]);
	else 
		WriteByte(0x00);
	
	if (num >= 1)
		WriteByte(digits[num%10]);
	else 
		WriteByte(0x00);
	
	StopTM();
	
	StartTM();
	WriteByte(0x8A);
	StopTM();
}
