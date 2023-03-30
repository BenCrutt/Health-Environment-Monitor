#include "UART.h"

void UART1_Init(void) {
	// Initialize UART1
	
	// Enable USART1 clock in peripheral clock register
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	
	// Select system clock as USART1 clock source
	RCC->CCIPR &= ~RCC_CCIPR_USART1SEL; // Reset bits
	RCC->CCIPR |= RCC_CCIPR_USART1SEL_0; // Set necessary	
}

void UART2_Init(void) {
	// Initialize UART2
	
	// Enable USART2 clock in peripheral clock register
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	
	// Select system clock as USART2 clock source
	RCC->CCIPR &= ~RCC_CCIPR_USART2SEL; // Reset bits
	RCC->CCIPR |= RCC_CCIPR_USART2SEL_0; // Set necessary	
}

void UART1_GPIO_Init(void) {
	// Enable GPIOA stuff
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
	// Set both GPIO pins to very high output speed
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);
	
	// Both pins have push-pull output type
	GPIOB->OTYPER |= (GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);
	
	// Set PA2 and PA3 to alternate function mode
	GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOB->MODER |= (GPIO_MODER_MODE7_1 | GPIO_MODER_MODE6_1);
	
	// Set alternate function of PA2 and PA3 to USART2
	GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6_3 | GPIO_AFRL_AFSEL7_3);
	
	// Both pins use pull-up resistors for I/O
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7); // Reset
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0); // Set bits
		
}

void UART2_GPIO_Init(void) {

	// Enable GPIOA stuff
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// Set both GPIO pins to very high output speed
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	
	// Both pins have push-pull output type
	GPIOA->OTYPER |= (GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
	
	// Set PA2 and PA3 to alternate function mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	GPIOA->MODER |= (GPIO_MODER_MODE3_1 | GPIO_MODER_MODE2_1);
	
	// Set alternate function of PA2 and PA3 to USART2
	GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_3 | GPIO_AFRL_AFSEL3_3);
	
	// Both pins use pull-up resistors for I/O
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3); // Reset
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPD2_0 | GPIO_PUPDR_PUPD3_0); // Set bits
}

void USART_Init(USART_TypeDef* USARTx) {
	// [TODO]
	
	// Disable USARTx
	USARTx->CR1 &= ~USART_CR1_UE;
	
	// Set word length to 8 bits
	USARTx->CR1 &= ~USART_CR1_M1;
	
	// Set oversampling mode to oversample by 16
	USARTx->CR1 &= ~USART_CR1_OVER8;
	
	// Set number of stop bits to 1
	USARTx->CR2 &= ~USART_CR2_STOP;
	
	// Set baud rate to 9600
	USARTx->BRR &= ~(USART_BRR_DIV_FRACTION | USART_BRR_DIV_MANTISSA);
	USARTx->BRR |= 8333;
	
	// Enable transmitter and receiver
	USARTx->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	
	// Enable USARTx
	USARTx->CR1 |= USART_CR1_UE;
}

uint8_t USART_Read (USART_TypeDef * USARTx) {
	// SR_RXNE (Read data register not empty) bit is set by hardware
	while (!(USARTx->ISR & USART_ISR_RXNE));  // Wait until RXNE (RX not empty) bit is set
	// USART resets the RXNE flag automatically after reading DR
	return ((uint8_t)(USARTx->RDR & 0xFF));
	// Reading USART_DR automatically clears the RXNE flag 
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes) {
	int i;
	// TXE is cleared by a write to the USART_DR register.
	// TXE is set by hardware when the content of the TDR 
	// register has been transferred into the shift register.
	for (i = 0; i < nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE));   	// wait until TXE (TX empty) bit is set
		// Writing USART_DR automatically clears the TXE flag 	
		USARTx->TDR = buffer[i] & 0xFF;
		USART_Delay(300);
	}
	while (!(USARTx->ISR & USART_ISR_TC));   		  // wait until TC bit is set
	USARTx->ISR &= ~USART_ISR_TC;
}   

void USART_Delay(uint32_t us) {
	uint32_t time = 100*us/7;    
	while(--time);   
}
