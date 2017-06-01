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

#ifndef filter_variable_h_
#define filter_variable_h_

#include "mk20dx128.h"	// to -DKINETISK
#include "kinetis.h"

#include <math.h>
#include <stdint.h>
#include "audio/utility/dspinst.h"
#include <arm_math.h>
#define AUDIO_SAMPLE_RATE_EXACT 44117.64706

class AudioFilterStateVariable
{
public:
	AudioFilterStateVariable()   {
		frequency(1000);
		octaveControl(1.0); // default values
		resonance(0.707);
		state_inputprev = 0;
		state_lowpass = 0;
		state_bandpass = 0;
	}
	void frequency(float freq) {
		if (freq < 20.0) freq = 20.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2.5) freq = AUDIO_SAMPLE_RATE_EXACT/2.5;
		setting_fcenter = (freq * (3.141592654/(AUDIO_SAMPLE_RATE_EXACT*2.0)))
			* 2147483647.0;
		// TODO: should we use an approximation when freq is not a const,
		// so the sinf() function isn't linked?
		setting_fmult = sinf(freq * (3.141592654/(AUDIO_SAMPLE_RATE_EXACT*2.0)))
			* 2147483647.0;
	}
	void frequencyX(uint16_t freq) {
		if (freq < 20.0) freq = 20.0;
		else if (freq > AUDIO_SAMPLE_RATE_EXACT/2.5) freq = AUDIO_SAMPLE_RATE_EXACT/2.5;
//        setting_fcenter = (freq * (3.141592654/(AUDIO_SAMPLE_RATE_EXACT*2.0)))
//            * 2147483647.0;
		// TODO: should we use an approximation when freq is not a const,
		// so the sinf() function isn't linked?
		const uint32_t m_pi_sr_2 =  (0x4000/(AUDIO_SAMPLE_RATE_EXACT*2)) * (uint32_t)(1<<31);
		const int32_t freqX = signed_multiply_32x16b(m_pi_sr_2, freq);
		setting_fmult = arm_sin_q31(freqX);
	}
	void resonance(float q) {
		if (q < 0.7) q = 0.7;
		else if (q > 5.0) q = 5.0;
		// TODO: allow lower Q when frequency is lower
		setting_damp = (1.0 / q) * 1073741824.0;
	}
	void resonanceX(uint32_t q) {
		// TODO: allow lower Q when frequency is lower
		setting_damp = q;
	}
	void octaveControl(float n) {
		// filter's corner frequency is Fcenter * 2^(control * N)
		// where "control" ranges from -1.0 to +1.0
		// and "N" allows the frequency to change from 0 to 7 octaves
		if (n < 0.0) n = 0.0;
		else if (n > 6.9999) n = 6.9999;
		setting_octavemult = n * 4096.0;
	}
	int16_t update(const int16_t s);
	int16_t update(const int16_t s, int16_t &lp, int16_t &bp, int16_t &hp);
	int32_t getFmult() const { return setting_fmult; }
private:
	void update_fixed(const int16_t *in,
		int16_t *lp, int16_t *bp, int16_t *hp);
	void update_variable(const int16_t *in, const int16_t *ctl,
		int16_t *lp, int16_t *bp, int16_t *hp);
	int32_t setting_fcenter;
	int32_t setting_fmult;
	int32_t setting_octavemult;
	int32_t setting_damp;
	int32_t state_inputprev;
	int32_t state_lowpass;
	int32_t state_bandpass;
};

#endif
