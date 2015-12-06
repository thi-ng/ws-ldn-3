#ifndef FILTER_IIR_H
#define FILTER_IIR_H

#include "synth.h"

typedef enum {
  IIR_LP = 0, IIR_HP, IIR_BP, IIR_BR
} IIRType;

typedef struct {
  float* src;
  float* lfo;
  float f[4];
  float cutoff;
  float resonance;
  float freq;
  float damp;
  IIRType type;
} IIRState;

void synth_init_iir(DSPNode *node, IIRType type, DSPNode *src, DSPNode *lfo, float cutoff, float reso);

void synth_set_iir_coeff(DSPNode *node, float cutoff, float reso);

uint8_t synth_process_iir(DSPNode *node, DSPPipe *pipe, Synth *synth);

#endif
