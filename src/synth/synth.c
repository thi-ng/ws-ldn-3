#include <string.h>
#include <stdlib.h>
#include "synth/synth.h"

static tinymt32_t synthRNG;

void synth_osc_init(SynthOsc *osc, OscFn fn, float gain, float phase,
		float freq, float dc) {
	osc->fn = fn;
	osc->phase = phase;
	osc->freq = FREQ_TO_RAD(freq);
	osc->amp = gain;
	osc->dcOffset = dc;
}

void synth_osc_set_wavetables(SynthOsc *osc, const float* tbl1,
		const float* tbl2) {
	osc->wtable1 = tbl1;
	osc->wtable2 = tbl2;
}

float synth_osc_sin(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return WTABLE_LOOKUP(wtable_sin, phase) * osc->amp;
}

float synth_osc_sin_math(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return sinf(phase) * osc->amp;
}

float synth_osc_sin_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return maddf(WTABLE_LOOKUP(wtable_sin, phase), osc->amp, osc->dcOffset);
}

float synth_osc_rect(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return stepf(phase, PI, osc->amp, -osc->amp);
}

float synth_osc_rect_phase(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return stepf(phase, PI + lfo2, osc->amp, -osc->amp);
}

float synth_osc_rect_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return osc->dcOffset + stepf(phase, PI, osc->amp, -osc->amp);
}

float synth_osc_saw(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return (phase * INV_PI - 1.0f) * osc->amp;
}

float synth_osc_saw_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	return maddf(phase * INV_PI - 1.0f, osc->amp, osc->dcOffset);
}

float synth_osc_tri(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	float x = 2.0f - (phase * INV_HALF_PI);
	x = 1.0f - stepf(x, 0.0f, -x, x);
	if (x > -1.0f) {
		return x * osc->amp;
	} else {
		return -osc->amp;
	}
}

float synth_osc_tri_dc(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	osc->phase = phase;
	float x = 2.0f - (phase * INV_HALF_PI);
	x = 1.0f - stepf(x, 0.0f, -x, x);
	if (x > -1.0f) {
		return maddf(x, osc->amp, osc->dcOffset);
	} else {
		return osc->dcOffset - osc->amp;
	}
}

float synth_osc_wtable_simple(SynthOsc *osc, float lfo, float lfo2) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	truncPhase(phase);
	osc->phase = phase;
	return WTABLE_LOOKUP(osc->wtable1, phase) * osc->amp;
}

float synth_osc_wtable_morph(SynthOsc *osc, float lfo, float morph) {
	float phase = truncPhase(osc->phase + osc->freq + lfo);
	truncPhase(phase);
	osc->phase = phase;
	uint32_t idx = WTABLE_INDEX(phase);
	return mixf(WTABLE_LOOKUP_RAW(osc->wtable1, idx),
			WTABLE_LOOKUP_RAW(osc->wtable2, idx), morph) * osc->amp;
}

float synth_osc_noise(SynthOsc *osc, float lfo, float lfo2) {
	return NORM_RANDF(&synthRNG) * osc->amp;
}

float synth_osc_noise_dc(SynthOsc *osc, float lfo, float lfo2) {
	return osc->dcOffset + NORM_RANDF(&synthRNG) * osc->amp;
}

float synth_osc_nop(SynthOsc *osc, float lfo, float lfo2) {
	return osc->dcOffset;
}

void synth_adsr_init(ADSR *env, float attRate, float decayRate,
		float releaseRate, float attGain, float sustainGain) {
	env->attackRate = attRate * ADSR_SCALE
	;
	env->decayRate = decayRate * ADSR_SCALE
	;
	env->releaseRate = releaseRate * ADSR_SCALE
	;
	env->attackGain = attGain * ADSR_SCALE
	;
	env->sustainGain = sustainGain * ADSR_SCALE
	;
	env->phase = ATTACK;
	env->currGain = 0.0f;
}

float synth_adsr_update(ADSR *env, float envMod) {
	switch (env->phase) {
	case ATTACK:
		if (env->currGain >= env->attackGain) {
			env->phase = DECAY;
		} else {
			env->currGain += env->attackRate * envMod;
			if (env->currGain > ADSR_SCALE) {
				env->currGain = ADSR_SCALE;
			}
		}
		break;
	case DECAY:
		if (env->currGain > env->sustainGain) {
			env->currGain -= env->decayRate * envMod;
		} else {
			env->phase = RELEASE; // skip SUSTAIN phase for now
		}
		break;
	case SUSTAIN:
		return env->sustainGain;
	case RELEASE:
		if (env->currGain > 3e-5f) { // ~0.98 in 16bit
			env->currGain -= env->releaseRate;
			if (env->currGain < 0.0f) {
				env->currGain = 0.0f;
			}
		} else {
			env->phase = IDLE;
		}
		break;
	default:
		break;
	}
	return env->currGain;
}

