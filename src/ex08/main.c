#include "ex08/main.h"

AppState appState = APP_IDLE;

USBH_HandleTypeDef hUSBHost; /* USB Host handle - DON'T change name*/
USBAppState usbAppState = USBH_USER_FS_INIT;

uint8_t midiReceiveBuffer[MIDI_BUF_SIZE];

static void usbUserProcess(USBH_HandleTypeDef *pHost, uint8_t vId);
static void midiApplication(void);
static void processMidiPackets(void);

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
		appState = APP_RUNNING;
		break;
	case APP_RUNNING:
		break;
	case APP_DISCONNECT:
		appState = APP_IDLE;
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
	if (numPackets > 0) {
		BSP_LED_On(LED_ORANGE);
	} else {
		BSP_LED_Off(LED_ORANGE);
	}
	if (numPackets != 0) {
		while (numPackets--) {
			packet.cin_cable = *ptr++;
			packet.evnt0 = *ptr++;
			packet.evnt1 = *ptr++;
			packet.evnt2 = *ptr++;

			uint8_t type = packet.evnt0;
			uint8_t subtype = packet.evnt1;

			if (packet.cin_cable != 0) {
				BSP_LED_On(LED_BLUE);
			}

			if ((type & 0xf0) == 0xb0) {
				uint8_t val = packet.evnt2;
				switch (subtype) {
				case 41:
					BSP_LED_On(LED_ORANGE);
					break;
				case 42:
					BSP_LED_Off(LED_ORANGE);
					break;
				default:
					break;
				}
			}
		}
	}
}

void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost) {
	processMidiPackets();
	USBH_MIDI_Receive(&hUSBHost, midiReceiveBuffer, MIDI_BUF_SIZE); // start a new reception
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
}
