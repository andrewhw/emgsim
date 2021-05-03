/*
 * Calculate acceleration and slope buffer data.
 *
 * $Id: acceleration.c 85 2011-03-29 18:05:57Z andrew $
 */
#include <string.h>

#include "tclCkalloc.h"
#include "mathtools.h"
#include "filtertools.h"

static double 
fromDoublePointer(void *data)
{
    double          result;
    memcpy(&result, data, sizeof(double));
    return result;
}

static double 
fromFloatPointer(void *data)
{
    float           result;
    memcpy(&result, data, sizeof(float));
    return (double) result;
}


static void 
toDoublePointer(void *data, double value)
{
    memcpy(data, &value, sizeof(double));
}

static void 
toFloatPointer(void *data, double value)
{
    float           tmpVal = (float) value;
    memcpy(data, &tmpVal, sizeof(float));
}


/*
 * ----------------------------------------------------------------
 * Zero-phase Acceleration calculation
 */
int 
calculateAccelerationBufferGeneric(
		void *v_target,
		void *v_data,
		int nDataPoints,
		int tileSize,
		double (*fromPointer) (void *),
		void (*toPointer) (void *, double value),
		int sampleDelta,
		void *v_threshold,
		float deltaTime,
		float conversionFactor,
		void *maxSeen
	)
{
	int             i;
	double          threshold = 0;
	double          slopeAhead, slopeBehind;
	double          acceleration;
	double          calcMaxAcceleration = 0;
	int             slopeAheadBegin, slopeAheadEnd;
	int             slopeBehindBegin, slopeBehindEnd;
	int             thresholdOffset = (-1);
	char           *c_target;
	char           *c_data;


	c_target = (char *) v_target;
	c_data = (char *) v_data;

	if (v_threshold != NULL)
	{
		threshold = (*fromPointer) (v_threshold);
	}
	/*
	 * zero out the ends of the buffer
	 */
	if (v_target != NULL)
	{
		memset(&c_target[0], 0, sampleDelta * tileSize * 2);
		memset(&c_target[(nDataPoints * tileSize) -
						 (sampleDelta * tileSize * 2 + 1)],
			   0, sampleDelta * tileSize * 2);
	}
	/**
	 * First data point simply gets the slope
	 * between it and the second point
	 */
	for (i = sampleDelta; i < nDataPoints - (2 * sampleDelta); i++)
	{

		slopeAheadBegin = i;
		slopeAheadEnd = i + 2 * sampleDelta;
		slopeAhead =
			((*fromPointer) (&c_data[slopeAheadEnd * tileSize]) -
			 (*fromPointer) (&c_data[slopeAheadBegin * tileSize]))
			/ (2.0 * sampleDelta * deltaTime);
		/*
			slope2 = (inbuf[i+2*bandpass]-inbuf[i]) / (float)(2.0 * bandpass * deltaT);
		*/

		slopeBehindBegin = i + sampleDelta;
		slopeBehindEnd = i - sampleDelta;
		slopeBehind =
			((*fromPointer) (&c_data[slopeBehindEnd * tileSize]) -
			 (*fromPointer) (&c_data[slopeBehindBegin * tileSize]))
			/ (2.0 * sampleDelta * deltaTime);
		/*
			slope1 = (inbuf[i+bandpass]-inbuf[i-bandpass])/(float)(2.0 * bandpass * deltaT);
		*/



		/** calculate the difference in slope */
		acceleration = ((slopeAhead - slopeBehind)
						/ (sampleDelta * deltaTime)) * conversionFactor;
		/*
		outbuf[i] = (long)((slope2 - slope1)/( bandpass * deltaT ) * (GetConversionFactor(INPUT)/1000000.0) );
		*/
		if (calcMaxAcceleration < acceleration)
			calcMaxAcceleration = acceleration;



		/** if we are supposed to save to a buffer, do so */
		if (v_target != NULL)
		{
			toPointer(&c_target[i * tileSize], acceleration);
		}
		/** if we are supposed to be watching for a threshold, do so */
		if (v_threshold != NULL)
		{
			if (thresholdOffset < 0 && acceleration > threshold)
			{
				thresholdOffset = i;

				/*
				 * if we are watching a threshold with no target
				 * buffer, then quit once we have exceeded the
				 * threshold value
				 */
				if (v_target == NULL)
					break;
			}
		}
	}

	if (maxSeen != NULL)
		toPointer(maxSeen, calcMaxAcceleration);

	return thresholdOffset;
}

OS_EXPORT int 
calculateAccelerationBuffer(
		float *acceleration,
		float *data,
		int nDataPoints,
		int sampleDelta,
		float deltaTime,
		float conversionFactor
	)
{
	return calculateAccelerationBufferGeneric(
			(void *) acceleration,
			(void *) data,
			nDataPoints,
			sizeof(float),
			fromFloatPointer,
			toFloatPointer,
			sampleDelta, NULL,
			deltaTime,
			conversionFactor,
			NULL);
}

