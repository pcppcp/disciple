#ifndef _SNARE_H_
#define _SNARE_H_
/* system includes */
/* local includes */
#include "mk20dx128.h"
#include "src/random.h"
#include "audio/filter_variable.h"
#include "audio/utils.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
class SnareDrum {
	public:
		SnareDrum() : m_vca(), m_baseF(5000) { 
			m_vco.setType(DDS::SQUARE);
			m_vco.setFrequency((uint16_t) 220);
			m_vca.setDecaySamples(DDS_SAMPLE_RATE / 4); 
			m_lpf.frequency(m_baseF); 
			m_lpf.resonance(7); 
		}
		int16_t next() {
			int16_t sample = xor_rand(0xffff);
//            sample += m_vco.next();
//            int16_t env = m_vca.next();
//            int32_t tmp = (sample * env) >> 12; 
//            m_lpf.frequencyX((env * 22050) >> 16);
//            m_vco.setFrequency((uint16_t)((env * m_baseF) >> 16));
			int16_t lp, bp, hp;
			m_lpf.update(sample, lp, bp, hp);
          	return hp;

		}
		void setCutoffFreq(const uint16_t f) { m_lpf.frequencyX(f); }
		int32_t getX() const {return m_lpf.getFmult(); }
		void trigger() {
			m_vca.m_acc = UINT32_MAX;
		}
	private:
		DDS m_vco;
		VCA m_vca;
		uint16_t m_baseF;
		AudioFilterStateVariable	m_lpf;
};
#endif

#endif /* _SNARE_H_ */

