/**
 ** Convert the output from the emg.dat file into the 16 bit
 ** format required by the DQEMG program.
 **
 ** $Id: statistics.cpp 13 2011-03-23 11:22:44Z andrew $
 **/

#include "os_defs.h"

#ifndef    MAKEDEPEND
# include    <stdio.h>
# include    <string.h>
# include    <errno.h>
#endif

#include "stringtools.h"
#include "buffertools.h"
#include "pathtools.h"

#include "statistics.h"
#include "SimulatorControl.h"
#include "Simulator.h"
#include "DQEmgData.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


int calculateStatistics(
		SimulationControl *controlValues,
		SimulationResult *simData,
		DQEmgData *dqemgContraction
	)
{
	emgValue *bufferData;
	unsigned long bufferLen;
	float samplingRate;
	float scale;
	int nTurns;

	const char *filename = "turncounts.txt";
	FILE *turnFP;

	bufferData = dqemgContraction->getChannel(0)->getDataAsShort(
			&bufferLen,
			&samplingRate,
			&scale
		);

	if (bufferData == NULL)
		return 0;

	nTurns = countTurns(bufferData, bufferLen, scale, (float) 50);
	turnFP = fopenpath(filename, "a");
	if (turnFP == NULL)
	{
		LogError("Cannot open turn count file : %s\n", strerror(errno));
		return 0;
	}
	fprintf(turnFP, "%f %f\n",
			controlValues->firing_.contractionLevelAsPercentMVC,
			(float) (nTurns / (double) controlValues->emg_elapsed_time));

	fclose(turnFP);

	return 1;
}