OS_EXPORT int 
getOffsetWhereThresholdExceeded(
		float *data,
		int nDataPoints,
		int sampleDelta,
		float threshold,
		float deltaTime,
		float conversionFactor,
		float *maxSeen
	)
{
	return calculateAccelerationBufferGeneric(
			NULL,
			(void *) data,
			nDataPoints,
			sizeof(float),
			fromFloatPointer,
			toFloatPointer,
			sampleDelta, &threshold,
			deltaTime, conversionFactor,
			maxSeen);
}


OS_EXPORT int 
calculateAccelerationBufferDouble(
		double *acceleration,
		double *data,
		int nDataPoints,
		int sampleDelta,
		float deltaTime,
		float conversionFactor
	)
{
	return calculateAccelerationBufferGeneric(
			(void *) acceleration,
			(void *) data,
			nDataPoints,
			sizeof(double),
			fromDoublePointer,
			toDoublePointer,
			sampleDelta, NULL,
			deltaTime,
			conversionFactor,
			NULL);
}

OS_EXPORT int 
getOffsetWhereThresholdExceededDouble(
		double *data,
		int nDataPoints,
		int sampleDelta,
		double threshold,
		float deltaTime,
		float conversionFactor,
		double *maxSeen
	)
{
	return calculateAccelerationBufferGeneric(
			NULL,
			(void *) data,
			nDataPoints,
			sizeof(double),
			fromDoublePointer,
			toDoublePointer,
			sampleDelta, &threshold,
			deltaTime,
			conversionFactor,
			maxSeen);
}

/*
 * ----------------------------------------------------------------
 * Zero-phase slope calculation.  To get the slope at a point,
 * calculate the slopes on either side and average for the tangential
 * slope
 */

int 
calculateSlopeBufferGeneric(
		void *v_target,
		void *v_data,
		int nDataPoints,
		int tileSize,
		double (*fromPointer) (void *),
		void (*toPointer) (void *, double value),
		int sampleDelta,
		void *v_threshold
	)
{
	int             i;
	double          threshold = 0;
	double          slope;
	int             slopeBegin, slopeEnd;
	int             thresholdOffset = (-1);
	char           *c_target;
	char           *c_data;


	c_target = (char *) v_target;
	c_data = (char *) v_data;

	if (v_threshold != NULL)
	{
		threshold = (*fromPointer) (v_threshold);
	}
	/*
	 * zero out the ends of the buffer
	 */
	if (v_target != NULL)
	{
		memset(&c_target[0], 0, sampleDelta * tileSize);
		memset(&c_target[(nDataPoints * tileSize) -
						 (sampleDelta * tileSize + 1)],
			   0, sampleDelta * tileSize);
	}
	/*
	 * Calculate the slope based on delta ahead and behind
	 */
	for (i = sampleDelta; i < nDataPoints - (2 * sampleDelta); i++)
	{

		slopeBegin = i - sampleDelta;
		slopeEnd = i + sampleDelta;
		slope =
			((*fromPointer) (&c_data[slopeEnd * tileSize]) -
			 (*fromPointer) (&c_data[slopeBegin * tileSize]))
			/ (sampleDelta * 2);

		/* if we are supposed to save to a buffer, do so */
		if (v_target != NULL)
		{
			toPointer(&c_target[i * tileSize], slope);
		}
		/* if we are supposed to be watching for a threshold, do so */
		if (v_threshold != NULL)
		{
			if (thresholdOffset < 0 && slope > threshold)
			{
				thresholdOffset = i;

				/*
				 * if we are watching a threshold with no target
				 * buffer, then quit once we have exceeded the
				 * threshold value
				 */
				if (v_target == NULL)
					break;
			}
		}
	}

	return thresholdOffset;
}


/*
 * Return a buffer of slope values based on a data buffer
 */
OS_EXPORT int 
calculateSlopeBuffer(
		float *slope,
		float *data,
		int nDataPoints,
		int sampleDelta
	)
{
	return calculateSlopeBufferGeneric(
			(void *) slope,
			(void *) data,
			nDataPoints,
			sizeof(float),
			fromFloatPointer,
			toFloatPointer,
			sampleDelta, NULL);
}


/*
 * Return a buffer of slope values based on a data buffer
 */
OS_EXPORT int 
calculateSlopeBufferDouble(
		double *slope,
		double *data,
		int nDataPoints,
		int sampleDelta
	)
{
	return calculateSlopeBufferGeneric(
			(void *) slope,
			(void *) data,
			nDataPoints,
			sizeof(double),
			fromDoublePointer,
			toDoublePointer,
			sampleDelta, NULL);
}

