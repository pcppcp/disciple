#ifndef _RACK_H_
#define _RACK_H_
/* system includes */
/* local includes */
#include <stdint.h>


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace Rack {

class Instrument
{
	public:
		// get next sample from an instrument
		virtual int16_t update() { return 0; }
		virtual void setPar1(const int16_t val) {}
		virtual void setPar2(const int16_t val) {}
		virtual int32_t getPar1() const { return 0; }
		virtual void trigger() {}
};

class InstrumentRack
{
	public:
		InstrumentRack();
		int16_t update(); // return a sample from all instruments mixed together
		void trigger(const uint8_t index); // trigger an instrument at index
		void setPar1(const uint8_t index, const uint16_t val); 
		int32_t getPar1(const uint8_t index) const;

		static const uint8_t MAX_INSTRUMENTS = 4;

		void setInstrument(const uint8_t index, Instrument *instr);
	private:
		Instrument* m_instruments[MAX_INSTRUMENTS];
};

int16_t update();
void trigger(const uint8_t idx);
void setPar1(const uint8_t idx, const uint16_t val);
int32_t getPar1(const uint8_t idx);
void setInstrument(const uint8_t idx, Instrument *instr);

}
#endif

#endif /* _RACK_H_ */

