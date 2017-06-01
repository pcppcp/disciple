#ifndef _HHCLOSED_H_
#define _HHCLOSED_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
class HHClosed {
	public:
		HHClosed() : m_vca() { 
			m_vca.setDecaySamples(DDS_SAMPLE_RATE / 12); 
			m_lpf.frequency(17000); 
			m_lpf.resonance(4.0);
		}
		int16_t next() {
			int16_t lp, bp, hp;
			int16_t sample = xor_rand(0xffff);
			m_lpf.update(sample, lp, bp, hp);
			m_lpf.update(hp, lp, bp, hp);
			int16_t env = m_vca.next();
			int32_t tmp = (bp * env) >> 16; 
          	return tmp;

		}
		void trigger() {
			m_vca.m_acc = UINT32_MAX;
		}
	private:
		VCA m_vca;
		AudioFilterStateVariable	m_lpf;
};
#endif

#endif /* _HHCLOSED_H_ */

