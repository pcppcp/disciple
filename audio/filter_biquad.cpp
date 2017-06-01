/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "mk20dx128.h"	// to -DKINETISK
#include "kinetis.h"

#include "filter_biquad.h"
#include "utility/dspinst.h"
#include <stdio.h>

#include "kinetis.h"

#if 0
int16_t AudioFilterBiquad::update(int16_t s_in)
{
	int32_t b0, b1, b2, a1, a2, sum;
	uint32_t in2, out2, bprev, aprev, flag;
	uint32_t *data, *end;
	int32_t *state;
	state = (int32_t *)definition;
	do {
		b0 = *state++;
		b1 = *state++;
		b2 = *state++;
		a1 = *state++;
		a2 = *state++;
		bprev = *state++;
		aprev = *state++;
		sum = *state & 0x3FFF;
		{
			in2 = s_in;
			sum = signed_multiply_accumulate_32x16b(sum, b0, in2);
			sum = signed_multiply_accumulate_32x16t(sum, b1, bprev);
			sum = signed_multiply_accumulate_32x16b(sum, b2, bprev);
			sum = signed_multiply_accumulate_32x16t(sum, a1, aprev);
			sum = signed_multiply_accumulate_32x16b(sum, a2, aprev);
			out2 = signed_saturate_rshift(sum, 16, 14);
			sum &= 0x3FFF;
			sum = signed_multiply_accumulate_32x16t(sum, b0, in2);
			sum = signed_multiply_accumulate_32x16b(sum, b1, in2);
			sum = signed_multiply_accumulate_32x16t(sum, b2, bprev);
			sum = signed_multiply_accumulate_32x16b(sum, a1, out2);
			sum = signed_multiply_accumulate_32x16t(sum, a2, aprev);
			bprev = in2;
			aprev = pack_16b_16b(
				signed_saturate_rshift(sum, 16, 14), out2);
			sum &= 0x3FFF;
			bprev = in2;
			s_in = aprev;
		} 
		flag = *state & 0x80000000;
		*state++ = sum | flag;
		*(state-2) = aprev;
		*(state-3) = bprev;
	} while (flag);
	return s_in;
}
#endif
int16_t AudioFilterBiquad::update(int16_t s_in)
{
	int32_t b0, b1, b2, a1, a2, sum;
	uint32_t in2, out2, bprev, aprev, flag;
	int32_t *state;
	state = (int32_t *)definition;
	do {
		b0 = *state++;
		b1 = *state++;
		b2 = *state++;
		a1 = *state++;
		a2 = *state++;
		bprev = *state++;
		aprev = *state++;
		sum = *state & 0x3FFF;
// original implementation by teensy processes two sample at the time
// y[n] = 
//	b0 * x[n] + 
//	b1 * x[n-1] + 
//	b2 * x[n-2] + 
//	a1 * y[n-1] + 
//	a2 * y[n-2]
		{
			in2 = s_in;
			sum = signed_multiply_accumulate_32x16b(sum, b0, in2);
			sum = signed_multiply_accumulate_32x16t(sum, b1, bprev);  // bprev[31:16] = x[n-1]
			sum = signed_multiply_accumulate_32x16b(sum, b2, bprev);  // bprev[16:00] = x[n-2]
			sum = signed_multiply_accumulate_32x16t(sum, a1, aprev);  // aprev[31:16] = y[n-1]
			sum = signed_multiply_accumulate_32x16b(sum, a2, aprev);  // aprev[16:00] = y[n-2]
			out2 = signed_saturate_rshift(sum, 16, 14); // val, bits, rshift
			sum &= 0x3FFF; // for next phase
			bprev = ((in2 << 16) | (bprev >> 16));
			aprev = ((out2 << 16)| (aprev >> 16));
			
			s_in = out2;
		} 
		flag = *state & 0x80000000;
		*state++ = sum | flag;
		*(state-2) = aprev;
		*(state-3) = bprev;
	} while (flag);
	return s_in;

}

void AudioFilterBiquad::setCoefficients(uint32_t stage, const int *coefficients)
{
	if (stage >= 4) return;
	int32_t *dest = definition + (stage << 3);
	__disable_irq();
	if (stage > 0) *(dest - 1) |= 0x80000000;
	*dest++ = *coefficients++;
	*dest++ = *coefficients++;
	*dest++ = *coefficients++;
	*dest++ = *coefficients++ * -1;
	*dest++ = *coefficients++ * -1;
	*dest++ = 0;
	*dest++ = 0;
	*dest   &= 0x80000000;
	__enable_irq();
}

