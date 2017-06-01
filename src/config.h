#ifndef _CONFIG_H_
#define _CONFIG_H_
/* system includes */
#include <stdint.h>
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace Cfg {

typedef struct _cfg {
	_cfg() : bpm(120), playing(1), x(5512), y(0) {}
	uint8_t bpm;
	uint8_t playing:1;
	uint16_t x;
	uint16_t y;
} cfg_t;

cfg_t * get();

}

#endif

#endif /* _CONFIG_H_ */

