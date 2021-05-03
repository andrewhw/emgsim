#include "os_defs.h"

#include "tclCkalloc.h"
#include "mathtools.h"
#include "filtertools.h"

#include "massert.h"
#include "log.h"

/*
 * ----------------------------------------------------------------
 * Return the Peak-to-Peak difference of the float buffer
 * (simply max - min)
 */
OS_EXPORT float 
calcPeakToPeakDifferenceFloat(float *buffer, int length)
{
	int             i;
	float           min, max;

	if (length < 1)
	{
		return 0;
	}
	min = max = buffer[0];
	for (i = 1; i < length; i++)
	{

		if (min > buffer[i])
			min = buffer[i];

		if (max < buffer[i])
			max = buffer[i];
	}

	return (max - min);
}


/*
 * ----------------------------------------------------------------
 * Return the Peak-to-Peak difference of the float buffer
 * (simply max - min)
 */
OS_EXPORT double 
calcPeakToPeakDifferenceDouble(double *buffer, int length)
{
	int             i;
	double          min, max;

	if (length < 1)
	{
		return 0;
	}
	min = max = buffer[0];
	for (i = 1; i < length; i++)
	{

		if (min > buffer[i])
			min = buffer[i];

		if (max < buffer[i])
			max = buffer[i];
	}

	return (max - min);
}

