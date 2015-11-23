#ifndef __EX06_SYNTH_H
#define __EX06_SYNTH_H

#include <stdint.h>
#include "ex06/wavetable.h"

#define TAU                     6.283185307f
#define PI                    	3.14159265f
#define HALF_PI                 1.570796326794897f
#define INV_TAU					(1.0f / TAU)
#define INV_PI					(1.0f / PI)
#define INV_HALF_PI				(1.0f / HALF_PI)

#define SAMPLERATE				44100
#define SYNTH_POLYPHONY			4
#define AUDIO_BUFFER_SIZE		16384
#define ADSR_SCALE				32767.0f;
#define DELAY_LENGTH			(uint32_t)(SAMPLERATE * 0.125f)

#define FREQ_TO_RAD(freq)		((PI * (freq)) / SAMPLERATE)

inline float truncPhase(float phase) {
	while (phase >= TAU) {
		phase -= TAU;
	}
	return phase;
}

inline float clampf(float x, float min, float max) {
	return (x < min) ? min : (x > max ? max : x);
}

inline float stepf(float x, float edge, float y1, float y2) {
	return (x < edge ? y1 : y2);
}

inline float maddf(float a, float b, float c) {
	return a * b + c;
}

inline float mixf(float a, float b, float t) {
	return maddf(b - a, t, a);
}

typedef struct SynthOsc SynthOsc;

typedef float (*OscFn)(SynthOsc*, float lfo, float lfo2);

struct SynthOsc {
	OscFn fn;
	float phase;
	float freq;
	float amp;
	float dcOffset;
	const float *wtable1;
	const float *wtable2;
};

typedef enum {
	IDLE = 0, ATTACK = 1, DECAY = 2, SUSTAIN = 3, RELEASE = 4
} ADSRPhase;

typedef struct {
	float currGain;
	float attackGain;
	float sustainGain;
	float attackRate;
	float decayRate;
	float releaseRate;
	ADSRPhase phase;
} ADSR;

typedef struct {
	SynthOsc osc[2];
	SynthOsc lfoPitch;
	SynthOsc lfoMorph;
	ADSR env;
	uint32_t flags;
} SynthVoice;

typedef struct {
	int16_t *buf;
	size_t len;
	uint32_t readPos;
	uint32_t writePos;
	int16_t *readPtr;
	int16_t *writePtr;
	int16_t inL;
	int16_t inR;
	uint8_t decay;
} SynthFXBus;

typedef struct {
	SynthVoice voices[SYNTH_POLYPHONY];
	SynthOsc lfoFilter;
	SynthOsc lfoEnvMod;
	SynthFXBus bus[1];
	uint8_t nextVoice;
} Synth;

void		synth_osc_init(SynthOsc *osc, OscFn fn, float gain, float phase, float freq, float dc);
void		synth_osc_set_wavetables(SynthOsc *osc, const float *tbl1, const float *tbl2);
float		synth_osc_sin(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_sin_dc(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_sin2(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_rect(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_rect_phase(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_rect_dc(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_saw(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_saw_dc(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_tri(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_tri_dc(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_noise(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_noise_dc(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_nop(SynthOsc *osc, float lfo, float lfo2);
float		synth_osc_wtable_morph(SynthOsc *osc, float lfo, float lfo2);

void		synth_adsr_init(ADSR *env, float attRate, float decayRate, float releaseRate, float attGain, float sustainGain);
float		synth_adsr_update(ADSR *env, float envMod);

void		synth_bus_init(SynthFXBus *bus, int16_t *buf, size_t len, uint8_t decay);

void		synth_voice_init(SynthVoice *voice, uint32_t flags);
void		synth_init(Synth *synth);

SynthVoice*	synth_new_voice(Synth *synth);
void		synth_render_slice(Synth *synth, int16_t *ptr, size_t len);

#endif
