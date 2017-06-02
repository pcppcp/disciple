#ifndef _STATIC_INSTRUMENTS_H_
#define _STATIC_INSTRUMENTS_H_
/* system includes */
/* local includes */
#include "rack.h"

#include "instruments/uton.h"


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

#define NUTONS	4

class IUton : public Rack::Instrument
{
	public:
		int16_t update() 	{ return m_i.next(); }
		void	trigger() 	{ m_i.trigger(); }
	
		Uton m_i;
		
};
namespace Globals {
	extern IUton g_uton[NUTONS];

}
#endif

#endif /* _STATIC_INSTRUMENTS_H_ */

