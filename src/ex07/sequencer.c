#include "ex07/sequencer.h"
#include "synth/synth.h"

SeqTrack* initTrack(SeqTrack *track, SeqTrackFn fn, float *notes,
		uint16_t length, uint32_t ticks) {
	track->fn = fn;
	track->notes = notes;
	track->length = length;
	track->ticks = ticks;
	track->currNote = 0;
	track->lastNoteTick = 0xffffffff;
	return track;
}

void updateTrack(Synth *synth, SeqTrack *track, uint32_t tick) {
	if (tick != track->lastNoteTick) {
		if (!(tick % track->ticks)) {
			float note = track->notes[track->currNote];
			if (note > 0.0f) {
				track->fn(synth, note, tick);
			}
			track->currNote = (track->currNote + 1) % track->length;
			track->lastNoteTick = tick;
		}
	}
}

void updateAllTracks(Synth *synth, SeqTrack* *tracks, uint8_t numTracks,
		uint32_t tick) {
	while (numTracks--) {
		updateTrack(synth, *(tracks++), tick);
	}
}
