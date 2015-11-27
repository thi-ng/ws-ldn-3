#include "ex08/main.h"

AppState appState = APP_IDLE;
__IO PlaybackState playbackState = PLAYBACK_IDLE;

USBH_HandleTypeDef hUSBHost; /* USB Host handle - DON'T change name*/
USBAppState usbAppState = USBH_USER_FS_INIT;

uint8_t midiReceiveBuffer[MIDI_BUF_SIZE];
uint8_t audioBuffer[AUDIO_BUFFER_SIZE];
__IO DMABufferState bufferState = BUFFER_OFFSET_NONE;

static void usbUserProcess(USBH_HandleTypeDef *pHost, uint8_t vId);
static void midiApplication(void);
static void processMidiPackets(void);
static void initAudio(void);
static void initSequencer(void);
static void resumePlayback();
static void pausePlayback();
static void playNoteInst1(Synth* synth, int8_t note, uint32_t tick);
static void playNoteInst2(Synth* synth, int8_t note, uint32_t tick);
static void updateAudioBuffer(Synth *synth);

static Synth synth;
static SeqTrack* tracks[2];

static int8_t notes1[16] = { 36, -1, 12, 12, -1, -1, -1, -1, 48, -1, 17, 12, -1,
		-1, -1, 24 };

static int8_t notes2[16] = { 0, 12, 0, 12, 0, 12, 0, 12, 7, 19, 7, 19, 7, 19, 7,
		19, };

int main(void) {
	HAL_Init();

	led_all_init();
	SystemClock_Config();

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

	USBH_Init(&hUSBHost, usbUserProcess, 0);
	USBH_RegisterClass(&hUSBHost, USBH_MIDI_CLASS);
	USBH_Start(&hUSBHost);
	while (1) {
		midiApplication();
		USBH_Process(&hUSBHost);
	}
}

void usbUserProcess(USBH_HandleTypeDef *usbHost, uint8_t eventID) {
	switch (eventID) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;
	case HOST_USER_DISCONNECTION:
		appState = APP_DISCONNECT;
		BSP_LED_Off(LED_GREEN);
		BSP_LED_Off(LED_BLUE);
		break;
	case HOST_USER_CLASS_ACTIVE:
		appState = APP_READY;
		BSP_LED_On(LED_GREEN);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_RED);
		break;
	case HOST_USER_CONNECTION:
		appState = APP_START;
		BSP_LED_On(LED_BLUE);
		break;
	default:
		break;
	}
}

void midiApplication(void) {
	switch (appState) {
	case APP_READY:
		USBH_MIDI_Receive(&hUSBHost, midiReceiveBuffer, MIDI_BUF_SIZE); // just once
		initAudio();
		initSequencer();
		appState = APP_RUNNING;
		playbackState = PLAYBACK_PLAY;
		break;
	case APP_RUNNING:
		if ((playbackState == PLAYBACK_PLAY)
				|| (playbackState == PLAYBACK_RESUME)) {
			uint32_t tick = HAL_GetTick();
			updateAllTracks(&synth, tracks, 2, tick);
			updateAudioBuffer(&synth);
		}
		break;
	case APP_DISCONNECT:
		appState = APP_IDLE;
		playbackState = PLAYBACK_IDLE;
		USBH_MIDI_Stop(&hUSBHost);
		break;
	default:
		break;
	}
}

void processMidiPackets() {
	uint8_t *ptr = midiReceiveBuffer;
	midi_package_t packet;

	uint16_t numPackets = USBH_MIDI_GetLastReceivedDataSize(&hUSBHost) >> 2;
//	if (numPackets > 0) {
//		BSP_LED_On(LED_ORANGE);
//	} else {
//		BSP_LED_Off(LED_ORANGE);
//	}
	if (numPackets != 0) {
		while (numPackets--) {
			packet.cin_cable = *ptr++;
			uint8_t type = *ptr++;
			uint8_t subtype = *ptr++;
			uint8_t val = *ptr++;

			if (packet.cin_cable != 0) {
				BSP_LED_On(LED_BLUE);
			}

			if ((type & 0xf0) == 0xb0) {
				switch (subtype) {
				case MIDI_CC_STOP_BT:
					BSP_LED_Off(LED_RED);
					pausePlayback();
					break;
				case MIDI_CC_PLAY_BT:
					BSP_LED_On(LED_RED);
					resumePlayback();
					break;
				default:
					break;
				}
			}
		}
	}
}

void initAudio(void) {
	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 85, SAMPLERATE) != 0) {
		Error_Handler();
	}
	synth_init(&synth);
	synth_bus_init(&(synth.bus[0]),
			(int16_t*) malloc(sizeof(int16_t) * DELAY_LENGTH),
			DELAY_LENGTH, 2);
	synth_osc_init(&(synth.lfoEnvMod), synth_osc_sin_dc, 0, 0, 0, 1.0f);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioBuffer[0], AUDIO_BUFFER_SIZE);
}

void initSequencer(void) {
	tracks[0] = initTrack((SeqTrack*) malloc(sizeof(SeqTrack)), playNoteInst1,
			notes1, 16, 250);

	tracks[1] = initTrack((SeqTrack*) malloc(sizeof(SeqTrack)), playNoteInst2,
			notes2, 16, 500);
}

void resumePlayback(void) {
	playbackState = PLAYBACK_RESUME;
	BSP_AUDIO_OUT_Resume();
}

void pausePlayback(void) {
	playbackState = PLAYBACK_PAUSE;
	BSP_AUDIO_OUT_Pause();
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

void playNoteInst1(Synth* synth, int8_t note, uint32_t tick) {
	float freq = notes[note];
	SynthVoice *voice = synth_new_voice(synth);
	synth_adsr_init(&(voice->env), 0.25f, 0.000025f, 0.005f, 1.0f, 0.95f);
	synth_osc_init(&(voice->lfoPitch), synth_osc_sin, FREQ_TO_RAD(5.0f), 0.0f,
			10.0f, 0.0f);
	synth_osc_init(&(voice->osc[0]), synth_osc_sin, 0.20f, 0.0f, freq, 0.0f);
	synth_osc_init(&(voice->osc[1]), synth_osc_sin, 0.10f, 0.0f, freq, 0.0f);
	//BSP_LED_Toggle(LED_GREEN);
}

void playNoteInst2(Synth* synth, int8_t note, uint32_t tick) {
	float freq = notes[note] * 0.5f;
	SynthVoice *voice = synth_new_voice(synth);
	synth_adsr_init(&(voice->env), 0.25f, 0.0000025f, 0.005f, 1.0f, 0.95f);
	synth_osc_init(&(voice->lfoPitch), synth_osc_sin, FREQ_TO_RAD(5.0f), 0.0f,
			10.0f, 0.0f);
	synth_osc_init(&(voice->osc[0]), synth_osc_sin, 0.30f, 0.0f, freq, 0.0f);
	synth_osc_init(&(voice->osc[1]), synth_osc_sin, 0.30f, 0.0f, freq * 0.51f,
			0.0f);
	//BSP_LED_Toggle(LED_ORANGE);
}

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost) {
	processMidiPackets();
	USBH_MIDI_Receive(&hUSBHost, midiReceiveBuffer, MIDI_BUF_SIZE); // start a new reception
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
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
