/**
 ** Module:             FIRING.C  source module
 **
 ** Author:             Stephen Scott
 **
 ** Date:               1988
 **
 **   Description:       creates firing times for active motor units
 **
 ** Modifications :
 **  converted from pascal by Michael Janzen 1989
 **
 ** This program creates the firing patterns for a motor unit.
 ** Input to the model specify the list of motor units in the detection
 ** area or the operator can choose to select certain motor units to
 ** create the firing patterns.
 **
 ** For a given neural drive (0-100%), the motor unit firing patterns
 ** are created for all motor units firing at that level.  The firing
 ** times are then saved to disk for use in the program "WAVEFORM.C"
 **
 ** This program was written in Pascal by Stephen Scott, copyright 1988
 ** It was converted to C by Michael Janzen, 1989, on a cold
 ** winter's night.
 **
 **
 ** $Id: firing.cpp 28 2020-09-25 20:00:58Z andrew $
 **/


#include "os_defs.h"

#ifndef MAKEDEPEND
# include <stdio.h>
# include <math.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
#endif

#include "tclCkalloc.h"
#include "reporttimer.h"
#include "random.h"
#include "pathtools.h"
#include "stringtools.h"
#include "listalloc.h"
#include "error.h"
#include "massert.h"
#include "log.h"

#define PRIVATE public
#include "MuscleData.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


/**
 * Calculate firings times for a single motor unit
 *
 * The equation for the firing times
 * (currentMU->mu_firingTime_[X])
 * is based on Andy Fuglevand's work
 */
static int
calculateSingleMUFiringTimes(
		MotorUnit *currentMU,
		int *nTimesFiringTooShort,
		float *meanIPI,
		float meanFiringRate,
		int totalElapsedTimeInSeconds,
		float coefficientOfVarianceInFiringTimes
	)
{
	double newDoubleFiringTime;
	long newLongFiringTime;
	double gaussianVariable;
	float sumIPI;
	int maxGenerationTimeInSeconds;


	sumIPI = 0;
	maxGenerationTimeInSeconds = totalElapsedTimeInSeconds + 1;



	/**
	 * Create the sequence of firing times
	 * (currentMU->mu_firingTime_[i]) for ith firing.
	 *
	 * The firing times are in units of 0.1 msec.
	 */
	currentMU->mu_nFirings_ = 0;

	/** add the first firing time separately */
	listMkCheckSize(
			1,
			(void **) &currentMU->mu_firingTime_,
			&currentMU->mu_nFiringBlocks_,
			16,
			sizeof(long), __FILE__, __LINE__);
	currentMU->mu_firingTime_[0] = (long)
			(ceil(10000. * (floatNormalizedRandom())
					* (1 / meanFiringRate)));
	currentMU->mu_nFirings_ = 1;


	/** now add the rest of the firings */
	while (currentMU->mu_firingTime_[ currentMU->mu_nFirings_ - 1]
			< (maxGenerationTimeInSeconds * 10000L))
	{

		gaussianVariable = fabs(gauss01());
		newDoubleFiringTime =
				ceil(10000.0 * (1.0 / meanFiringRate
				+ (1.0 / meanFiringRate)
						* coefficientOfVarianceInFiringTimes
						* gaussianVariable));

		/** check for overflow */
		if (newDoubleFiringTime +
				(double) currentMU->mu_firingTime_[
								currentMU->mu_nFirings_ - 1
						] >= LONG_MAX)
		{
			LogCrit("Overflow in Firing time for MU %d\n",
						currentMU->mu_id_);
			return 0;
		}

		newLongFiringTime = (long) newDoubleFiringTime;

		/* error check */
		if (newLongFiringTime < 100)
		{
			Error("IPI %d only %ld -- minimum time 10 ms\n",
					currentMU->mu_nFirings_,
					newLongFiringTime);
			Error("   Gaussian value %f\n", gaussianVariable);
			(*nTimesFiringTooShort)++;
			newDoubleFiringTime = 100;
		}

		/**
		 * First, log the new mean IPI
		 */
		sumIPI = sumIPI + newLongFiringTime;

		/**
		 * then add offset of last firing time
		 * to convert to absolute firing time
		 */
		newLongFiringTime +=
				currentMU->mu_firingTime_[
						currentMU->mu_nFirings_ - 1
					];

		/**
		 * grow the firing list (if necessary)
		 * and store the new time in the list.
		 */
		listMkCheckSize(
					currentMU->mu_nFirings_ + 1,
					(void **)
						&currentMU->mu_firingTime_,
					&currentMU->mu_nFiringBlocks_,
					16,
					sizeof(long), __FILE__, __LINE__);

		currentMU->mu_firingTime_[
						currentMU->mu_nFirings_
					] = newLongFiringTime;
		currentMU->mu_nFirings_++;
	}


	/** store the mean IPI for this MU */
	if (currentMU->mu_nFirings_ == 0)
		*meanIPI = 0;
	else
		*meanIPI = sumIPI / (float) currentMU->mu_nFirings_;

	/** return success */
	return 1;
}


