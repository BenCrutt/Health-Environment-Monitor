#include "I2C.h"

extern void Error_Handler(void);

// Inter-integrated Circuit Interface (I2C)
// up to 100 Kbit/s in the standard mode, 
// up to 400 Kbit/s in the fast mode, and 
// up to 3.4 Mbit/s in the high-speed mode.

// Recommended external pull-up resistance is 
// 4.7 kOmh for low speed, 
// 3.0 kOmh for the standard mode, and 
// 1.0 kOmh for the fast mode
	
//===============================================================================
//                        I2C GPIO Initialization
//===============================================================================
void I2C_GPIO_Init(void) {
	// I2C1_SCL = PB6
	// I2C1_SDA = PB7
	
	// Enable GPIO B Clk
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
	// Reset bits and set PB6 and PB7 to AF mode
	GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOB->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);

	// PB6 and PB7 to I2C1 SCL and SDA, respectively
	// Both using AF4 = 3'b100
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL6_2 | GPIO_AFRL_AFSEL7_2);
	
	// Set PB6 and PB7 to Very High Output Speed
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
	
	// Set output type of PB6 adn PB7 to Open-Drain
	GPIOB->OTYPER |= (GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);
	
	// Set Pull-Up/Down for PB6 and PB7 to Pull-Up
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0);
	
	
}
	
#define I2C_TIMINGR_PRESC_POS	28
#define I2C_TIMINGR_SCLDEL_POS	20
#define I2C_TIMINGR_SDADEL_POS	16
#define I2C_TIMINGR_SCLH_POS	8
#define I2C_TIMINGR_SCLL_POS	0

//===============================================================================
//                          I2C Initialization
//===============================================================================
void I2C_Initialization(void){
	uint32_t OwnAddr = 0x52;
	
	// Enable clock for I2C1
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;
	
	// Set I2C1 CLK source as system CLK
	RCC->CCIPR &= ~(RCC_CCIPR_I2C1SEL);
	RCC->CCIPR |= RCC_CCIPR_I2C1SEL_0;
	
	// Reset and enable I2C1
	RCC->APB1RSTR1 |= RCC_APB1RSTR1_I2C1RST;
	RCC->APB1RSTR1 &= ~(RCC_APB1RSTR1_I2C1RST);
	
	// Disable I2C before modifying
	I2C1->CR1 &= ~(I2C_CR1_PE);
	
	// Enable analog noise filter
	I2C1->CR1 &= ~(I2C_CR1_ANFOFF);
	
	// Disable digital noise filter
	I2C1->CR1 &= ~(I2C_CR1_DNF);
	
	// Enable error interrupts
	I2C1->CR1 |= I2C_CR1_ERRIE;
	
	// Enable clock stretching
	I2C1->CR1 &= ~(I2C_CR1_NOSTRETCH);
	
	// Set the master to operate in 7-bit addressing mode
	I2C1->CR2 &= ~(I2C_CR2_ADD10);
	
	// Enable automatic end mode
	I2C1->CR2 |= I2C_CR2_AUTOEND;
	
	// Enable NACK generation
	I2C1->CR2 |= I2C_CR2_NACK;
	
	// PSC = 15 -> CLK = 5 MHz
	I2C1->TIMINGR |= I2C_TIMINGR_PRESC;
	I2C1->TIMINGR &= ~(1UL<<31);
	
	// SCLDEL = SDADEL = 15
	I2C1->TIMINGR |= (I2C_TIMINGR_SCLDEL | I2C_TIMINGR_SDADEL);
	
	// SCLH = SCLL = 50
	I2C1->TIMINGR &= ~(I2C_TIMINGR_SCLH | I2C_TIMINGR_SCLL);
	I2C1->TIMINGR |= (50UL << 8);
	I2C1->TIMINGR |= (50UL);
	
	// Disable own address 1
	I2C1->OAR1 &= ~(I2C_OAR1_OA1EN);
	
	// Set to 7 bit mode
	I2C1->OAR1 &= ~(I2C_OAR1_OA1MODE);
	
	// Write OwnAdr
	I2C1->OAR1 &= ~(0xFFU);
	I2C1->OAR1 |= OwnAddr;
	
	// Enable own address 1
	I2C1->OAR1 |= I2C_OAR1_OA1EN;
	
	// 
	I2C1->CR1 |= I2C_CR1_PE;
}

