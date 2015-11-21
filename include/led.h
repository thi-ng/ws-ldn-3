#ifndef __ACT_LED_H
#define __ACT_LED_H

#include "stm32f401_discovery.h"

#define LED_ORANGE LED3
#define LED_GREEN LED4
#define LED_RED LED5
#define LED_BLUE LED6

void led_all_init(void);
void led_all_on(void);
void led_all_off(void);

#endif
