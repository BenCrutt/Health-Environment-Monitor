#include "ADC.h"

#include "stm32l476xx.h"

#include <stdint.h>

void ADC_Wakeup(void) {
    int wait_time;

    // To start ADC operations, the following sequence should be applied
    // DEEPPWD = 0: ADC not in deep-power down
    // DEEPPWD = 1: ADC in deep-power-down (default reset state)
    if ((ADC1->CR & ADC_CR_DEEPPWD) == ADC_CR_DEEPPWD)
        ADC1->CR &= ~ADC_CR_DEEPPWD; // Exit deep power down mode if still in that state

    // Enable the ADC internal voltage regulator
    // Before performing any operation such as launching a calibration or enabling the ADC, the ADC
    // voltage regulator must first be enabled and the software must wait for the regulator start-up
    // time.
    ADC1->CR |= ADC_CR_ADVREGEN;

    // Wait for ADC voltage regulator start-up time
    // The software must wait for the startup time of the ADC voltage regulator (T_ADCVREG_STUP)
    // before launching a calibration or enabling the ADC.
    // T_ADCVREG_STUP = 20 us
    wait_time = 20 * (80000000 / 1000000);
    while (wait_time != 0) {
        wait_time--;
    }
}

void ADC_Common_Configuration() {
    // Set up ADC Common Configuration
	
		// Enable the I/O analog switch voltage booster
		SYSCFG->CFGR1 |= SYSCFG_CFGR1_BOOSTEN;
	
		// Modifying ADC Common Control Register:
		// Enable V_REFINT
		ADC123_COMMON->CCR |= ADC_CCR_VREFEN;
	
		// Ensure that the clock is not divided (PSC = 0)
		ADC123_COMMON->CCR &= ~ADC_CCR_PRESC;
	
		// Ensure ADC clock scheme is set to HCLK/1 synchronous clock mode
		ADC123_COMMON->CCR &= ~ADC_CCR_CKMODE; // Reset bits
		ADC123_COMMON->CCR |= ADC_CCR_CKMODE_0; // Set bits
	
		// Ensure all ADCs are operating in independent mode
		ADC123_COMMON->CCR &= ~ADC_CCR_DUAL;
}

void ADC_Pin_Init(void) {
    // Enable GPIO A Clock
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
		// Set PA1 to Analog mode
		GPIOA->MODER |= GPIO_MODER_MODE4;
	
		// Set PA1 to No Pull-Up, No Pull_Down
		GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;
	
		// Connect PA1 to ADC input
		//GPIOA->ASCR &= ~0xFFU;
		GPIOA->ASCR |= GPIO_ASCR_ASC4;
}

void ADC_Init(void) {
    // Enable ADC clock
		RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	
		// Reset ADC
		RCC->AHB2RSTR |= RCC_AHB2RSTR_ADCRST; // Reset
		RCC->AHB2RSTR &= ~RCC_AHB2RSTR_ADCRST; // Clear reset bit
	
    // Other ADC Initialization
    ADC_Pin_Init();
    ADC_Common_Configuration();
    ADC_Wakeup();

    // Disable ADC1 before modifying
		ADC1->CR &= ~ADC_CR_ADEN;
	
		// Configure ADC1 to have 12-bit resolution...
		ADC1->CFGR &= ~ADC_CFGR_RES;
	
		// ... and right alignment
		ADC1->CFGR &= ~ADC_CFGR_ALIGN;
	
		// Set sequence length to 1
		ADC1->SQR1 &= ~ADC_SQR1_L;
		
		// Ensure channel 6 is used in first conversion
		ADC1->SQR1 &= ~ADC_SQR1_SQ1; // Reset bits
		ADC1->SQR1 |= ADC_SQR1_SQ1_0;
		ADC1->SQR1 |= ADC_SQR1_SQ1_3; // 0b01001
		
		// Set channel 6 to single-ended mode
		ADC1->DIFSEL &= ~ADC_DIFSEL_DIFSEL_9;
		
		// Set sampling time for CH6 to 24.5 ADC clock cycles
		ADC1->SMPR1 &= ~ADC_SMPR1_SMP9; // Reset bits
		ADC1->SMPR1 |= ADC_SMPR1_SMP9_0;
		ADC1->SMPR1 |= ADC_SMPR1_SMP9_1; // 0b011
		
		// Ensure ADC is in single conversion mode...
		ADC1->CFGR &= ~ADC_CFGR_CONT;
		
		// ... and that hardware trigger detection is disabled
		ADC1->CFGR &= ~ADC_CFGR_EXTEN;
		
		// Enable ADC1 after modifying
		ADC1->CR |= ADC_CR_ADEN;
		
		// Wait for ADC1 to be ready
		while((ADC1->ISR & ADC_ISR_ADRDY) == 0);	
}
