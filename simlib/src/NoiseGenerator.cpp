/**
 ** EDF Read/Write tools
 **
 ** $Id: NoiseGenerator.cpp 4 2008-04-24 21:27:41Z andrew $
 **/

#include "os_defs.h"

# ifndef        MAKEDEPEND
#  include      <stdio.h>
#  include      <string.h>
#  include      <math.h>
# endif

#include "NoiseGenerator.h"

#include "random.h"

#include "tclCkalloc.h"
#include "error.h"
#include "massert.h"

static const int        MAX_CYCLES_DEVIATION = 16;

NoiseGenerator::NoiseGenerator(double volume, int useMomentum)
{
	memset(this, 0, sizeof(NoiseGenerator));
	volume_ = volume;
	useMomentum_ = useMomentum;
}

NoiseGenerator::~NoiseGenerator()
{
}

/**
 ** Combine individual MFPs into a NoiseGenerator with jitter info
 ** calculated above
 **
 ** Use:        private
 **/
void
NoiseGenerator::addNoise(float *data, int nElements)
{
	int     i, isReversing;
	double noiseValue, newValue;

	if (volume_ == 0.0)
		return;

	for (i = 0; i < nElements; i++)
	{

		/** pick some values */
		noiseValue = localRandomDouble() * volume_;
		if (useMomentum_)
		{
			isReversing = localRandom() & 0x1000;
			if (isReversing == 0)
			{
				posCounts_++;
			} else
			{
				negCounts_++;
			}

			/*
			 * we want "momentum", but we also want to not deviate
			 * too much from the zero level.  .'. what we do is
			 * continue with "momentum" if isReversing is 0,
			 * otherwise we reverse the direction we are going
			 */
			if (isReversing != 0)
				noiseValue = (-1) * noiseValue;

			newValue = lastValue_ + noiseValue;

			/** if they have opposite signs, they crossed zero */
			if (lastValue_ * newValue <= 0.0)
			{
				countFromLastZeroCrossing_ = 0;
			} else
			{

				/*
				 * if we have exceeded the number of cycles
				 * we wish to maintain before heading back
				 * towards zero, force the sign of the noise
				 * value to take us back zero-ward
				 */
				if (++countFromLastZeroCrossing_ > MAX_CYCLES_DEVIATION)
				{
					if (noiseValue * lastValue_ > 0)
					{
						noiseValue = (-1) * noiseValue;
						newValue = lastValue_ + noiseValue;
					}
				}
			}
			lastValue_ = (float) newValue;

		} else
		{
			lastValue_ = (float) (noiseValue * volume_);
		}

		/** now add into the data */
		data[i] += lastValue_;
	}
}

/**
 ** Dump out what we know about ourselves
 **/
void
NoiseGenerator::dump(FILE *fp)
{
	fprintf(fp, "Noise Generator - volume %f\n", volume_);
	fprintf(fp, "     +ve counts %ld\n", posCounts_);
	fprintf(fp, "     -ve counts %ld\n", negCounts_);
}

