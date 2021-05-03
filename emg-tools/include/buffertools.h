/**
 * Buffer calculation tools
 *
 * $Id: buffertools.h 8 2008-04-24 22:22:33Z andrew $
 */

#ifndef __BUFFER_TOOLS_HEADER__
#define __BUFFER_TOOLS_HEADER__

#include "os_defs.h"
#include "os_types.h"
#include "emg.h"

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

OS_EXPORT int calculateAcceleration(
		float *target,
		const emgValue *src,
		int nValues
	    );

OS_EXPORT int bandpassFilter(
		float *target,
		const emgValue *src,
		int nValues, int filterOrder,
		int bandpass, float deltaT
	    );


typedef struct emgTurn {
	osUint32 durationInSamples;
	double durationInMS;
	double amplitudeInUV;
} emgTurn;

OS_EXPORT int countAndStoreTurns(
		emgTurn **turnBuffer,
		emgValue *buffer,
		osUint32 bufferLen,
		float scale,
		float threshold,
		float samplingRateInHz
	    );

OS_EXPORT int countTurns(
		emgValue *buffer,
		osUint32 bufferLen,
		float scale,
		float threshold
	    );

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif


#endif /* __BUFFER_TOOLS_HEADER__ */

