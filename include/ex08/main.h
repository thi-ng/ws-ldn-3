#ifndef __EX08_MAIN_H
#define __EX08_MAIN_H

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f401_discovery.h"
#include "stm32f401_discovery_audio.h"
#include "stm32f401_discovery_accelerometer.h"
#include "stm32f4xx_it.h"
#include "clockconfig.h"
#include "led.h"
#include "usbh_MIDI.h"

#define MIDI_BUF_SIZE 64

typedef enum {
	APP_IDLE = 0, APP_START, APP_READY, APP_RUNNING, APP_DISCONNECT
} AppState;

typedef enum {
	PLAYBACK_PAUSE = 0, PLAYBACK_RESUME, PLAYBACK_IDLE
} PlaybackState;

typedef enum {
	USBH_USER_FS_INIT = 0, USBH_USER_AUDIO
} USBAppState;

#endif
