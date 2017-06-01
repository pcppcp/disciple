#ifndef _VCA_H_
#define _VCA_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}



class VCA {
	public:
		VCA() : m_acc(0), m_inc(0) {}
		uint16_t next() {
			uint16_t ret = m_acc >> 16;
			if(m_acc < m_inc) {
				m_acc = 0;
				return 0;
			}
			m_acc -= m_inc;
			return ret;
		}
		void setDecay(uint16_t d) {
			m_inc = d * (UINT32_MAX / DDS_SAMPLE_RATE);
		}
		void setDecaySamples(uint16_t s) {
			m_inc = UINT32_MAX / s;					
		}
			
		void trigger() { m_acc = UINT32_MAX; }

		uint32_t m_acc;
		uint32_t m_inc;
};

#endif

#endif /* _VCA_H_ */

