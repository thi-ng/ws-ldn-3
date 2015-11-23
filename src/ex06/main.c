#include "ex06/main.h"

__IO DMABufferState bufferState = BUFFER_OFFSET_NONE;

uint16_t audioBuffer[AUDIO_BUFFER_SIZE/2];

extern __IO uint32_t millis;

int main(void) {
	HAL_Init();

	led_all_init();

	SystemClock_Config();

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 70, SAMPLERATE) != 0) {
		Error_Handler();
	}

	Synth synth;
	synth_init(&synth);
	synth_bus_init(&(synth.bus[0]),
			(int16_t*) malloc(sizeof(int16_t) * DELAY_LENGTH),
			DELAY_LENGTH, 2);
	synth_osc_init(&(synth.lfoEnvMod), synth_osc_sin_dc, 1.5f, PI, 0.05f, 2.0f);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_BUFFER_SIZE);

	for (uint32_t i = 0; i < AUDIO_BUFFER_SIZE / 2;) {
		int16_t x = (int16_t) (sinf(FREQ_TO_RAD(220.0f) * (float)(i)) * 8191.0f);
		audioBuffer[i++] = x;
		audioBuffer[i++] = x;
	}

	while (1) {
		if (!(millis % 200)) {
			BSP_LED_Toggle(LED_GREEN);
			// SynthVoice *voice = synth_new_voice(&synth);
//			synth_adsr_init(&(voice->env), 0.0025f, 0.000025f, 0.00005f, 1.0f,
//					0.25f);
//			synth_osc_init(&(voice->lfoPitch), synth_osc_sin, FREQ_TO_RAD(5.0f),
//					0.0f, 10.0f, 0.0f);
//			synth_osc_init(&(voice->lfoMorph), synth_osc_saw_dc, 0.499f, 0.0f,
//					30.0f + 29.0f * sinf(millis * 0.0005f), 0.5f);
//			synth_osc_init(&(voice->osc[0]), synth_osc_sin, 0.15f, 0.0f, 440.0f,
//					0.0f);
//			synth_osc_init(&(voice->osc[1]), synth_osc_sin, 0.15f, 0.0f, 441.0f,
//					0.0f);
			HAL_Delay(16);
		}
		if (bufferState == BUFFER_OFFSET_HALF) {
			int16_t *ptr = (int16_t*) &audioBuffer[0];
			//synth_render_slice(&synth, ptr, AUDIO_BUFFER_SIZE >> 3);
			bufferState = BUFFER_OFFSET_NONE;
		}

		if (bufferState == BUFFER_OFFSET_FULL) {
			int16_t *ptr = (int16_t*) &audioBuffer[AUDIO_BUFFER_SIZE >> 1];
			//synth_render_slice(&synth, ptr, AUDIO_BUFFER_SIZE >> 3);
			bufferState = BUFFER_OFFSET_NONE;
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BSP_LED_Toggle(LED_BLUE);
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	bufferState = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	bufferState = BUFFER_OFFSET_FULL;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &audioBuffer[0],
	AUDIO_BUFFER_SIZE / 2);
}

void BSP_AUDIO_OUT_Error_CallBack(void) {
	Error_Handler();
}
