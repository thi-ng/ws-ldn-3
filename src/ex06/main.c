#include "ex06/main.h"

__IO DMABufferState bufferState = BUFFER_OFFSET_NONE;
__IO int32_t noteDir = 1;
__IO int32_t isPressed = 0;

uint8_t audioBuffer[AUDIO_BUFFER_SIZE];

static void updateAudioBuffer(Synth *synth);
//static tinymt32_t rng;

int main(void) {
	HAL_Init();
	SystemClock_Config();
	led_all_init();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	HAL_Delay(1000);

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 75, SAMPLERATE) != 0) {
		Error_Handler();
	}

	Synth synth;
	synth_init(&synth);
	synth_bus_init(&(synth.bus[0]),
			(int16_t*) malloc(sizeof(int16_t) * DELAY_LENGTH),
			DELAY_LENGTH, 2);
	synth_osc_init(&(synth.lfoEnvMod), synth_osc_sin_dc, 1.5f, PI, 0.05f, 2.0f);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_BUFFER_SIZE);

	uint32_t nextNote = 0;
	int32_t noteID = 0;

	uint8_t scale[] = { 0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26, 28, 31, 33,
			36 };

	OscFn instruments[] = { synth_osc_sin, synth_osc_rect, synth_osc_tri,
			synth_osc_saw };

	//tinymt32_init(&rng, 0xdecafbad);

	while (1) {
		uint32_t tick = HAL_GetTick();
		if ((tick % 250) == 0) {
			if (nextNote) {
				BSP_LED_Toggle(LED_GREEN);
				SynthVoice *voice = synth_new_voice(&synth);
				synth_adsr_init(&(voice->env), 0.0025f, 0.000025f, 0.00005f,
						1.0f, 0.25f);
				synth_osc_init(&(voice->lfoPitch), synth_osc_sin,
						FREQ_TO_RAD(5.0f), 0.0f, 10.0f, 0.0f);
				float freq = notes[7 + scale[noteID]];
				uint32_t instID = 2;
				//uint32_t instID = tinymt32_generate_uint32(&rng) & 3;
				synth_osc_init(&(voice->osc[0]), instruments[instID], 0.15f,
						0.0f, freq, 0.0f);
				freq = notes[2 + scale[noteID]];
				//instID = tinymt32_generate_uint32(&rng) & 3;
				synth_osc_init(&(voice->osc[1]), instruments[instID], 0.15f,
						0.0f, freq * 1.01f, 0.0f);
				nextNote = 0;
				noteID = (noteID + noteDir);
				if (noteID >= 16) {
					noteID = 0;
				} else if (noteID <= 0) {
					noteID = 15;
				}
			}
		} else {
			nextNote = 1;
		}
		updateAudioBuffer(&synth);
	}
}

void updateAudioBuffer(Synth *synth) {
	if (bufferState == BUFFER_OFFSET_HALF) {
		int16_t *ptr = (int16_t*) &audioBuffer[0];
		synth_render_slice(synth, ptr, AUDIO_BUFFER_SIZE >> 3);
		bufferState = BUFFER_OFFSET_NONE;
	}

	if (bufferState == BUFFER_OFFSET_FULL) {
		int16_t *ptr = (int16_t*) &audioBuffer[AUDIO_BUFFER_SIZE >> 1];
		synth_render_slice(synth, ptr, AUDIO_BUFFER_SIZE >> 3);
		bufferState = BUFFER_OFFSET_NONE;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
	if (pin == KEY_BUTTON_PIN) {
		if (!isPressed) {
			BSP_LED_Toggle(LED_BLUE);
			noteDir = -noteDir;
			isPressed = 1;
		} else {
			isPressed = 0;
		}
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	bufferState = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	bufferState = BUFFER_OFFSET_FULL;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &audioBuffer[0],
	AUDIO_BUFFER_SIZE >> 1);
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	BSP_LED_On(LED_RED);
	while (1) {
	}
}