/**
 * Calculate firing times for all MUs above their firing threshold.
 *
 *
 * If activation level is above motor unit i's activation
 * threshold, set firing rate for motor unit i, repeat
 * for all i
 */
static int
calculateFiringTimes(
		const char *firingsDirectory,
		MuscleData *MD,
		float *firingThresholdsByMUInDetect,
		int *nTimesFiringTooShort,
		float *pps,
		int totalElapsedTimeInSeconds,
		float coefficientOfVarianceInFiringTimes,
		float contractionLevelAsPercentMVC,
		float recruitmentSlope,
		float minimumFiringRate,
		float maximumFiringRate
	)
{
	char ftmuFilename[FILENAME_MAX];
	FILE *ftmuFP;
	MotorUnit *currentMU;
	float meanFiringRate;
	float meanIPI;
	long difference;
	long totalFirings = 0;
	float squaredMean;
	//float variance;
	int i, j;

	/** if there are previous active MU's we delete them first */
	if (MD->activeMotorUnit_ != NULL)
	{
		ckfree(MD->activeMotorUnit_);
		MD->activeMotorUnit_ = NULL;
		MD->nActiveMotorUnits_ = 0;
		MD->nActiveMotorUnitBlocks_ = 0;
	}


	*nTimesFiringTooShort = 0;
	*pps = 0;


//	LogInfo("\n");
//	LogInfo("Firing rate by active MU in detection area:\n");
//	LogInfo("\n");
//
//	LogInfo(
//		" -----+------------+------------+----------+----------+\n");
//	LogInfo(
//		"      | Mean Firing|            |          | Coeff.   |\n");
//	LogInfo(
//		" MU # |     Rate   |  Mean IPI  | Std. Dev |    of    |\n");
//	LogInfo(
//		"      |  (imp/sec) |   (msec)   |          | Variance |\n");
//	LogInfo(
//		" -----+------------+------------+----------+----------+\n");

	for (i = 0; i < MD->nMotorUnitsInDetectionArea_; i++)
	{

		currentMU = MD->motorUnitInDetect_[i];

		/**
		 * If activation level is above motor unit i's activation
		 * threshold, set firing rate for motor unit i, repeat
		 * for all i
		 */
		if (contractionLevelAsPercentMVC >= firingThresholdsByMUInDetect[i])
		{


			listMkCheckSize(
					MD->nActiveMotorUnits_ + 1,
					(void **) &MD->activeMotorUnit_,
					&MD->nActiveMotorUnitBlocks_,
					16,
					sizeof(MotorUnit *), __FILE__, __LINE__);
			MD->activeMotorUnit_[
						MD->nActiveMotorUnits_++
					] = currentMU;

			meanFiringRate = recruitmentSlope
					* (contractionLevelAsPercentMVC -
							firingThresholdsByMUInDetect[i])
					+ minimumFiringRate;


			/*
			 * keep firing rate below maximum possible firing rate
			 */
			if (meanFiringRate > maximumFiringRate)
				meanFiringRate = (float) maximumFiringRate;


			/**
			 * get a set of firing times for this MU
			 */
			if ( ! calculateSingleMUFiringTimes(
						currentMU,
						nTimesFiringTooShort,
						&meanIPI,
						meanFiringRate,
						totalElapsedTimeInSeconds,
						coefficientOfVarianceInFiringTimes
					) )
			{
				return 0;
			}


			/*
			 * Determine if the firing rates are
			 * statistically valid
			 */
			squaredMean = 0.0;   /* sum of squares */
			for (j = 2; j < currentMU->mu_nFirings_; j++)
			{
				difference =
					(currentMU->mu_firingTime_[j]
					- currentMU->mu_firingTime_[j-1]);
				squaredMean += (difference * difference);
			}
			//variance =
			//	(squaredMean -
			//	(float) currentMU->mu_nFirings_
			//				* meanIPI * meanIPI) /
			//	(float) (currentMU->mu_nFirings_ - 1);


			/**
			 * let the user see what we did
			 */
//			LogInfo(" %4d | %10.2f | %10.2f | %8.2f | %8.2f |\n",
//							currentMU->mu_id_,
//							meanFiringRate,
//							meanIPI / 10.0,
//							sqrt(variance) / 10.0,
//							sqrt(variance) / meanIPI);

			/** accumulate the total number of pulses */
			totalFirings += currentMU->mu_nFirings_;

			/*
			 * store motor unit i firing times to disk name
			 * file "mu__.dat" where the blank is the motor
			 * unit number
			 *
			 * This file stores the number of firings at the
			 * top of the file followed by the list of firing
			 * times
			 */
			slnprintf(ftmuFilename, FILENAME_MAX,
					"%s\\FTMU%d.dat", firingsDirectory,
					MD->activeMotorUnit_[i]->mu_id_);
			ftmuFP = fopenpath(ftmuFilename, "wb");
			if (ftmuFP == NULL)
			{
				Error("Unable to open %s", ftmuFilename);
				ckfree(firingThresholdsByMUInDetect);
				return 0;
			}
			fprintf(ftmuFP, "%d\n", currentMU->mu_nFirings_);
			for (j = 0; j < currentMU->mu_nFirings_; j++)
			{
				fprintf(ftmuFP, "    %ld\n",
							currentMU->mu_firingTime_[j]);
			}
			fclose(ftmuFP);
		}
	}

//	LogInfo(" -----+------------+------------+----------+----------+\n");

	*pps = (float) totalFirings / (float) totalElapsedTimeInSeconds;

	return 1;
}

