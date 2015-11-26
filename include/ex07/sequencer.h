#ifndef __EX07_SEQUENCER_H_
#define __EX07_SEQUENCER_H_

#include "synth/synth.h"

typedef struct SeqTrack SeqTrack;

typedef void (*SeqTrackFn)(Synth *synth, float freq, uint32_t tick);

struct SeqTrack {
	SeqTrackFn fn;
	float *notes;
	uint16_t length;
	uint16_t currNote;
	uint32_t ticks;
	uint32_t lastNoteTick;
};

SeqTrack* initTrack(SeqTrack *track, SeqTrackFn fn, float *notes, uint16_t length, uint32_t ticks);
void updateTrack(Synth *synth, SeqTrack *track, uint32_t tick);
void updateAllTracks(Synth *synth, SeqTrack **tracks, uint8_t numTracks, uint32_t tick);

#endif
