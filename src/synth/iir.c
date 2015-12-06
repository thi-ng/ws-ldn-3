#include <math.h>
#include <stdio.h>
#include "iir.h"

void synth_init_iir(DSPNode *node, IIRType type, DSPNode *src, DSPNode *lfo,
		float cutoff, float reso) {
	synth_free_node_state(node);
	IIRState *state = (IIRState*) malloc(sizeof(IIRState));
	state->src = &src->buf[0];
	state->lfo = (lfo != NULL ? &lfo->buf[0] : NULL);
	state->f[0] = 0.0f; // lp
	state->f[1] = 0.0f; // hp
	state->f[2] = 0.0f; // bp
	state->f[3] = 0.0f; // br
	state->type = type;
	node->state = state;
	node->handler = synth_process_iir;
	synth_set_iir_coeff(node, cutoff, reso);
}

void synth_set_iir_coeff(DSPNode *node, float cutoff, float reso) {
	IIRState *iir = (IIRState*) (node->state);
	iir->cutoff = cutoff;
	iir->resonance = reso;
	iir->freq = 2.0f * sinf(PI * fminf(0.5f, cutoff * INV_NYQUIST_FREQ));
	iir->damp = fminf(2.0f * (1.0f - powf(reso, 0.25f)),
			fminf(2.0f, 2.0f / iir->freq - iir->freq * 0.5f));
	printf("freq: %f, damp: %f\n", iir->freq, iir->damp);
}

uint8_t synth_process_iir(DSPNode *node, DSPPipe *pipe, Synth *synth) {
	IIRState *state = (IIRState*) (node->state);
	float *src = state->src;
	float *lfo = state->lfo;
	float *buf = &node->buf[0];
	float *f = state->f;
	uint32_t len = AUDIO_BUFFER_SIZE_DIV2;
	while (len--) {
		float input = *src++;

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
		*buf++ = (lfo != NULL) ? (input + (output - input) * *lfo++) : output;
	}
	return 0;
}