/**
 * Calculate firing thresholds for all the MUs in the detection
 * area.
 *
 *
 * This routine creates the power function to set the motor
 * unit recruitment threshold given the number of motor units,
 * and the smallest and largest recruitment threshold values.
 *
 * From Andy Fuglevands work.
 *
 *
 * The slope of activation follows the graph from the book
 * Muscles Alive
 */
static float *
calculateFiringThresholds(
		MuscleData *MD,
		int forceFiring,
		float maximumFiringThreshold
	)
{
	MotorUnit *currentMU, *tmpMU;
	float *firingThresholdsByMUInDetect;
	float checkThreshold;
	double ratio1, ratio2;
	int healthyMaxNumFibres, sumFibres;
	int lastIndex;
	int i, j;
#ifdef DEBUG
	FILE *dfp;
#endif

#ifdef DEBUG
	dfp = fopen("FiringThreshold.txt", "w");
#endif

	/**
	 * get a list into which we will put the thresholds
	 * (in units of MVC)
	 */
	firingThresholdsByMUInDetect = (float *)
			   ckalloc((MD->nMotorUnitsInDetectionArea_)
				* (sizeof(float)));
	MSG_ASSERT(firingThresholdsByMUInDetect != NULL,
					"Allocation Failure");


	/**
	 * thresholds are calculated by fraction of total
	 * fibres cumulatively allocated to a particular
	 * motor unit (and all those already recruited
	 * below it) as a fraction of the point of total
	 * recruitment (the maximumFiringThreshold)
	 */
	healthyMaxNumFibres = MD->getOriginalNumberOfFibres();

	/** ratios for old implementation */
	ratio2 = log((double) maximumFiringThreshold)
				/ (MD->nMotorUnitsInMuscle_ - 1);
	ratio1 = 1.0 / exp(ratio2);

	/** clear the accumulators for new implementation */
	sumFibres = 0;
	lastIndex = (-1);
	for (i = 0; i < MD->nMotorUnitsInDetectionArea_; i++)
	{
		/** get the next MU from the master list */
		currentMU = MD->getMotorUnitInDetectionArea(i);
		if (currentMU == NULL)
			continue;

		/**
		 * add up all the fibres allocated to MU's between
		 * the in-detect MU's
		 */
		for (j = lastIndex + 1; j < currentMU->getID() - 1; j++)
		{
			tmpMU = MD->getMotorUnitFromMasterList(j);
			if (tmpMU != NULL)
			{
				LogInfo("       Skipped MU %d outside detect\n", j);
				sumFibres += tmpMU->getNumFibres();
				LogInfo("         - Skipped MU contained %d fibres\n",
								tmpMU->getNumFibres());
			}
		}

		lastIndex = currentMU->getID() - 1;

		/**
		 * Add in our own fibres, the resulting fraction of
		 * the total fibres indicates where we come in between
		 * 0 % MVC and maximumFiringThreshold (which is in %MVC)
		 */
		sumFibres += currentMU->getNumFibres();
//		LogInfo("   SumFibres %d  (total %d)\n", sumFibres, healthyMaxNumFibres);
		firingThresholdsByMUInDetect[i] =
							maximumFiringThreshold *
								(sumFibres / (float) healthyMaxNumFibres);


		checkThreshold = (float) (ratio1 * exp(ratio2 *
								(MD->motorUnitInDetect_[i]->mu_id_)));
//		LogInfo(
//    "   Firing Threshold %d (%d) : new %f, old %f  diff %f  fraction %f\n",
//				currentMU->getID(), i,
//				firingThresholdsByMUInDetect[i], checkThreshold,
//				firingThresholdsByMUInDetect[i] - checkThreshold,
//				firingThresholdsByMUInDetect[i] / checkThreshold);
#ifdef DEBUG
		if (dfp != NULL)
		{
			fprintf(dfp, "%f %f\n",
				firingThresholdsByMUInDetect[i], checkThreshold);
		}
#endif
	}
#ifdef DEBUG
		fclose(dfp);
#endif

	return firingThresholdsByMUInDetect;
}


