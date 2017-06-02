/**
* @brief 
* @file static_menus.cpp
* @author J.H. 
* @date 2017-06-02
*/

/* module header */
#include "static_menus.h"

/* system includes C */
#include <stdio.h>

/* system includes C++ */


/* local includes */
#include "macros.h"
#include "fixedpoint.h"

namespace Globals {
IUton *g_uton_ptr = NULL;

static Menu::sm_item_t s_submenu_cfg_items[] = {
	{ "bpm", 	"???", 0},
	{ "state", 	"???", 0}
};


// config menu
Menu::sm_menu_t g_submenu_cfg		= {s_submenu_cfg_items, NITEMS(s_submenu_cfg_items)};		

// uton submenu
static Menu::sm_item_t s_submenu_uton_items[] = {
	{ "oscF", "???", 0, +[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setOscFreq(g_uton_ptr->m_i.oscFreq()-10); } , 
						+[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setOscFreq(g_uton_ptr->m_i.oscFreq()+10); }, 
						NULL },
	{ "oscD", "???", 0},
	{ "noiF", "???", 0, +[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setNoiseCutoff(g_uton_ptr->m_i.noiseCutoff()-10); } , 
						+[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setNoiseCutoff(g_uton_ptr->m_i.noiseCutoff()+10); }, 
						NULL },
	{ "noiQ", "???", 0, +[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setNoiseQ(g_uton_ptr->m_i.noiseQ()-0xff); } , 
						+[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setNoiseQ(g_uton_ptr->m_i.noiseQ()+0xff); }, 
						NULL },
	{ "noiT", "???", 0, +[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setNoiseFilterType((g_uton_ptr->m_i.noiseFilterType()-1) % 3); } , 
						+[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setNoiseFilterType((g_uton_ptr->m_i.noiseFilterType()+1) % 3); }, 
						NULL },
	{ "noiD", "???", 0},
	{ "mix", "???", 0, 	+[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setMix(g_uton_ptr->m_i.mix()-0x1ff); } , 
						+[](void) { if(g_uton_ptr == NULL) return; g_uton_ptr->m_i.setMix(g_uton_ptr->m_i.mix()+0x1ff); }, 
						NULL }
};
Menu::sm_menu_t g_submenu_uton 		= {s_submenu_uton_items, NITEMS(s_submenu_uton_items)};
uint8_t g_uton_menu_change = 0; // if true, update_uton_submenu() will set the text

void update_uton_submenu()
{
	if(g_uton_ptr == NULL) return;
	if(g_uton_menu_change == 0) return;
	Menu::sm_item_t *items = g_submenu_uton.items;
	sniprintf(items[0].text2, COL1_TEXT_MAXLEN, "%-5d", g_uton_ptr->m_i.oscFreq());
	sniprintf(items[1].text2, COL1_TEXT_MAXLEN, "%-5d", g_uton_ptr->m_i.oscDecay());
	sniprintf(items[2].text2, COL1_TEXT_MAXLEN, "%-5d", g_uton_ptr->m_i.noiseCutoff());
	sniprintf(items[3].text2, COL1_TEXT_MAXLEN, "%1d.%-5ld", fp2int(g_uton_ptr->m_i.noiseQ(), 14), frac2int(g_uton_ptr->m_i.noiseQ(), 14));
	sniprintf(items[4].text2, COL1_TEXT_MAXLEN, "%s", g_uton_ptr->m_i.noiseFilterTypeStr() );
	sniprintf(items[5].text2, COL1_TEXT_MAXLEN, "%-5d", g_uton_ptr->m_i.noiseDecay() );
	sniprintf(items[6].text2, COL1_TEXT_MAXLEN, "%-5d", g_uton_ptr->m_i.mix() );
	g_uton_menu_change = 0;
}

}