void synth_voice_init(SynthVoice *voice, uint32_t flags) {
	synth_osc_init(&(voice->lfoPitch), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	synth_osc_init(&(voice->lfoMorph), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	synth_init_iir(&(voice->filter[0]), IIR_HP, 0.0f, 0.85f);
	synth_init_iir(&(voice->filter[1]), IIR_HP, 0.0f, 0.85f);
	voice->age = 0;
	voice->flags = flags;
}

void synth_init(Synth *synth) {
	synth->nextVoice = 0;
	for (uint8_t i = 0; i < SYNTH_POLYPHONY; i++) {
		SynthVoice *voice = synth_new_voice(synth);
		synth_adsr_init(&(voice->env), 0.0025f, 0.00025f, 0.00005f, 1.0f,
				0.25f);
		voice->env.phase = IDLE;
	}
	synth_osc_init(&(synth->lfoFilter), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	synth_osc_init(&(synth->lfoEnvMod), synth_osc_nop, 0.0f, 0.0f, 0.0f, 0.0f);
	synth_bus_init(&(synth->bus[0]), malloc(sizeof(int16_t)), 1, 2);
	tinymt32_init(&synthRNG, 0xcafebad);
}

SynthVoice* synth_new_voice(Synth *synth) {
	SynthVoice* voice = NULL;
	uint32_t maxAge = 0;
	for (uint32_t i = 0; i < SYNTH_POLYPHONY; i++) {
		if (synth->voices[i].env.phase == IDLE) {
			voice = &synth->voices[i];
			break;
		}
		if (synth->voices[i].age > maxAge) {
			voice = &synth->voices[i];
			maxAge = synth->voices[i].age;
		}
	}
	synth_voice_init(voice, 0);
	return voice;
}

void synth_bus_init(SynthFXBus *bus, int16_t *buf, size_t len, uint8_t decay) {
	if (bus->buf != NULL) {
		free(bus->buf);
	}
	bus->buf = buf;
	bus->len = len;
	bus->readPos = 1;
	bus->writePos = 0;
	bus->readPtr = &buf[bus->readPos];
	bus->writePtr = &buf[bus->writePos];
	bus->decay = decay;
	memset(buf, 0, len << 1);
}

void synth_init_iir(IIRState *state, IIRType type, float cutoff, float reso) {
	state->f[0] = 0.0f; // lp
	state->f[1] = 0.0f; // hp
	state->f[2] = 0.0f; // bp
	state->f[3] = 0.0f; // br
	state->type = type;
	synth_set_iir_coeff(state, cutoff, reso);
}

void synth_set_iir_coeff(IIRState *iir, float cutoff, float reso) {
	iir->cutoff = cutoff;
	iir->resonance = reso;
	iir->freq = 2.0f * sinf(PI * fminf(0.5f, cutoff * INV_NYQUIST_FREQ));
	iir->damp = fminf(2.0f * (1.0f - powf(reso, 0.05f)),
			fminf(2.0f, 2.0f / iir->freq - iir->freq * 0.5f));
}

float synth_process_iir(IIRState *state, float input, float env) {
	float *f = state->f;
	// 1st pass
	f[3] = input - state->damp * f[2];
	f[0] = f[0] + state->freq * f[2];
	f[1] = f[3] - f[0];
	f[2] = state->freq * f[1] + f[2];
	float output = f[state->type];

	// 2nd pass
	f[3] = input - state->damp * f[2];
	f[0] = f[0] + state->freq * f[2];
	f[1] = f[3] - f[0];
	f[2] = state->freq * f[1] + f[2];
	output = 0.5f * (output + f[state->type]);
	return (input + (output - input) * env);
}

void synth_render_slice(Synth *synth, int16_t *ptr, size_t len) {
	int32_t sumL, sumR;
	SynthOsc *lfoEnvMod = &(synth->lfoEnvMod);
	SynthFXBus *fx = &(synth->bus[0]);
	while (len--) {
		sumL = sumR = 0;
		float envMod = lfoEnvMod->fn(lfoEnvMod, 0.0f, 0.0f);
		SynthVoice *voice = &(synth->voices[SYNTH_POLYPHONY - 1]);
		while (voice >= synth->voices) {
			voice->age++;
			ADSR *env = &(voice->env);
			if (env->phase) {
				float gain = synth_adsr_update(env, envMod);
				SynthOsc *osc = &(voice->lfoPitch);
				float lfoVPitchVal = osc->fn(osc, 0.0f, 0.0f);
				osc = &(voice->lfoMorph);
				float lfoVMorphVal = osc->fn(osc, 0.0f, 0.0f);
				osc = &(voice->osc[0]);
				float val = osc->fn(osc, lfoVPitchVal, lfoVMorphVal);
				val = synth_process_iir(&(voice->filter[0]), val, 1.0f);
				sumL += (int32_t) (gain * val);
				osc++;
				val = osc->fn(osc, lfoVPitchVal, lfoVMorphVal);
				val = synth_process_iir(&(voice->filter[1]), val, 1.0f);
				sumR += (int32_t) (gain * val);
			}
			voice--;
		}
		sumL += *(fx->readPtr);
		clamp16(sumL);
		sumR += *(fx->readPtr);
		clamp16(sumR);
#ifdef SYNTH_USE_DELAY
		fx->readPtr++;
		fx->readPos++;
		if (fx->readPos >= fx->len) {
			fx->readPos = 0;
			fx->readPtr = &(fx->buf[0]);
		}
#endif
		//sumL = (sumL + sumR) >> 1;
		*ptr = sumL;
		ptr++;
		*ptr = sumR;
		ptr++;
#ifdef SYNTH_USE_DELAY
		*(fx->writePtr) = clamp16((sumL + sumR) >> fx->decay);
		fx->writePtr++;
		fx->writePos++;
		if (fx->writePos >= fx->len) {
			fx->writePos = 0;
			fx->writePtr = &(fx->buf[0]);
		}
#endif
	}
}