/**
 ** ----------------------------------------------------------------
 ** Function:     FIRING
 **
 ** Description:
 ** Creates firing times for the simulated muscle.
 **     Needs to know (globally) the contraction level
 **     slope, variance etc.
 **
 **/
OS_EXPORT int
firing(
		const char *firingsDirectory,
		MuscleData *MD,
		float contractionLevelAsPercentMVC,
		float coefficientOfVarianceInFiringTimes,
		float recruitmentSlope,
		float minimumFiringRate,
		float maximumFiringRate,
		float maximumFiringThreshold,
		int totalElapsedTimeInSeconds,
		int forceFiring
	)
{
	char amuFilename[FILENAME_MAX];
	FILE *amuFP;
	float *firingThresholdsByMUInDetect;
	int nTimesFiringTooShort;
	float pps;
	int i;



	LogInfo("  -- Firing Time Calculations --\n");
	LogInfo("\n");
	LogInfo("# of MUs in muscle         : %d\n",
				MD->nMotorUnitsInMuscle_);
	LogInfo("# of MUs in detection area : %d\n",
				MD->nMotorUnitsInDetectionArea_);


	/**
	 * Get the firing thresholds
	 */
	firingThresholdsByMUInDetect =
				calculateFiringThresholds(
						MD,
						forceFiring,
						maximumFiringThreshold
					);


	/**
	 * Log controls which govern firing
	 */
	LogInfo("\n");
	LogInfo("Minimal Firing Rate:                %s\n",
			niceDouble(minimumFiringRate));
	LogInfo("Maximal Firing Rate:                %s\n",
			niceDouble(maximumFiringRate));
	LogInfo("Firing Rate Slope:                  %.3f\n",
			recruitmentSlope);
	LogInfo("Coefficient of Variation:           %.3f\n",
			coefficientOfVarianceInFiringTimes);
	LogInfo("\n");
	LogInfo("Contraction level (%%MVC)            %.2f\n",
			contractionLevelAsPercentMVC);
	LogInfo("Max MU recruitment threshold (%%MVC) %.2f\n",
			maximumFiringThreshold);
	LogInfo("\n\n");



	if ( ! calculateFiringTimes(
				firingsDirectory,
				MD,
				firingThresholdsByMUInDetect,
				&nTimesFiringTooShort,
				&pps,
				totalElapsedTimeInSeconds,
				coefficientOfVarianceInFiringTimes,
				contractionLevelAsPercentMVC,
				recruitmentSlope,
				minimumFiringRate,
				maximumFiringRate
			) )
	{
		LogCrit("Firing time calculation failed -- aborting\n");
		return 0;
	}


	/*
	 * Save the list of active motor units
	 */
	slnprintf(amuFilename, FILENAME_MAX,
			"%s\\AMU.dat", firingsDirectory);
	LogInfo("\n");
	LogInfo("  Timing failure occurred %d times\n",
			nTimesFiringTooShort);

	LogInfo("\n");
	LogInfo("  Firing rate for muscle %g pps\n", pps);


	amuFP = fopenpath(amuFilename, "wb");
	if (amuFP == NULL)
	{
		Error("Unable to open %s", amuFilename);
		ckfree(firingThresholdsByMUInDetect);
		return 0;
	}

	fprintf(amuFP, "%d\n", MD->nActiveMotorUnits_);
	for (i = 0; i < MD->nActiveMotorUnits_; i++)
	{
		fprintf(amuFP, "  %d\n", MD->activeMotorUnit_[i]->mu_id_);
	}
	fclose(amuFP);

	LogInfo("%d active MUs stored in file: %s\n",
				MD->nActiveMotorUnits_,
				amuFilename);


	ckfree(firingThresholdsByMUInDetect);

	return 1;
}


