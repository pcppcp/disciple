/**
* @brief 
* @file menu.cpp
* @author J.H. 
* @date 2017-05-23
*/

/* module header */
#include "menu.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "macros.h"

static menu_t s_menu;

namespace Menu {

sm_menu_t g_submenu_uton = { NULL, 0 };



menu_t *get() { return &s_menu; }

void menu_init(sm_menu_t &menu) 
{
	for (size_t i = 0; i < menu.nitems; i++) {
		menu.items[i].flags = 0;
	}
	menu.items[0].flags = sm_item_t::ITEM_SELECTED;
}

static size_t _menu_find_selected(const sm_menu_t &menu)
{
	for (size_t i = 0; i < menu.nitems; i++) {
		if(menu.items[i].flags & sm_item_t::ITEM_SELECTED) return i;
	}
	return 0;
}

void menu_next(sm_menu_t &menu)
{
	auto i = _menu_find_selected(menu);
	auto item = &menu.items[i];
	if(item->flags & sm_item_t::ITEM_EDIT) {
		if(item->next_cb != nullptr) item->next_cb();
		return;
	}
	item->flags &= ~(sm_item_t::ITEM_SELECTED | sm_item_t::ITEM_EDIT);
	i = (i == (menu.nitems-1)) ? 0 : i+1; // wrap around
	menu.items[i].flags |= sm_item_t::ITEM_SELECTED;
}
void menu_prev(sm_menu_t &menu)
{
	auto i = _menu_find_selected(menu);
	auto item = &menu.items[i];
	if(item->flags & sm_item_t::ITEM_EDIT) {
		if(item->prev_cb != nullptr) item->prev_cb();
		return;
	}
	item->flags &= ~(sm_item_t::ITEM_SELECTED | sm_item_t::ITEM_EDIT);
	i = (i == 0) ? menu.nitems-1 : i-1; // wrap around
	menu.items[i].flags |= sm_item_t::ITEM_SELECTED;
}
void menu_action(sm_menu_t &menu)
{
	auto i = _menu_find_selected(menu);
	if(menu.items[i].flags & sm_item_t::ITEM_EDIT) 	menu.items[i].flags &= ~sm_item_t::ITEM_EDIT;
	else											menu.items[i].flags |= sm_item_t::ITEM_EDIT;
}


}
