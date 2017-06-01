#ifndef _SEQ_H_
#define _SEQ_H_
/* system includes */
/* local includes */
#include <stdint.h>


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define SEQ_STEPS 		16
#define INSTRUMENTS_MAX	4

namespace Seq {

typedef struct {
	uint8_t trigger:1;
	uint8_t param1;
	uint8_t param2;
} seq_step_t;

typedef struct _pat {
	_pat() : pat(), len(SEQ_STEPS), idx(0) {}
	seq_step_t 	pat[INSTRUMENTS_MAX][SEQ_STEPS];
	uint8_t		len;
	uint8_t		idx;
} pattern_t;



pattern_t *getPattern();
void pattern_inc(pattern_t &pat); 
const seq_step_t *pattern_get_step(const pattern_t &pat, const uint8_t instr);

}

#endif

#endif /* _SEQ_H_ */

