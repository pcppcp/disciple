#ifndef _UTON_H_
#define _UTON_H_
/* system includes */
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#include "audio/utils.h"
#include "audio/vca.h"
#include "audio/filter_variable.h"
#include "src/random.h"

// drum model inspired by microtonic.
// 					noise 	-> filter -> envelope |
//											 	  +-> mixer 
//  modulator ->	osc		-> 		  -> envelope |

//
// feature list
//  [x] OSC
//  [ ] OSC pitchmod [ ] amount [ ] rate [ ] type

//	[x] noise source
//	[x] noise filter	[x] type 	[x] frequency	[ ] Q
//	[ ] noise envelope 	[x] linear 	[ ] exp 		[ ] retrig

//	[x] mix
//	[ ] mix EQ [ ] freq [ ] gain
//	[ ] distort


class Uton {
    public:
        Uton() : m_vco(), m_vcaOSC(), 
			m_oscFreq(440), m_oscDecay(DDS_SAMPLE_RATE / 2),
			m_noiseDecay(DDS_SAMPLE_RATE / 2),  m_noiseCutoff(300),m_noiseQ(0x5a85), m_noiseFilterType(0),
			m_mix(0xffff>>1)
		{ 
			m_vco.setFrequency(m_oscFreq); 
			m_vcaOSC.setDecaySamples(m_oscDecay); 
			m_vcaNoise.setDecaySamples(m_noiseDecay); 
			m_SVF_noise.frequency(m_noiseCutoff);
			m_SVF_noise.resonanceX(m_noiseQ);
		}
        int16_t next() {
			// oscillator
			int32_t osc_sample 		=  m_vco.next();
			uint16_t env =  m_vcaOSC.next();
			osc_sample = (osc_sample * env) >> 16;
			
			uint32_t nextF = (env * m_oscFreq) >> 16;
			m_vco.setFrequency((uint16_t)nextF);

			// noise
			int32_t noise_sample 	= xor_rand(0xffff);
			int16_t lp, hp, bp;
			m_SVF_noise.update(noise_sample, lp, hp, bp);
			m_SVF_noise.frequencyX((env * m_noiseCutoff) >> 16);
			switch(m_noiseFilterType) {
				case 0: noise_sample = lp; break;
				case 1: noise_sample = bp; break;
				case 2: 
				default:
						noise_sample = hp; break;
			}

			noise_sample = (noise_sample * env) >> 16;
			
			
			// mix
			noise_sample = (noise_sample * (0xffff - m_mix)) >> 16;
			osc_sample = (osc_sample * (m_mix)) >> 16;
			return osc_sample + noise_sample; // + noise_sample;
        } 
        void trigger() {
			m_vco.m_acc = 0;
			m_vcaOSC.trigger();
			m_vcaNoise.trigger();
        }
		void setMix			(const uint16_t mix) 	{ m_mix = mix; }

		void setOscFreq		(const uint16_t par)	{ m_oscFreq = par; m_vco.setFrequency((uint16_t)m_oscFreq); }
		void setOscDecay	(const uint16_t par)	{ m_oscDecay = par; m_vcaOSC.setDecaySamples(par); }
		void setNoiseDecay	(const uint16_t par)	{ m_vcaNoise.setDecaySamples(par); }
		void setNoiseCutoff (const uint16_t par)	{ m_noiseCutoff = par; m_SVF_noise.frequency(par); }
		void setNoiseQ 		(const uint16_t par)	{ m_noiseQ = par; m_SVF_noise.resonanceX(par << 16); }
		void setNoiseFilterType (const uint16_t par)	{ m_noiseFilterType = par % 3; }
		
		uint16_t oscFreq() const 			{ return m_oscFreq; }
		uint16_t oscDecay() const 			{ return m_oscDecay; }
		uint16_t noiseDecay() const 		{ return m_noiseDecay; }
		uint16_t noiseCutoff() const 		{ return m_noiseCutoff; }
		uint16_t noiseQ() const 		{ return m_noiseQ; }
		uint8_t noiseFilterType() const 	{ return m_noiseFilterType; }
		uint16_t mix() const				{ return m_mix ; } 

		const char * noiseFilterTypeStr()	{
			switch(m_noiseFilterType) {
				case 0:	return "lp";
				case 1:	return "bp";
				case 2:	return "hp";
				default: return "??";
			}	
		}

    private:
        DDS m_vco;
        VCA m_vcaOSC;
        VCA m_vcaNoise;
	
		AudioFilterStateVariable	m_SVF_noise;

		uint16_t 	m_oscFreq;
		uint16_t	m_oscDecay;
		uint16_t	m_noiseDecay;
		uint16_t	m_noiseCutoff;
		uint16_t	m_noiseQ;
		uint8_t 	m_noiseFilterType; // 0 = lp, 1 = bp, 2 = hp
		uint16_t 	m_mix;
};
#endif

#endif /* _UTON_H_ */

