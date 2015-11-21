#include "ex01/main.h"

uint32_t currLED = 0;
int32_t ledDirection = 1;

Led_TypeDef ledSeq[] = { LED_ORANGE, LED_RED, LED_BLUE, LED_GREEN };

int main(void) {
	HAL_Init();

	led_all_init();
	SystemClock_Config();
	// option #1
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
	// option #2
	//BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	BSP_LED_Toggle(ledSeq[currLED]);

	while (1) {
		// option #1: synchronous button read via GPIO pin polling
		if (BSP_PB_GetState(BUTTON_KEY) == SET) {
			while (BSP_PB_GetState(BUTTON_KEY) != RESET);
			ledDirection = -ledDirection;
		}
		BSP_LED_Toggle(ledSeq[currLED]);
		currLED = (currLED + ledDirection) & 3;
		BSP_LED_Toggle(ledSeq[currLED]);
		HAL_Delay(LED_SPEED);
	}
}

// option #2: async button handling via EXTI interrupt

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	if (GPIO_Pin == KEY_BUTTON_PIN) {
//		//while (BSP_PB_GetState(BUTTON_KEY) != RESET);
//		ledDirection = -ledDirection;
//	}
//}
