#include "LED.h"

void LED_Init(void) {
	// Enable GPIO Clocks
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;  // Enable clock of Port A for Green LED
	
	// Initialize Green LED
	GPIOA->MODER &= ~(3UL<<10); // Clear mode bits
	GPIOA->MODER |= 1UL<<10;    // Set mode to output
	GPIOA->OTYPER &= ~(1UL<<5); // Select push-pull output
	GPIOA->PUPDR &= ~(3UL<<10); // No Pull-up, No pull-down
}

void Green_LED_Off(void) {
	GPIOA->ODR &= ~1UL << 5; //Reset to off
}

void Green_LED_On(void) {
	GPIOA->ODR |= 1UL << 5;  //Set to on
}

void Green_LED_Toggle(void) {
	GPIOA->ODR ^= 1UL << 5;  //Toggles
}
