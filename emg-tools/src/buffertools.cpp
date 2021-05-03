/*
 * ----------------------------------------------------------------
 * Buffer calculation tools
 *
 * Filtering and turn-counting are here
 */

#include <string.h>

#include "buffertools.h"
#include "tclCkalloc.h"
#include "listalloc.h"
#include "massert.h"

#include "os_defs.h"


OS_EXPORT int
calculateAcceleration(float *target, const emgValue * src, int nValues)
{
	return bandpassFilter(target, src, nValues,
						  2,		/* acceleration */
						  1,		/* bandpass */
						  1.0);		/* 1/samplingrate */
}

OS_EXPORT int
bandpassFilter(float *target, const emgValue * src,
			   int nValues, int filterOrder, int bandpass,
			   float deltaT)
{
	int i;
	float slope1, slope2;

	if (filterOrder == 1)
	{
		for (i = 0; i < bandpass; i++)
			target[i] = 0.0;

		for (i = bandpass; i < nValues - bandpass; i++)
			target[i] = (float) (src[i + bandpass] - src[i - bandpass]);

		for (i = nValues - bandpass; i < nValues; i++)
			target[i] = 0.0;


	} else if (filterOrder == 2)
	{
		for (i = 0; i < bandpass; i++)
			target[i] = 0.0;

		for (i = bandpass; i < nValues - 2 * bandpass; i++)
		{

			slope2 = (src[i + 2 * bandpass] - src[i])
				/ (float) (2.0 * bandpass * deltaT);

			slope1 = (src[i + bandpass] - src[i - bandpass])
				/ (float) (2.0 * bandpass * deltaT);

			target[i] = (float) (slope2 - slope1)
				/ (bandpass * deltaT);
		}

		for (i = nValues - 2 * bandpass; i < nValues; i++)
			target[i] = 0;

	} else
	{
		return 0;
	}

	/*
	 * if bandpass = 1, apply 3-point triangle filter.
	 * (in case of slope or accel)
	 */
	if (bandpass == 1)
	{
		float *tmpBuffer;

		tmpBuffer = (float *) ckalloc(sizeof(float) * nValues);
		memcpy(tmpBuffer, target, nValues);

		for (i = 1; i < nValues - 1; i++)
		{
			target[i] = (float) ((tmpBuffer[i - 1] * 0.25)
								 + (tmpBuffer[i] * 0.5)
								 + (tmpBuffer[i + 1] * 0.25));
		}
		ckfree(tmpBuffer);
	}
	return 1;
}

static void
addTurnToBuffer(
		emgTurn ** turnBuffer,
		int *turnBufferBlocks,
		int turnIndex,
		int turnDuration,
		double turnAmplitude,
		float samplingRateInHz
	)
{
	int status;

	status = listMkCheckSize(
							   turnIndex + 1,
							   (void **) turnBuffer,
							   turnBufferBlocks,
							   sizeof(emgTurn),
							   128, __FILE__, __LINE__);
	MSG_ASSERT(status, "malloc failed");

	(*turnBuffer)[turnIndex].amplitudeInUV = turnAmplitude;
	(*turnBuffer)[turnIndex].durationInSamples = turnDuration;
	(*turnBuffer)[turnIndex].durationInMS =
		1000.0 * (turnDuration / samplingRateInHz);
}

/**
 * Count turns based on a buffer from an algorithm from
 * Fitch and Willison as found in a paper by Boyd, Bratty and Lawrence
 *
 * Turns which exceed a threshold (specified in scaled units)
 * are counted.  Turns are recorded if the passed buffer
 * (turnBuffer) is not NULL.
 */
OS_EXPORT int
countAndStoreTurns(
		emgTurn ** turnBuffer,
		emgValue * buffer,
		osUint32 bufferLen,
		float scale,
		float turnAmplitudeThreshold,
		float samplingRateInHz
	)
{
	int turnCount = (-1);
	int turnBufferBlocks = 0;
	int positiveRiseFlag = 0;
	int negativeFallFlag = 0;
	int positiveRisePos = 0;
	int negativeFallPos = 0;
	double posReferenceValue;
	double negReferenceValue;
	unsigned long lastTurnIndex = 0;
	double currentValue, lastTurnVoltageValue = 0;
	unsigned long i;


	posReferenceValue = buffer[0] * scale;
	negReferenceValue = buffer[0] * scale;

	for (i = 0; i < bufferLen; i++)
	{
		currentValue = (buffer[i] * scale);
		if ((currentValue - negReferenceValue) > turnAmplitudeThreshold)
		{
			positiveRiseFlag = 1;
			negReferenceValue = currentValue;
			positiveRisePos = i;
		}
		if ((posReferenceValue - currentValue) > turnAmplitudeThreshold)
		{
			negativeFallFlag = 1;
			posReferenceValue = currentValue;
			negativeFallPos = i;
		}
		if (positiveRiseFlag && negativeFallFlag)
		{
			if (negativeFallPos < positiveRisePos)
			{
				/** Negative turn has occurred */
				turnCount++;
				negativeFallFlag = 0;
				if (turnBuffer != NULL)
				{

					/**
					 * calculate difference between this point and
					 * the last one logged, store the amplitude
					 * and distance
					 */
					addTurnToBuffer(
							   turnBuffer, &turnBufferBlocks, turnCount + 1,
									i - lastTurnIndex,
									currentValue - lastTurnVoltageValue,
									samplingRateInHz
						);
					lastTurnIndex = i;
					lastTurnVoltageValue = currentValue;
				}
			}
			if (negativeFallPos > positiveRisePos)
			{
				/** Positive turn has occurred */
				turnCount++;
				if (turnBuffer != NULL)
				{

					/**
					 * calculate difference between this point and
					 * the last one logged, store the amplitude
					 * and distance
					 */
					addTurnToBuffer(
							   turnBuffer, &turnBufferBlocks, turnCount + 1,
									i - lastTurnIndex,
									currentValue - lastTurnVoltageValue,
									samplingRateInHz
						);
					lastTurnIndex = i;
					lastTurnVoltageValue = currentValue;
				}
				positiveRiseFlag = 0;
			}
		}
		if (currentValue > posReferenceValue)
		{
			posReferenceValue = currentValue;
			positiveRisePos = i;
		}
		if (currentValue < negReferenceValue)
		{
			negReferenceValue = currentValue;
			negativeFallPos = i;
		}
	}

	return turnCount;
}


/**
 * Count turns based on a buffer from an algorithm from
 * Fitch and Willison as found in a paper by Boyd, Bratty and Lawrence
 */
OS_EXPORT int
countTurns(
		emgValue * buffer,
		osUint32 bufferLen,
		float scale,
		float turnAmplitudeThreshold
	)
{
	return countAndStoreTurns(NULL,
							  buffer, bufferLen, scale,
							  turnAmplitudeThreshold,
							  0);
}

