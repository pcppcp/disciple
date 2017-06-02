#ifndef _STATIC_MENUS_H_
#define _STATIC_MENUS_H_
/* system includes */
/* local includes */
#include "menu.h"
#include "static_instruments.h"


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
namespace Globals {
extern Menu::sm_menu_t g_submenu_uton;
extern Menu::sm_menu_t g_submenu_cfg;

extern uint8_t g_uton_menu_change;
extern IUton *g_uton_ptr;

// convenience function to update text in uton submenu
void update_uton_submenu();
}

#endif

#endif /* _STATIC_MENUS_H_ */

