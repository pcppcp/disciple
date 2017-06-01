/**
* @brief 
* @file rack.cpp
* @author J.H. 
* @date 2017-05-23
*/

/* module header */
#include "rack.h"

/* system includes C */

/* system includes C++ */


/* local includes */
#include "instruments/kick.h"
#include "instruments/Snare.h"
#include "instruments/HHClosed.h"
#include "assert.h"

namespace Rack {


class IKick : public Instrument
{
	public:
		int16_t update() 	{ return m_kdrum.next(); }
		void	trigger() 	{ m_kdrum.trigger(); }
		void 	setPar1(const int16_t p) { m_kdrum.setFreq((uint16_t)p); }
	private:
		KickDrum m_kdrum;
};

class ISnare : public Instrument
{
	public:
		int16_t update() 	{ return m_snare.next(); }
		void	trigger() 	{ m_snare.trigger(); }
		void 	setPar1(const int16_t p) { m_snare.setCutoffFreq(p); }
		int32_t getPar1() const { return m_snare.getX(); }
	private:
		SnareDrum m_snare;
};
class IHHC : public Instrument
{
	public:
		int16_t update() 	{ return m_hh.next(); }
		void	trigger() 	{ m_hh.trigger(); }
	private:
		HHClosed m_hh;
};

//static IKick s_kik1;
//static IKick s_kik2;
//static ISnare s_snare;
//static IHHC	 s_hhc;
static InstrumentRack s_rack;

InstrumentRack::InstrumentRack()
{
	memset(m_instruments, 0, sizeof(m_instruments));
//    s_kik2.setPar1(5000);
	m_instruments[0] = nullptr;
//    m_instruments[1] = &s_kik1;
//    m_instruments[1] = &s_snare;
//    m_instruments[2] = &s_hhc;
	m_instruments[1] = nullptr;
}

int16_t InstrumentRack::update()
{
	int16_t result = 0;
	for(int i = 0; i < MAX_INSTRUMENTS; i++) {
		Instrument *instr = m_instruments[i];
		if(instr == nullptr) break;
		result += instr->update();
	}
	return result;
}

void InstrumentRack::trigger(const uint8_t idx)
{
	assert(idx < MAX_INSTRUMENTS);
	Instrument *instr = m_instruments[idx];
	assert(instr != nullptr);
	if(instr == nullptr) return;
	instr->trigger();
}

void InstrumentRack::setPar1(const uint8_t idx, uint16_t val)
{
	assert(idx < MAX_INSTRUMENTS);
	Instrument *instr = m_instruments[idx];
	assert(instr != nullptr);
	if(instr == nullptr) return;
	instr->setPar1(val);
}
int32_t InstrumentRack::getPar1(const uint8_t idx) const
{
	assert(idx < MAX_INSTRUMENTS);
	Instrument *instr = m_instruments[idx];
	assert(instr != nullptr);
	if(instr == nullptr) return 0;
	return instr->getPar1();
}

void InstrumentRack::setInstrument(const uint8_t index, Instrument *instr)
{
	assert(index < MAX_INSTRUMENTS);
	assert(m_instruments[index] == NULL);

	m_instruments[index] = instr;
}

int16_t update()
{
	return s_rack.update();
}

void trigger(const uint8_t idx)
{
	s_rack.trigger(idx);
}
void setPar1(const uint8_t idx, const uint16_t val)
{
	s_rack.setPar1(idx, val);
}
int32_t getPar1(const uint8_t idx)
{
	return s_rack.getPar1(idx);
}

void setInstrument(const uint8_t idx, Instrument *instr)
{
	return s_rack.setInstrument(idx, instr);
}

}