static int
loadFiringTimeData(const char *filename, MotorUnit *currentMU)
{
	char inputLine[4096];
	FILE *fp;
	int readHeader = 0;
	int firingTimeIndex = 0;


	/*
	 * Load the list of active motor units
	 */
	fp = fopenpath(filename, "rb");
	if (fp == NULL)
	{
		Error("Unable to open %s", filename);
		return 0;
	}


	while (fgets(inputLine, 4096, fp) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		if ( ! readHeader )
		{
			sscanf(inputLine, "%d", &currentMU->mu_nFirings_);

			/**
			 * allocate the list for Active MU's.  The
			 * actual MU's must already have been loaded
			 * by a call to loadMUdata
			 */
			currentMU->mu_firingTime_ = (long *)
						ckalloc(sizeof(long)
								* currentMU->mu_nFirings_);
			readHeader = 1;
		} else
		{

			if (firingTimeIndex < currentMU->mu_nFirings_)
			{

				/**
				 * record the firing time in the list
				 */
				sscanf(inputLine, "%ld",
						&currentMU->mu_firingTime_[firingTimeIndex]);

				firingTimeIndex++;
			} else
			{
				LogError("Too many lines in '%s'\n", filename);
				return 0;
			}
		}
	}

	if (firingTimeIndex < currentMU->mu_nFirings_)
	{
		LogError("Too few lines in '%s'\n", filename);
		return 0;
	}

	return 1;
}


/**
 ** ----------------------------------------------------------------
 ** Function:     FIRING
 **
 ** Description:
 ** Creates firing times for the simulated muscle.
 **     Needs to know (globally) the contraction level
 **     slope, variance etc.
 **
 **/
OS_EXPORT  int
loadFiring(MuscleData *MD, const char *firingsDirectory)
{
	char filename[FILENAME_MAX];
	MotorUnit *currentMU;
	int i;


	/**
	 * Load the firing times themselves
	 */
	for (i = 0; i < MD->nActiveMotorUnits_; i++)
	{

		currentMU = MD->activeMotorUnit_[i];
		if (currentMU == NULL)
			continue;

		slnprintf(filename, FILENAME_MAX, "%s\\FTMU%d.dat",
						firingsDirectory,
						currentMU->mu_id_);

		if ( ! loadFiringTimeData(filename, currentMU) )
		{
			return 0;
		}
	}

	return 1;
}



