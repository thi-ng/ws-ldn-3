#ifndef __EX06_MAIN_H
#define __EX06_MAIN_H

#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "stm32f401_discovery.h"
#include "stm32f401_discovery_audio.h"
#include "stm32f401_discovery_accelerometer.h"
#include "ex06/stm32f4xx_it.h"
#include "clockconfig.h"
#include "led.h"
#include "synth.h"
#include "scales.h"
#include "math.h"

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL
} DMABufferState;

#endif
