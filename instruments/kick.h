#ifndef _KICK_H_
#define _KICK_H_
/* system includes */
/* local includes */
#include "audio/utils.h"
#include "audio/vca.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
class KickDrum {
    public:
        KickDrum() : m_vco(), m_vca(), m_baseF(440) { m_vco.setFrequency((uint16_t)m_baseF); m_vca.setDecaySamples(DDS_SAMPLE_RATE / 4); }
        int16_t next() {
            int16_t sample;
            uint16_t env;
            int64_t tmp;
            sample = m_vco.next();
            env = m_vca.next();
            uint16_t nextF = (env * m_baseF) >> 16;
            m_vco.setFrequency(nextF);
            tmp = (sample  *env);
            return tmp >> 16;
        } 
        void trigger() {
            m_vco.m_acc = 0;
			m_vco.setFrequency((uint16_t)m_baseF);
            m_vca.m_acc = UINT32_MAX;
        }
		void setFreq(const uint16_t f) { m_baseF = f; m_vco.setFrequency((uint16_t)m_baseF); }
    private:
        DDS m_vco;
        VCA m_vca;
		uint16_t m_baseF;
};
#endif

#endif /* _KICK_H_ */

