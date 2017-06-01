#ifndef _HW_H_
#define _HW_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define HW_BUTTONS	6

namespace HW {

typedef struct {
	volatile uint8_t val:2;
	// state values for button debouncing
	volatile uint8_t s:1;
	volatile uint8_t i:4;
} button_t;

typedef struct _enc {
	_enc() : state(0), dir(0) {}
	uint8_t state;
	int8_t dir;
} enc_t;

typedef struct _hw {
	_hw();
	uint8_t led0:1;
	uint8_t led1:1;
	uint8_t led2:1;
	uint8_t led3:1;

	uint8_t ledPCB:1;

	uint8_t led0_blink;
	uint8_t led1_blink;
	uint8_t led2_blink;
	uint8_t led3_blink;
	uint8_t ledPCB_blink;


	button_t buttons[HW_BUTTONS];

	enc_t	 enc0;
	enc_t	 enc1;


	enum Led {
		LED_SEQ_0  = 0,
		LED_SEQ_1  = 1,
		LED_SEQ_2  = 2,
		LED_SEQ_3  = 3,
		LED_PCB	   = 4
	};

	enum ButtonState {
		BUTTON_STATE_NULL		=	0,
		BUTTON_STATE_PRESSED	=	1,
		BUTTON_STATE_RELEASED	=	2,
		BUTTON_STATE_HOLD		=	3
	};
	enum ButtonId {
		BUTTON_ID_0	= 0,
		BUTTON_ID_1	= 1,
		BUTTON_ID_2	= 2,
		BUTTON_ID_3	= 3,
		BUTTON_ID_ENC0 = 4,
		BUTTON_ID_ENC1 = 5
	};
	enum EncoderId {
		ENCODER_ID_0 = 0,
		ENCODER_ID_1 = 0
	};
} hw_t;

// run once to init registers, clocks etc.
extern int init();
extern void resetUpdateTimer();


typedef void (*buttonEventCB_ft)(hw_t::ButtonId b, hw_t::ButtonState s);
typedef void (*encoderEventCB_ft)(const int8_t dir);

extern void setButtonEventCB(buttonEventCB_ft f);
extern void setEncoderEventCB(const uint8_t enc_no, encoderEventCB_ft f);
extern void setLed(hw_t::Led led, bool s);
extern void setLedBlink(hw_t::Led led, uint8_t blink);

extern hw_t *getHW();


}

#endif

#endif /* _HW_H_ */

