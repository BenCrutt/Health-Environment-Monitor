#ifndef __STM32L476R_NUCLEO_SYSTICK_H
#define __STM32L476R_NUCLEO_SYSTICK_H

#include "stm32l476xx.h"

extern volatile uint32_t msTicks;

void SysTick_Init(void);
void SysTick_Handler(void);
void delay (uint32_t T);

#endif /* __STM32L476R_NUCLEO_SYSTICK_H */
