#ifndef _MENU_H_
#define _MENU_H_
/* system includes */
#include <stdint.h>
#include <string.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
	enum state {
		I_BPM		= 0,
		I_X			= 1,
		I_PLAYING	= 2,

		I_SET_BPM 	= 10,
		I_SET_X 	= 11,


		I_PATTERN_1 = 20,
		I_PATTERN_2 = 21,
		I_PATTERN_3 = 22,
		I_PATTERN_4 = 23,

		I_PATTERN_1_EDIT = 40,
		I_PATTERN_2_EDIT = 41,
		I_PATTERN_3_EDIT = 42,
		I_PATTERN_4_EDIT = 43
	} cur_state = I_PATTERN_1;

	enum state 			level1[4] 	= {I_PATTERN_1, I_PATTERN_2, I_PATTERN_3, I_PATTERN_4};
	

	uint8_t index=0;
} menu_t;


#ifdef __cplusplus
}

#define COL1_TEXT_MAXLEN	12


namespace Menu {
	typedef void (*menuEventCB_t)(void);

	typedef struct {
		char text1[COL1_TEXT_MAXLEN];
		char text2[COL1_TEXT_MAXLEN];
		uint8_t flags; // read-only, selected, editing
		enum {
			ITEM_RO			= 1<<0,
			ITEM_SELECTED	= 1<<1,
			ITEM_EDIT		= 1<<2
		};
		menuEventCB_t prev_cb;
		menuEventCB_t next_cb;
		menuEventCB_t action_cb;
	} sm_item_t;

	typedef struct {
		sm_item_t 	*items;
		size_t 		nitems;
	} sm_menu_t;

	// move to a next item in menu
	void menu_init(sm_menu_t &menu);
	void menu_next(sm_menu_t &menu);
	void menu_prev(sm_menu_t &menu);
	void menu_action(sm_menu_t &menu);

	extern sm_menu_t g_submenu_uton;

	menu_t *get();
}

#endif

#endif /* _MENU_H_ */

