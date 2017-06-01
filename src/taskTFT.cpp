/**
* @brief 
* @file taskTFT.cpp
* @author J.H. 
* @date 2017-05-23
*/

/* module header */
#include "taskTFT.h"

/* system includes C */

/* system includes C++ */
#include "ILI9341_t3.h"


/* local includes */
#include "menu.h"
#include "seq.h"
#include "config.h"
#include "assert.h"
#include "macros.h"
#include "fixedpoint.h"

#include "arm_math.h"
#include "audio/utility/dspinst.h"


/* temp includes - to be removed */
#include "instruments/uton.h"
#include "rack.h"

namespace TFT {

#define BUFSIZE		64
#define MENU_ITEMS 	3

//#define AUDIO_SAMPLE_RATE_EXACT 44100.0
#define AUDIO_SAMPLE_RATE_EXACT 44117.64706

static TaskHandle_t s_tftTask = NULL;

class IUton : public Rack::Instrument
{
	public:
		int16_t update() 	{ return m_i.next(); }
		void	trigger() 	{ m_i.trigger(); }
	
		Uton m_i;
		
};
static IUton s_uton;

static void printSM(ILI9341_t3 &tft, const Menu::sm_item_t &item)
{
	if(item.flags & Menu::sm_item_t::ITEM_SELECTED) 			tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK); // hilighted
	else tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); // default. ..
	tft.print(item.text1);
	tft.print(" ");
	if (item.flags & Menu::sm_item_t::ITEM_EDIT)				tft.setTextColor(ILI9341_BLACK, ILI9341_CYAN); // edit val
	tft.println(item.text2);
}

static Menu::sm_item_t s_submenu_items[] = {
	{ "oscF", "???", 0, +[](void) { s_uton.m_i.setOscFreq(s_uton.m_i.oscFreq()-10); } , 
						+[](void) { s_uton.m_i.setOscFreq(s_uton.m_i.oscFreq()+10); }, NULL },
	{ "oscD", "???", 0},
	{ "noiF", "???", 0, +[](void) { s_uton.m_i.setNoiseCutoff(s_uton.m_i.noiseCutoff()-10); } , 
						+[](void) { s_uton.m_i.setNoiseCutoff(s_uton.m_i.noiseCutoff()+10); }, NULL },
	{ "noiQ", "???", 0, +[](void) { s_uton.m_i.setNoiseQ(s_uton.m_i.noiseQ()-0xff); } , 
						+[](void) { s_uton.m_i.setNoiseQ(s_uton.m_i.noiseQ()+0xff); }, NULL },
	{ "noiT", "???", 0, +[](void) { s_uton.m_i.setNoiseFilterType((s_uton.m_i.noiseFilterType()-1) % 3); } , 
						+[](void) { s_uton.m_i.setNoiseFilterType((s_uton.m_i.noiseFilterType()+1) % 3); }, NULL },
	{ "noiD", "???", 0},
	{ "mix", "???", 0, 	+[](void) { s_uton.m_i.setMix(s_uton.m_i.mix()-0x1ff); } , 
						+[](void) { s_uton.m_i.setMix(s_uton.m_i.mix()+0x1ff); }, NULL }
};

static void printInstrument1(ILI9341_t3 &tft)
{
	Menu::sm_item_t *items = Menu::g_submenu_uton.items;
	sniprintf(items[0].text2, COL1_TEXT_MAXLEN, "%-5d", s_uton.m_i.oscFreq());
	sniprintf(items[1].text2, COL1_TEXT_MAXLEN, "%-5d", s_uton.m_i.oscDecay());
	sniprintf(items[2].text2, COL1_TEXT_MAXLEN, "%-5d", s_uton.m_i.noiseCutoff());
	sniprintf(items[3].text2, COL1_TEXT_MAXLEN, "%1d.%-5d", fp2int(s_uton.m_i.noiseQ(), 14), frac2int(s_uton.m_i.noiseQ(), 14));
	sniprintf(items[4].text2, COL1_TEXT_MAXLEN, "%s", s_uton.m_i.noiseFilterTypeStr() );
	sniprintf(items[5].text2, COL1_TEXT_MAXLEN, "%-5d", s_uton.m_i.noiseDecay() );
	sniprintf(items[6].text2, COL1_TEXT_MAXLEN, "%-5d", s_uton.m_i.mix() );

	for(size_t i = 0; i < Menu::g_submenu_uton.nitems; i++) {
		printSM(tft, items[i]);
	}

}

static void tftTask(void *pvParameters)
{
	ILI9341_t3 tft = ILI9341_t3(10, 9);
	tft.begin();
	tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
	tft.setTextSize(2);
	tft.setRotation(3);
	menu_t *_menu = Menu::get();
	Seq::pattern_t *pattern = Seq::getPattern();


	while(1)
	{
//        tft.fillScreen(ILI9341_BLACK);
		tft.setCursor(0,0);

		for(int i = 0; i < INSTRUMENTS_MAX; i++) {
			
			for(int j = 0; j < SEQ_STEPS; j++) {
				const char *c = NULL;
				int color = ILI9341_YELLOW;
				if((_menu->cur_state == (menu_t::I_PATTERN_1 + i)) || (_menu->cur_state == (menu_t::I_PATTERN_1_EDIT + i))) 		color =  ILI9341_CYAN;
				if((_menu->cur_state == (menu_t::I_PATTERN_1_EDIT + i) && (j == _menu->index)))	color =  ILI9341_GREEN;
				if(pattern->idx == j) 	tft.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
				else					tft.setTextColor(color, ILI9341_BLACK);
				if(pattern->pat[i][j].trigger == 0)  	 c = ".";
				else						 		 	 c = "O";
				tft.print(c);
			}
			tft.print("\n");
		}
		tft.print("\n");
		switch(_menu->cur_state) {
			case menu_t::I_PATTERN_1: 
				printInstrument1(tft);
			break;
			default:
			break;
		}
//                case menu_t::I_BPM: 	
//                    tft.print("bpm "); 
//                    sniprintf(buf, BUFSIZE, "%-3d", Cfg::get()->bpm);
//                    if (_menu->cur_state == menu_t::I_SET_BPM) tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
//                    
//                    tft.print(buf); 
//                break;
//                case menu_t::I_X: 		tft.print("x"); 
//                    sniprintf(buf, BUFSIZE, " %6d", Cfg::get()->x);
//                    if (_menu->cur_state == menu_t::I_SET_X) tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
//                    tft.print(buf); 
//                break;
//                case menu_t::I_PLAYING:	
//                    tft.print((Cfg::get()->playing) ? "play" : "stop");
//                default: break;
//            tft.println();
		
		ulTaskNotifyTake( pdTRUE, 200);
		}
}



void init() {
Menu::g_submenu_uton.items = s_submenu_items;
Menu::g_submenu_uton.nitems = NITEMS(s_submenu_items) ;
	Rack::setInstrument(0, &s_uton);	
	assert(s_tftTask == NULL);
	xTaskCreate( tftTask, "led", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY, &s_tftTask ); 
}

void update() {
	xTaskNotifyGive(s_tftTask);
}

TaskHandle_t getTask()
{
	return s_tftTask;
}

}
