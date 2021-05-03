/**
 ** Utils for managing noise in the signal
 **
 ** $Id: noise.h 4 2008-04-24 21:27:41Z andrew $
 **/
#ifndef __NOISE_HEADER__
#define __NOISE_HEADER__

#include "os_defs.h"

# ifndef        MAKEDEPEND
# include       <stdio.h>
# endif

/**
 ** Forward Declarations
 **/
OS_EXPORT int addNoiseToBuffer(
		                float *buffer,
		                int bufferLen,
		                double samplingRate,
		                double signalToNoiseRatio
		            );

#endif /* __NOISE_HEADER__ */

