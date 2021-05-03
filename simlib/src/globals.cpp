/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: globals.cpp 28 2020-09-25 20:00:58Z andrew $
 **/
#include <os_defs.h>

#ifndef MAKEDEPEND
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

#  ifdef                OS_WINDOWS_NT
#   include <io.h>
#  endif

# include <ctype.h>
# include <sys/stat.h>
#endif

#include "MUP.h"
#include "SimulatorControl.h"
#include "dco.h"

#include "globalHandler.h"
#include "pathtools.h"
#include "random.h"
#include "userinput.h"
#include "error.h"
#include "tclCkalloc.h"
#include "log.h"

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

struct globals *g = NULL;


/* assign hard-coded defaults */
void clearGlobalPointers()
{
	/* muscle_dir - muscle directory */
	g->muscle_dir = NULL;

	/* firings_dir - firing times directory */
	g->firings_dir = NULL;

	/* MUPs_dir - MUPs  directory */
	g->MUPs_dir = NULL;

	/* output_dir - final output directory */
	g->output_dir = NULL;
}


/* assign hard-coded defaults */
int setGlobalDefaultValues(struct globals *globalValues)
{
	globalValues->list_ = NULL;

		/* version string */
	strcpy(globalValues->version_string, __PROG_VERSION__);

#ifdef  OS_WINDOWS_NT
		/* output_stem - parent of all output dirs */
	strncpy(globalValues->output_stem, "c:\\simulator\\data",
			FILENAME_MAX);
#else
		/* output_stem - parent of all output dirs */
	strncpy(globalValues->output_stem, "data",
			FILENAME_MAX);
#endif

		/* muscle_dir_sub - muscle directory */
	strncpy(globalValues->muscle_dir_sub, "XXX",
			FILENAME_MAX);

		/* firings_dir - firing times directory */
	strncpy(globalValues->firings_dir_sub, "Firing-Data",
			FILENAME_MAX);

		/* MUPs_dir - MUPs  directory */
	strncpy(globalValues->MUPs_dir_sub, "MFP-Data",
			FILENAME_MAX);

		/* output_dir - final output directory */
	strncpy(globalValues->output_dir_sub, "emg",
			FILENAME_MAX);

		/* patient_name - patient directory name */
	strncpy(globalValues->patient_name, "patient",
			FILENAME_MAX);

	clearGlobalPointers();

		/* first_samp - first sample location to be loaded */
	globalValues->first_samp = 0;

	globalValues->text_output = 0;


		/* smpling_freq - sampling frequency for MUP (kHz) */
	globalValues->smpling_freq = 31.250;

	globalValues->muscle_->setMuscleParamDefaultValues(200);

	globalValues->muscleLayoutFunctionType = LAYOUT_WEIGHTING;

		/* slope - ipi firing slope */
	globalValues->firing_.recruitmentSlope = 0.8f;

		/* cntrctn_lev - contraction level */
	globalValues->firing_.contractionLevelAsPercentMVC = 5.0f;

		/* minfiring - min firing rate */
	globalValues->firing_.minimumFiringRate = 8;

		/* maxfiring - max firing rate */
	globalValues->firing_.maximumFiringRate = 42;

		/* cofv - coeff of variance */
	globalValues->firing_.coefficientOfVarianceInFiringTimes = 0.25f;

		/* max_thrshld - maximum recruitment thrshld */
	globalValues->firing_.maximumFiringThreshold = 50.0f;

		/* electrode_type - between 1 and 4 */
	globalValues->electrode_type = 2;

		/* uptake - tip uptake distance */
	globalValues->tipUptakeDistance = 4500;
	globalValues->canUptakeDistance = 4500;

	globalValues->canPhysicalRadius = 250;

	globalValues->needleReferenceSetup = MUP::TIP_VERSUS_CANNULA;

		/* MUPs_per_mu - MUPs per mu */
	globalValues->MUPs_per_mu = 1;

		/* minimum metric to seek needle to */
	globalValues->minimumMuscleMetricThreshold = 0.25f;
	globalValues->seekNeedle = 1;

		/* jitter - variance of jitter value */
	globalValues->jitter = 25;

	globalValues->doJitter = 1;
	globalValues->super_jitter_seeds = 0;
	globalValues->jitterAccelThreshold =
		    (float) (MUP::sGetJitterAccelerationThreshold());

	globalValues->significantFibreAccelerationThreshold =
			globalValues->jitterAccelThreshold;

	globalValues->use_noise = 1;
	globalValues->signalToNoiseRatio = 25.0;

		/* stddev_x - standard deviation in 50 um */
	globalValues->stddev_x = 20.0;

		/* stddev_y - standard deviation in 50 um */
	globalValues->stddev_y = 20.0;

		/* stddev_z - standard deviation in 50 um */
	globalValues->stddev_z = 20.0;

		/* needle_TC - (ms) needle move time constant */
	globalValues->needle_TC = 400;


		/* emg_elapsed_time - time index to stop EMG generation */
	globalValues->emg_elapsed_time = 30;

		/*
		 * max value for conversion to 16 bit
		 *
		 * While the theoretical max here is MAX_SHRT,
		 * DQEMG seems to have problems when the data
		 * is not in the range += 4096
		 */
	globalValues->maxShortVoltage = 4096;

	globalValues->jitterInterpolationExpansion
		        = MUP::sGetExpansionFactor();


	/*
	 * set up defaults for pathology modelling
	 */
	globalValues->pathology.neuropathicMULossFraction = 0.0f;
	globalValues->pathology.neuropathicMaxAdoptionDistanceInUM =
					(int) ((3.0 / CELLS_PER_MM) * 1000.0);
	globalValues->pathology.neuropathicEnlargementFraction = 1.5;

	globalValues->pathology.myopathicCycleNewInvolvementPercentage = 5;
	globalValues->pathology.myopathicFractionOfFibresAffected = 0.0f;
	globalValues->pathology.myopathicFibreDiameterMean = 40.0;
	globalValues->pathology.myopathicFibreGraduallyDying = 0;
	globalValues->pathology.myopathicDependentProcedure = 1;
	globalValues->pathology.myopathicFibreDeathDiameter = 25.0f;
	globalValues->pathology.myopathicPercentageOfAffectedFibersDying = 0.0f;
	globalValues->pathology.myopathicHypertrophicFibreFraction = 0.05f;
	globalValues->pathology.myopathicHypertrophySplitThreshold = 2.0;
	globalValues->pathology.myopathicPercentageOfHypertrophicFibersSplit = 0.0f;
	globalValues->pathology.myopathicAtrophyRatePerCycle
					= (float) MYOPATHY_ATROPHY_FRACTION_PER_CYCLE;
	globalValues->pathology.myopathicHypertrophyRatePerCycle
					= (float) MYOPATHY_HYPERTROPHY_FRACTION_PER_CYCLE;

	strncpy(globalValues->fileDescription.operator_name,
		        "Simulated Operator",
				FILENAME_MAX);
	strncpy(globalValues->fileDescription.patient_name,
		        "Simulated Patient",
				FILENAME_MAX);
	strncpy(globalValues->fileDescription.muscle_description,
		        "Biceps Brachii",
				FILENAME_MAX);
	globalValues->fileDescription.patient_id = 0;
	globalValues->fileDescription.muscle_side =  DQEmgData::RIGHT;
	globalValues->fileDescription.new_muscle = 0;
	globalValues->fileDescription.new_operator = 0;
	globalValues->fileDescription.new_patient = 0;

	globalValues->use_last_muscle = 0;
	globalValues->use_old_firing_times = 1;
	globalValues->filter_raw_signal = 0;
	globalValues->generateMFPsWithoutInitiation = 0;
	globalValues->recordMFPPeakToPeak = 0;
	globalValues->mu_layout_type = GRID_MU_LAYOUT;

	globalValues->needle_z_position = (float) 15.0;
	globalValues->needle_x_position = (float) 0.0;
	globalValues->needle_y_position = (float) 0.0;
	globalValues->cannula_length = (float) 10.0;

	globalValues->generate_second_channel = 1;

	return 1;
}

/* clean up the globals structure */
void deleteGlobals()
{
	if (g != NULL)
	{
		if (g->muscle_dir != NULL)		ckfree(g->muscle_dir);
		if (g->firings_dir != NULL)		ckfree(g->firings_dir);
		if (g->MUPs_dir != NULL)		ckfree(g->MUPs_dir);
		if (g->output_dir != NULL)		ckfree(g->output_dir);

		if (g->muscle_ != NULL)				delete g->muscle_;

		if (g->list_ != NULL)
		    deleteAttValList(g->list_);
		ckfree(g);
		g = NULL;
	}
}

