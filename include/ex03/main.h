#ifndef __EX03_MAIN_H
#define __EX03_MAIN_H

#include "stm32f401_discovery.h"
#include "stm32f401_discovery_accelerometer.h"
#include "stm32f401_discovery_audio.h"
#include "stm32f401_discovery_gyroscope.h"
#include <ex03/mems.h>
#include <stdio.h>

typedef void (*DemoFn)(void);

void Error_Handler(void);

#endif
