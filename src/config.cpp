/**
* @brief 
* @file config.cpp
* @author J.H. 
* @date 2017-05-22
*/

/* module header */
#include "config.h"

/* system includes C */

/* system includes C++ */


/* local includes */


namespace Cfg {
	static cfg_t s_cfg;
	cfg_t * get() { return &s_cfg; }
}
