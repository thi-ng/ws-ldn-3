#include "synth/sequencer.h"
#include "synth/synth.h"

SeqTrack* initTrack(SeqTrack *track, SeqTrackFn fn, int8_t *notes,
		uint16_t length, uint32_t ticks, float tempoScale) {
	track->fn = fn;
	track->notes = notes;
	track->length = length;
	track->ticks = (uint32_t) (ticks * tempoScale);
	track->currNote = 0;
	track->lastNoteTick = 0xffffffff;
	track->gain = 1.0f;
	track->pitchBend = 0;
	track->tempoScale = tempoScale;
	return track;
}

void updateTrack(Synth *synth, SeqTrack *track, uint32_t tick) {
	if (tick != track->lastNoteTick) {
		if (!(tick % track->ticks)) {
			int8_t note = track->notes[track->currNote];
			if (note >= 0) {
				track->fn(synth, track, note, tick);
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
