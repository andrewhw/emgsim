/**
 ** Noise Generators are used to add noise to the EMG (or other) 
 ** signal.
 **
 ** $Id: NoiseGenerator.h 4 2008-04-24 21:27:41Z andrew $
 **/
#ifndef __NOISE_GENERATOR_CLASS_HEADER__
#define __NOISE_GENERATOR_CLASS_HEADER__

#include "os_defs.h"

# ifndef        MAKEDEPEND
# include       <stdio.h>
# endif

/**
 ** NoiseGenerator Data structure.
 **/
class NoiseGenerator {
private:
		        double          volume_;
		        float           lastValue_;
		        int                     useMomentum_;

		        long            posCounts_;
		        long            negCounts_;

		        long            countFromLastZeroCrossing_;

public:

		        /** create a NoiseGenerator at the given volume */
		        NoiseGenerator(double volume, int useMomentum_);
		        ~NoiseGenerator();

		        /** generate N units of noise */
		        void addNoise(float *data, int N);

		        /** dump our state */
		        void dump(FILE *dumpfp);
};

#endif

