#include <stdlib.h>

#include "macros.h"
#include "mk20dx128.h"
#include "FreeRTOS.h"
#include "task.h"
#include "kinetis.h"
#include "random.h"
#include "hw.h"
#include "assert.h"

#include "seq.h"
#include "dsp_out.h"
#include "config.h"


#include "taskTFT.h"
#include "menu.h"
#include "rack.h"



static TaskHandle_t _seqTask = NULL;


static void seqTask(void *)
{
	Seq::pattern_t *pattern = Seq::getPattern();
	Cfg::cfg_t *cfg = Cfg::get();

	while(1) {
		if(!cfg->playing) { ulTaskNotifyTake(pdTRUE, 1000); continue; }
		Seq::pattern_inc(*pattern);
		Rack::setPar1(1, cfg->x);
		for(int i = 0; i < Rack::InstrumentRack::MAX_INSTRUMENTS; i++) {
			const Seq::seq_step_t *step = Seq::pattern_get_step(*pattern, i);
			if((step != NULL) && (step->trigger)) Rack::trigger(i);
		}
		TFT::update();
		ulTaskNotifyTake( pdTRUE, (1000*60) / cfg->bpm / 4);
	}
}

static void buttonEventCB(HW::hw_t::ButtonId b, HW::hw_t::ButtonState s)
{
	Seq::pattern_t *pattern = Seq::getPattern();

//    switch(b) {
//        case HW::hw_t::BUTTON_ID_0: i = 0; break;
//        case HW::hw_t::BUTTON_ID_1: i = 1; break;
//        case HW::hw_t::BUTTON_ID_2: i = 2; break;
//        case HW::hw_t::BUTTON_ID_3: i = 3; break;
//        default: assert(false);
//    }
	Cfg::cfg_t *cfg = Cfg::get();
	menu_t *_menu = Menu::get();
	if (s == HW::hw_t::BUTTON_STATE_RELEASED) {
		if(b == HW::hw_t::BUTTON_ID_ENC0) {
			switch(_menu->cur_state) {
				case menu_t::I_BPM: 	_menu->cur_state = menu_t::I_SET_BPM; break;
				case menu_t::I_SET_BPM: _menu->cur_state = menu_t::I_BPM; break;
				case menu_t::I_X: 		_menu->cur_state = menu_t::I_SET_X; break;
				case menu_t::I_SET_X: _menu->cur_state = menu_t::I_X; break;
				case menu_t::I_PLAYING: cfg->playing = !cfg->playing; xTaskNotifyGive(_seqTask); break;

				// this is retarded
				case menu_t::I_PATTERN_1: _menu->cur_state = menu_t::I_PATTERN_1_EDIT; _menu->index = 0; break;
				case menu_t::I_PATTERN_2: _menu->cur_state = menu_t::I_PATTERN_2_EDIT; _menu->index = 1; break;
				case menu_t::I_PATTERN_3: _menu->cur_state = menu_t::I_PATTERN_3_EDIT; _menu->index = 2; break;
				case menu_t::I_PATTERN_4: _menu->cur_state = menu_t::I_PATTERN_4_EDIT; _menu->index = 3; break;
				case menu_t::I_PATTERN_1_EDIT: _menu->cur_state = menu_t::I_PATTERN_1; break;
				case menu_t::I_PATTERN_2_EDIT: _menu->cur_state = menu_t::I_PATTERN_2; break;
				case menu_t::I_PATTERN_3_EDIT: _menu->cur_state = menu_t::I_PATTERN_3; break;
				case menu_t::I_PATTERN_4_EDIT: _menu->cur_state = menu_t::I_PATTERN_4; break;
					
				default: break;
			}
		} else
		if(b == HW::hw_t::BUTTON_ID_ENC1) {
				Menu::menu_action(Menu::g_submenu_uton);
		} else
		if(b == HW::hw_t::BUTTON_ID_0) {
			switch(_menu->cur_state) {
				case menu_t::I_PATTERN_1_EDIT: 	pattern->pat[0][_menu->index].trigger = !pattern->pat[0][_menu->index].trigger ;break;
				case menu_t::I_PATTERN_2_EDIT: 	pattern->pat[1][_menu->index].trigger = !pattern->pat[1][_menu->index].trigger ;break;
				case menu_t::I_PATTERN_3_EDIT: 	pattern->pat[2][_menu->index].trigger = !pattern->pat[2][_menu->index].trigger ;break;
				case menu_t::I_PATTERN_4_EDIT: 	pattern->pat[3][_menu->index].trigger = !pattern->pat[3][_menu->index].trigger ;break;
				default: break;
			}
		}
	}
}

static void encoder0EventCB(const int8_t dir)
{
	static int8_t counter = 0;

	TaskHandle_t _tftTask = TFT::getTask();
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	Cfg::cfg_t *cfg = Cfg::get();
	counter += dir;
	if((abs(counter) % 4) != 0) return;

	menu_t *_menu = Menu::get();

	switch(_menu->cur_state) {
		case menu_t::I_BPM: 
		case menu_t::I_X: 
		case menu_t::I_PLAYING: 
		case menu_t::I_PATTERN_1: 
		case menu_t::I_PATTERN_2: 
		case menu_t::I_PATTERN_3: 
		case menu_t::I_PATTERN_4: 
			_menu->index += dir;
			if(_menu->index > NITEMS(_menu->level1)) _menu->index = NITEMS(_menu->level1)-1;
			_menu->index %= NITEMS(_menu->level1);
			_menu->cur_state = _menu->level1[_menu->index];
		break;
		case menu_t::I_SET_BPM:		cfg->bpm += dir; 						break;
		case menu_t::I_SET_X:		if(cfg->x < 0xff*dir) cfg->x = 0; else cfg->x += 0xff*dir;
									break;
		case menu_t::I_PATTERN_1_EDIT:
		case menu_t::I_PATTERN_2_EDIT:
		case menu_t::I_PATTERN_3_EDIT:
		case menu_t::I_PATTERN_4_EDIT:
			_menu->index += dir;
			_menu->index %= 16;
		break;

		default:
			_menu->cur_state = menu_t::I_BPM;
	}



	vTaskNotifyGiveFromISR( _tftTask, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

static void encoder1EventCB(const int8_t dir)
{
	static int8_t counter = 0; // callback is called four times per dent

	TaskHandle_t _tftTask = TFT::getTask();
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	counter += dir;
	if((abs(counter) % 4) != 0) return;
	if(dir == 1) 	Menu::menu_next(Menu::g_submenu_uton);
	if(dir == -1) 	Menu::menu_prev(Menu::g_submenu_uton);

	
	vTaskNotifyGiveFromISR( _tftTask, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}



extern "C" int main(void)
{
	HW::setButtonEventCB(buttonEventCB);
	HW::setEncoderEventCB(0, encoder0EventCB);
	HW::setEncoderEventCB(1, encoder1EventCB);
	HW::init();
	DSP::init();
	TFT::init();

	Seq::pattern_t *pattern = Seq::getPattern();



	pattern->pat[0][0].trigger = 1;
//    pattern->pat[0][3].trigger = 1;
//    pattern->pat[0][6].trigger = 1;

//    pattern->pat[2][2].trigger = 1;
//    pattern->pat[2][6].trigger = 1;
//    pattern->pat[2][10].trigger = 1;
//    pattern->pat[2][14].trigger = 1;

	// --  create basic tasks --
	xTaskCreate( seqTask, "seq", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, &_seqTask ); // alive LED
	vTaskStartScheduler();
	return 0;

}

