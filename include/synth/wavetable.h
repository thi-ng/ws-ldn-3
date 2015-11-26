#ifndef __WAVETABLE_H
#define __WAVETABLE_H

#include <math.h>
#include "synth/synth.h"

#define WAVE_TABLE_LENGTH		1024
#define WAVE_TABLE_SCALE		(WAVE_TABLE_LENGTH / TAU)
#define WTABLE_LOOKUP(table, x)	(*(table + (uint32_t)((x) * WAVE_TABLE_SCALE)))

extern const float wtable_sin[WAVE_TABLE_LENGTH];
extern const float wtable_harmonics_1[WAVE_TABLE_LENGTH];
extern const float wtable_harmonics_2[WAVE_TABLE_LENGTH];
extern const float wtable_harmonics_3[WAVE_TABLE_LENGTH];
extern const float wtable_noise[WAVE_TABLE_LENGTH];

#endif
