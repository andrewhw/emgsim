/**
 ** ----------------------------------------------------------------
 ** Noise contribution code.
 **/

#ifndef    MAKEDEPEND
# include <stdio.h>
#endif

#include "os_types.h"

#include "noise.h"

#include "tclCkalloc.h"
#include "random.h"
#include "filtertools.h"
#include "stringtools.h"

#include "massert.h"
#include "log.h"


/*
#define         USE_LONG_LONG_FOR_RMS
*/
#define         USE_LONG_DOUBLE_FOR_RMS

# if defined ( USE_LONG_DOUBLE_FOR_RMS )
#    define    RMS_TYPE         long double

# elif defined ( USE_LONG_LONG_FOR_RMS )
#    define    RMS_TYPE         OsIndepUint64

# else
#   error No RMS type defined
# endif


/**
 ** ----------------------------------------------------------------
 ** Calculate the root mean square of the buffered signal
 **/
OS_EXPORT double
getBufferRMS(float *buffer, int bufferLength)
{
	int i;
	double bufferRms;
	RMS_TYPE bufferMeanSquare, lastBufferMeanSquare;


	/* calculate the buffer RMS value */
	lastBufferMeanSquare = bufferMeanSquare = 0;
	for (i = 0; i < bufferLength; i++)
	{
		bufferMeanSquare = bufferMeanSquare +
		                (RMS_TYPE) (buffer[i] * buffer[i]);
		MSG_ASSERT(lastBufferMeanSquare <= bufferMeanSquare,
		        "Rollover in RMS calculation");
		lastBufferMeanSquare = bufferMeanSquare;
	}
	bufferMeanSquare = bufferMeanSquare / bufferLength;
	bufferRms = sqrt((double) bufferMeanSquare);

	return bufferRms;
}



/**
 ** ----------------------------------------------------------------
 ** Calculate noise at a given S/N ratio, and add
 ** it in to the buffer
 **/
OS_EXPORT int
addNoiseToBuffer(
		float *buffer,
		int bufferLength,
		double samplingRate,
		double signalToNoiseRatio
	)
{
	double *raw_noise;
	double *filtered_noise;
	int i;
	double *a, *b;
	double maxNoise = 0;
	double bufferRms;
	int nA, nB, status;

	LogInfo("Adding noise to simulated signal with S/N ratio of %s\n",
		        niceDouble(signalToNoiseRatio));

	raw_noise = (double *) ckalloc(bufferLength * sizeof(double));
	filtered_noise = (double *)
		        ckalloc(bufferLength * sizeof(double));

	bufferRms = getBufferRMS(buffer, bufferLength);


	/* create the raw noise */
	for (i = 0; i < bufferLength; i++)
	{
		raw_noise[i] = gauss01() * (bufferRms / signalToNoiseRatio);
	}

	/* now get the appropriate filter values */
	status = getABParams(&b, &nB, &a, &nA,
		                FILTAB_O8_31250_10_10000HZ,
		                samplingRate);
	if (status <= 0)
		return 0;

	/* filter the data */
	filtfilt(filtered_noise, raw_noise,
		            bufferLength, b, nB, a, nA);

	/* put the noise in the buffer */
	for (i = 0; i < bufferLength; i++)
	{
		if (maxNoise < filtered_noise[i])
		    maxNoise = filtered_noise[i];
		buffer[i] = (float) (buffer[i] + filtered_noise[i]);
	}

	/* clean up */
	ckfree(raw_noise);
	ckfree(filtered_noise);

	return 1;
}


