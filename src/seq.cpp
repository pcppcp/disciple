/**
* @brief 
* @file seq.cpp
* @author J.H. 
* @date 2017-05-17
*/

/* module header */
#include "seq.h"

/* system includes C */
#include <stdio.h>

/* system includes C++ */


/* local includes */

namespace Seq {

static pattern_t s_pat;


pattern_t *getPattern()
{
	return &s_pat;
}

void pattern_inc(pattern_t &pat) { pat.idx++; pat.idx %= pat.len; }
const seq_step_t *pattern_get_step(const pattern_t &pat, uint8_t instr)
{
	if(instr > INSTRUMENTS_MAX) return NULL;
	if(pat.idx > SEQ_STEPS)		return NULL;
	return &pat.pat[instr][pat.idx];
}

}
