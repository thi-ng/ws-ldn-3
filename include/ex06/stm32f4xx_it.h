#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#include "ex06/main.h"

void Error_Handler(void);
void SysTick_Handler(void);
void EXTI0_IRQHandler(void);
void I2S2_IRQHandler(void);

#endif
