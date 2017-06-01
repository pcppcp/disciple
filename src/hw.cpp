/**
* @brief 
* @file hw.cpp
* @author J.H. 
* @date 2017-05-17
*/

/* module header */
#include "hw.h"

/* system includes C */
#include "FreeRTOS.h"
#include "timers.h"
#include "mk20dx128.h"

/* system includes C++ */


/* local includes */
#include "macros.h"
#include "assert.h"

#define LED_S_0		D,1
#define LED_S_1		A,12
#define LED_S_2		A,13
#define LED_S_3		D,4
#define LED_PCB_CFG C,5     // teensy pin   13, led on teensy board

#define BUTTON_0	D,0
#define BUTTON_1	A,12
#define BUTTON_2	A,13
#define BUTTON_3	D,4

#define BUTTON_ENC0	D,6
#define BUTTON_ENC1	D,5

#define BUTTON_I_MAX	3

#define ENC0_A	C,2
#define ENC0_B	C,1
#define ENC1_A	B,2
#define ENC1_B	B,3

#define UPDATE_OUTPUT_TIMER_PERIOD	100
#define UPDATE_INPUT_TIMER_PERIOD	20

#define ENCODERS_COUNT	2

namespace HW {

static void enc0_isr(void);
static void enc1_isr(void);

static const int8_t enc_states [] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table

static hw_t 		s_hw; 				// struct to hold info about current hw state
static xTimerHandle s_hwOutputTimer = NULL;	// called to update output (leds...)
static xTimerHandle s_hwInputTimer = NULL;	// called to update input (buttons + encoders...)

static buttonEventCB_ft _s_buttonEvent_cb   = NULL;
static encoderEventCB_ft _s_encoderEvent_cb[ENCODERS_COUNT] = { 0 };

hw_t::_hw() : led0(0), led1(0), led2(0), led3(0), ledPCB(0), led0_blink(0) {}

static void HWUpdateTimerCB(xTimerHandle t)
{
	static uint8_t blinko_counter;
	blinko_counter++;
	blinko_counter = blinko_counter % 10;

//    PIN_SET(LED_PCB_CFG, s_hw.ledPCB);
	PIN_SET(LED_S_1, 	s_hw.led1);
	PIN_SET(LED_S_2, 	s_hw.led2);
	PIN_SET(LED_S_3, 	s_hw.led3);

	if((s_hw.led0_blink & blinko_counter)) {
		PIN_TOGGLE(LED_S_0);
	} else {
		PIN_SET(LED_S_0, 	s_hw.led0);
	}
	if((s_hw.led1_blink & blinko_counter)) {
		PIN_TOGGLE(LED_S_1);
	}
	if( (s_hw.led2_blink & blinko_counter)) {
		PIN_TOGGLE(LED_S_2);
	}
	if( (s_hw.led3_blink & blinko_counter)) {
		PIN_TOGGLE(LED_S_3);
	}
//    xTimerChangePeriod(s_hwOutputTimer, UPDATE_OUTPUT_TIMER_PERIOD, 0);
}

static void HWUpdateInputTimerCB(xTimerHandle t)
{
	for(int i = 0; i < HW_BUTTONS; i++) {
		button_t *button_p = &s_hw.buttons[i];
		// -- update button state --
		uint8_t val;
		switch(i) {
			case 0: val = PIN_STATE(BUTTON_0); break;
			case 1: val = PIN_STATE(BUTTON_1); break;
			case 2: val = PIN_STATE(BUTTON_2); break;
			case 3: val = PIN_STATE(BUTTON_3); break;
			case 4: val = PIN_STATE(BUTTON_ENC0); break;
			case 5: val = PIN_STATE(BUTTON_ENC1); break;
		}
		button_p->i = (val == 0) ? MIN(BUTTON_I_MAX, button_p->i+1) : MAX(0, button_p->i-1);
		if(button_p->i == 0) {
			button_p->s = 0;
		} else if(button_p->i >= BUTTON_I_MAX) {
			button_p->s = 1;
			button_p->i = BUTTON_I_MAX;
		}

		button_p->val = (button_p->val << 1) | button_p->s;
		if((_s_buttonEvent_cb != NULL) && 
				((button_p->val == hw_t::BUTTON_STATE_PRESSED) || 
				 (button_p->val == hw_t::BUTTON_STATE_HOLD) || 
				 (button_p->val == hw_t::BUTTON_STATE_RELEASED))) _s_buttonEvent_cb((hw_t::ButtonId)i, (HW::hw_t::ButtonState)button_p->val);
	}

}

int init(void)
{
	assert(s_hwOutputTimer == NULL);

	INIT_OUTPUT(LED_S_0);
//    INIT_OUTPUT(LED_S_1);
//    INIT_OUTPUT(LED_S_2);
//    INIT_OUTPUT(LED_S_3);
//    INIT_OUTPUT(LED_PCB_CFG);
//    PIN_SET(LED_PCB_CFG,1);
	PIN_SET(LED_S_0, 1);
//    PIN_SET(LED_S_1, 1);
//    PIN_SET(LED_S_2, 1);
//    PIN_SET(LED_S_3, 1);

	INIT_INPUT(BUTTON_0);
	INIT_INPUT(BUTTON_1);
	INIT_INPUT(BUTTON_2);
	INIT_INPUT(BUTTON_3);
	INIT_INPUT(BUTTON_ENC0);
	INIT_INPUT(BUTTON_ENC1);

	INIT_INPUT(ENC0_A)
	INIT_INPUT(ENC0_B)
	INIT_INPUT(ENC1_A)
	INIT_INPUT(ENC1_B)
	ENABLE_INTERRUPT(ENC0_A)
	ENABLE_INTERRUPT(ENC0_B)
	ENABLE_INTERRUPT(ENC1_A)
	ENABLE_INTERRUPT(ENC1_B)
	_VectorsRam[IRQ_PORTC+16] = enc0_isr;
	_VectorsRam[IRQ_PORTB+16] = enc1_isr;

	s_hwOutputTimer = xTimerCreate("hw_out", UPDATE_OUTPUT_TIMER_PERIOD,  pdTRUE,  ( void * ) 0, HWUpdateTimerCB);
	s_hwInputTimer = xTimerCreate("hw_in", UPDATE_INPUT_TIMER_PERIOD,  pdTRUE,  ( void * ) 0, HWUpdateInputTimerCB);

	xTimerStart(s_hwOutputTimer, pdFALSE);
	xTimerStart(s_hwInputTimer, pdFALSE);

	return 0;

}

static void enc0_isr(void)
{
	uint32_t isfr = PORTC_ISFR;
	PORTC_ISFR = isfr;
	// TODO: do not update encoder if there is no interrupt on pin


	s_hw.enc0.state <<=2;  //remember previous state
//  s_hw.enc0.state |= (GPIOC_PDIR >> 1) & 0x3;
	s_hw.enc0.state |= (((PIN_STATE(ENC0_B) > 0) << 1) | (PIN_STATE(ENC0_A) > 0));
	s_hw.enc0.dir = enc_states[s_hw.enc0.state & 0xf];
	if((s_hw.enc0.dir != 0) && (_s_encoderEvent_cb != NULL)) _s_encoderEvent_cb[0](s_hw.enc0.dir);
}

static void enc1_isr(void)
{
	uint32_t isfr = PORTB_ISFR;
	PORTB_ISFR = isfr;

	s_hw.enc1.state <<=2;  //remember previous state
	s_hw.enc1.state |= (((PIN_STATE(ENC1_B) > 0) << 1) | (PIN_STATE(ENC1_A) > 0));
	s_hw.enc1.dir = enc_states[s_hw.enc1.state & 0xf];
	if((s_hw.enc1.dir != 0) && (_s_encoderEvent_cb != NULL)) _s_encoderEvent_cb[1](s_hw.enc1.dir);
}

void resetUpdateTimer()
{
	xTimerChangePeriod(s_hwOutputTimer, 1, 0);
	xTimerReset(s_hwOutputTimer, 0);
}

void setButtonEventCB(buttonEventCB_ft f)
{
	_s_buttonEvent_cb = f;
}

void setEncoderEventCB(const uint8_t enc_no, encoderEventCB_ft f)
{
	assert(enc_no < ENCODERS_COUNT);
	_s_encoderEvent_cb[enc_no] = f;
}

void setLed(hw_t::Led led, bool s)
{
	switch(led) {
		case hw_t::LED_SEQ_0: s_hw.led0 = s; PIN_SET(LED_S_0, s); break;
		case hw_t::LED_SEQ_1: s_hw.led1 = s; PIN_SET(LED_S_1, s); break;
		case hw_t::LED_SEQ_2: s_hw.led2 = s; PIN_SET(LED_S_2, s); break;
		case hw_t::LED_SEQ_3: s_hw.led3 = s; PIN_SET(LED_S_3, s); break;
//        case hw_t::LED_PCB:   s_hw.ledPCB = s; PIN_SET(LED_PCB_CFG, s); break;
	}
}

void setLedBlink(hw_t::Led led, uint8_t b)
{
	switch(led) {
		case hw_t::LED_SEQ_0: s_hw.led0_blink = b; break;
		case hw_t::LED_SEQ_1: s_hw.led1_blink = b; break;
		case hw_t::LED_SEQ_2: s_hw.led2_blink = b; break;
		case hw_t::LED_SEQ_3: s_hw.led3_blink = b; break;
		case hw_t::LED_PCB: s_hw.ledPCB_blink = b; break;
	}
}

hw_t *getHW() { return &s_hw;}
}
