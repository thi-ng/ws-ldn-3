#include "led.h"

void led_all_init() {
	BSP_LED_Init(LED_ORANGE);
	BSP_LED_Init(LED_RED);
	BSP_LED_Init(LED_GREEN);
	BSP_LED_Init(LED_BLUE);
}

void led_all_on() {
	BSP_LED_On(LED_ORANGE);
	BSP_LED_On(LED_RED);
	BSP_LED_On(LED_GREEN);
	BSP_LED_On(LED_BLUE);
}

void led_all_off() {
	BSP_LED_Off(LED_ORANGE);
	BSP_LED_Off(LED_RED);
	BSP_LED_Off(LED_GREEN);
	BSP_LED_Off(LED_BLUE);
}