//===============================================================================
//                           I2C Start
// Master generates START condition:
//    -- Slave address: 7 bits
//    -- Automatically generate a STOP condition after all bytes have been transmitted 
// Direction = 0: Master requests a write transfer
// Direction = 1: Master requests a read transfer
//=============================================================================== 
int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction) {	
	
	// Direction = 0: Master requests a write transfer
	// Direction = 1: Master requests a read transfer
	
	uint32_t tmpreg = 0;
	
	// This bit is set by software, and cleared by hardware after the Start followed by the address
	// sequence is sent, by an arbitration loss, by a timeout error detection, or when PE = 0.
	tmpreg = I2Cx->CR2;
	
	tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
	
	if (Direction == READ_FROM_SLAVE)
		tmpreg |= I2C_CR2_RD_WRN;  // Read from Slave
	else
		tmpreg &= ~I2C_CR2_RD_WRN; // Write to Slave
		
	tmpreg |= (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << 16 ) & I2C_CR2_NBYTES));
	
	tmpreg |= I2C_CR2_START;
	
	I2Cx->CR2 = tmpreg; 
	
   	return 0;  // Success
}

//===============================================================================
//                           I2C Stop
//=============================================================================== 
void I2C_Stop(I2C_TypeDef * I2Cx){
	// Master: Generate STOP bit after the current byte has been transferred 
	I2Cx->CR2 |= I2C_CR2_STOP;								
	// Wait until STOPF flag is reset
	while( (I2Cx->ISR & I2C_ISR_STOPF) == 0 ); 
}

//===============================================================================
//                           Wait for the bus is ready
//=============================================================================== 
void I2C_WaitLineIdle(I2C_TypeDef * I2Cx){
	// Wait until I2C bus is ready
	while( (I2Cx->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY );	// If busy, wait
}

//===============================================================================
//                           I2C Send Data
//=============================================================================== 
int8_t I2C_SendData(I2C_TypeDef * I2Cx, const uint8_t DeviceAddress, const uint8_t *pData, uint8_t Size) {
	int i;
	
	if (Size <= -1 || pData == NULL) return -1;
	
	I2C_WaitLineIdle(I2Cx);
	
	if (I2C_Start(I2Cx, DeviceAddress, Size, WRITE_TO_SLAVE) < 0 ) return -1;

	// Send Data
	// Write the first data in DR register
	// while((I2Cx->ISR & I2C_ISR_TXE) == 0);
	// I2Cx->TXDR = pData[0] & I2C_TXDR_TXDATA;  

	for (i = 0; i < Size; i++) {
		// TXE is set by hardware when the I2C_TXDR register is empty. It is cleared when the next
		// data to be sent is written in the I2C_TXDR register.
		// while( (I2Cx->ISR & I2C_ISR_TXE) == 0 ); 

		// TXIS bit is set by hardware when the I2C_TXDR register is empty and the data to be
		// transmitted must be written in the I2C_TXDR register. It is cleared when the next data to be
		// sent is written in the I2C_TXDR register.
		// The TXIS flag is not set when a NACK is received.
		while((I2Cx->ISR & I2C_ISR_TXIS) == 0 );
		I2Cx->TXDR = pData[i] & I2C_TXDR_TXDATA;  // TXE is cleared by writing to the TXDR register.
	}
	
	// Wait until TC flag is set 
	while((I2Cx->ISR & I2C_ISR_TC) == 0 && (I2Cx->ISR & I2C_ISR_NACKF) == 0);
	
	if( (I2Cx->ISR & I2C_ISR_NACKF) != 0 ) return -1;

	I2C_Stop(I2Cx);
	return 0;
}


//===============================================================================
//                           I2C Receive Data
//=============================================================================== 
int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size) {
	int i;
	
	if(Size <= 0 || pData == NULL) return -1;

	I2C_WaitLineIdle(I2Cx);

	I2C_Start(I2Cx, DeviceAddress, Size, READ_FROM_SLAVE); // 0 = sending data to the slave, 1 = receiving data from the slave
						  	
	for (i = 0; i < Size; i++) {
		// Wait until RXNE flag is set 	
		while( (I2Cx->ISR & I2C_ISR_RXNE) == 0 );
		pData[i] = I2Cx->RXDR & I2C_RXDR_RXDATA;
	}
	
	// Wait until TCR flag is set 
	while((I2Cx->ISR & I2C_ISR_TC) == 0);
	
	I2C_Stop(I2Cx);
	
	return 0;
}
