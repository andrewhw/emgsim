/*
 * MakeMUP routines -- generate MFP's and combine them into
 * MUPS.
 *
 * $Id: makeMUP.cpp 28 2020-09-25 20:00:58Z andrew $
 */

#define PRIVATE public

#include "os_defs.h"

#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <math.h>
# include <stdlib.h>
# include <float.h>
#  ifdef                OS_WINDOWS_NT
#   include <io.h>
#  endif
# include <fcntl.h>

# include <time.h>
# include <ctype.h>
#endif

#include "SimulatorControl.h"
#include "SimulatorConstants.h"
#include "filtertools.h"
#include "MUP_utils.h"

#include "massert.h"
#include "random.h"
#include "reporttimer.h"
#include "error.h"
#include "pathtools.h"
#include "mathtools.h"
#include "stringtools.h"
#include "listalloc.h"
#include "tclCkalloc.h"
#include "log.h"

#include "MUP.h"
#include "NeedleInfo.h"
#include "Simulator.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


#define         INPUT_BUFFER_SIZE       4096

/** Output matches litreture with this scale factor */
#define scale_factor                            1.0

/** Determines whether or not the current correction has happened */
#define CurrCorr								1


	/*
	 * forward declarations
	 */

/* clean up all allocated buffers */
static void sCleanBuffers();

/* clean up all allocated Left buffers */
static void sCleanLeftBuffers();

/* get a buffer for the convolution result */
static double *sGetConvolutionBuffer(int MUPLength);

/* get a buffer for convolution of the left part of the fiber */
static double *sGetConvLeftBuffer(int MUPLength);

#ifdef  DEAD_CODE
/* get a buffer for bipolar needle */
static double *sGetConvRightBuffer(int MUPLength);
#endif

/* get a set of buffers for the FFT */
static void sGetFFTBuffers(int MUPLength,
		double **fftBuffer,
		double **weightBuffer,
		double **currentBuffer
	);

/* get a set of buffers for the left part of the fiber */
static void sGetFFTLeftBuffers(int MUPLength,
		double **fftLeftBuffer,
		double **weightLeftBuffer
	);

static int calculateMUP(
		MUP *newMUP,
		int MUPId,
		MUPControl *MUPControl,
		MuscleFibre **muscleFibre,
		int num_fibres,
		NeedleInfo *needle,
		int mu_number,
		int *inUptakeAreaFlag,
		float zPlacementStdDev,
		int tipUptakeDistanceInMicrons,
		int canUptakeDistanceInMicrons
	);

static int calculateSingleFibreMFP(
		int MUPLength,
		double *convolution,
		float z_dist_endpl,
		float radial_sep,
		float diam,
		float cond_vel
	);

static int calculateSingleFibreMFPWithInitiation(
		int MUPLength,
		double *convolution,
		float z_dist_endpl,
		float radial_sep,
		float diam,
		float cond_vel,
		float fiber_L,
		float EP_Location
	);

static int calculateBipolarMFAP(
		int MUPLength,
		double *convolution,
		float z_dist_endpl,
		float radial_sep,
		float radial_sep2,
		float diam,
		float cond_vel
	);

static int calculateBipolarMFAPWithInitiation(
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float radialSeparationInMM,
		float auxRadialSeparationInMM,
		float diameterInMM,
		float conductionVelocity_MMperMS,

		float FiberLengthInMM,
		float EndPlateLocationInMM
	);

static int calculateConcentricMFAP(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		int electrodeType,
		double *convolution,
		float z_dist_endpl,
		float deltay,
		float diam,
		float cond_vel,
		float muscleFibreXLocationInMM
	);

static int calculateConcentricMFAPWithInitiation(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		int electrodeType,
		double *convolution,
		float z_dist_endpl,
		float deltay,
		float diam,
		float cond_vel,
		float muscleFibreXLocationInMM,
		float FiberLengthInMM,
		float EndPlateLocationInMM
	);


static int calculateCannulaMFAP(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float fibreLocXInMM,
		float fibreLocYInMM,
		NeedleInfo *needle,
		float fibreDiameterInMM,
		float conductionVelocity_MMperMS
	);

static int calculateCannulaMFAPWithInitiation(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float fibreLocXInMM,
		float fibreLocYInMM,
		NeedleInfo *needle,
		float fibreDiameterInMM,
		float conductionVelocity_MMperMS,

		float FiberLengthInMM,
		float EndPlateLocationInMM
	);


//#define DEBUG_DISTANCE

#ifdef DEBUG_DISTANCE
static FILE *fibreDistTipFP_ = NULL;
static FILE *fibreDistCanFP_ = NULL;
#endif /* DEBUG_DISTANCE */


/*
 * ----------------------------------------------------------------
 * This function uses the saved muscle information
 * and makes motor-unit action potentials.
 * Only active motor units are considered.
 */
int
makeMUP(
		MuscleData *muscleDefinition,
		int **allMUPIds,
		int *nMUPs
	)
{
	MUPControl MUPControl;

	extern struct globals *g;


	memset(&MUPControl, 0, sizeof(MUPControl));

	MUPControl.MUPsPerMu = g->MUPs_per_mu;
	MUPControl.firingsDirectory = g->firings_dir;
	MUPControl.MUPDirectory = g->MUPs_dir;
	MUPControl.muscleDirectory = g->muscle_dir;
	MUPControl.MUPLength = MUP_LENGTH;
	MUPControl.electrodeType = g->electrode_type;
	MUPControl.stdDev_X = g->stddev_x;
	MUPControl.stdDev_Y = g->stddev_y;
	MUPControl.stdDev_Z = g->stddev_z;
	MUPControl.tipUptakeDistanceInMicrons = g->tipUptakeDistance;
	MUPControl.canUptakeDistanceInMicrons = g->canUptakeDistance;

	MUP::sSetJitterAccelerationThreshold(
		        g->jitterAccelThreshold);

	if ( muscleDefinition->nActiveInDetectMotorUnits_ > 0)
	{
		muscleDefinition->nActiveInDetectMotorUnits_ = 0;
		muscleDefinition->nActiveInDetectMotorUnitBlocks_ = 0;
		ckfree(muscleDefinition->activeInDetectMotorUnit_);
		muscleDefinition->activeInDetectMotorUnit_ = NULL;
	}

	/** truncate the old Peak-to-Peak file if we are overwriting */
	if (g->recordMFPPeakToPeak)
	{
		FILE *tfp;
		tfp = fopen(MFPP2PLOGFILE_NAME, "w");
		fclose(tfp);
	}

	return generateAllMUPs(
		        muscleDefinition,
		        &MUPControl,
		        allMUPIds,
		        nMUPs
		    );
}

static int
MUPGenerationLoop__(
		MuscleData *MD,
		MUPControl *MUPControl,
		int **allMUPIds,
		int *nMUPs
	)
{
	struct report_timer *reportTimer;
	MUP *currentMUP;
	int in_uptake_area;
	int MUPStatus;
	int i;

	LogInfo("Jitter Calculation Expansion is  : %s samples\n",
		    niceDouble(MUP::sGetExpansionFactor()));
	LogInfo("Jitter Acceleration Threshold is : %s v/ss\n",
		    niceDouble(MUP::sGetJitterAccelerationThreshold()));

	*nMUPs = MD->nActiveMotorUnits_;
	(*allMUPIds) = (int *)
		    ckalloc(MD->nActiveMotorUnits_ * sizeof(int));

	for (i = 0; i < MD->nActiveMotorUnits_; i++)
	{
		(*allMUPIds)[i] = (-1);
	}

	reportTimer = startReportTimer(MD->nActiveMotorUnits_);
	for (i = 0; i < MD->nActiveMotorUnits_; i++)
	{

		LogInfo("    Creating MFPs for MUP %s from MU %d\n",
		            reportTime(i+1, reportTimer),
		            MD->activeMotorUnit_[i]->mu_id_);

		/*
		 *  Iterate if more than one MUP/MU
		 */

		/* default is false, set in MUP() */
		in_uptake_area = 0;


		/*
		 * adjust needle position before entering MUP function
		 *
		 * distances are in mm
		 */
		currentMUP = new MUP(
		            MUPControl->MUPDirectory,
		            MD->activeMotorUnit_[i]->mu_id_);
		(*allMUPIds)[i] = MD->activeMotorUnit_[i]->mu_id_;

		MUPStatus = calculateMUP(
		            currentMUP,
		            0, /* MUP position index */
		            MUPControl,
		            MD->activeMotorUnit_[i]->mu_fibre_,
		            MD->activeMotorUnit_[i]->mu_nFibres_,
		            MD->getNeedleInfo(),
		            MD->activeMotorUnit_[i]->mu_id_,
		            &in_uptake_area,
		            MUPControl->stdDev_Z,
		            MUPControl->tipUptakeDistanceInMicrons,
		            MUPControl->canUptakeDistanceInMicrons
		        );

		if (!MUPStatus)
		{
		    LogInfo("MUP() failed.\n");
		    LogInfo("Aborting . . .\n");
		    return 0;
		}
		if (in_uptake_area && (currentMUP->getNMFPs() > 0))
		{
		    currentMUP->save();

		    listMkCheckSize(
		            MD->nActiveInDetectMotorUnits_ + 1,
		            (void **) &MD->activeInDetectMotorUnit_,
		            &MD->nActiveInDetectMotorUnitBlocks_,
		            16,
		            sizeof(MotorUnit *), __FILE__, __LINE__);
		    MD->activeInDetectMotorUnit_[
		                MD->nActiveInDetectMotorUnits_++
		            ] = MD->activeMotorUnit_[i];
		}
		delete currentMUP;
	}

	LogInfo("\n");

	deleteReportTimer(reportTimer);
	sCleanBuffers();
	sCleanLeftBuffers();

	{
		char filename[FILENAME_MAX];
		slnprintf(filename, FILENAME_MAX,
				"%s\\AMU-inDetect.dat", g->muscle_dir);
		MD->writeAMUInDetectInfo(filename);
	}

	return 1;
}

int
generateAllMUPs(
		MuscleData *MD,
		MUPControl *MUPControl,
		int **allMUPIds,
		int *nMUPs
	)
{
	int status;
	int space;
	int i;

	LogInfo("\n  -- Creating MUP potentials --\n");


#ifdef DEBUG_DISTANCE
	{
		char filename[FILENAME_MAX];

		slnprintf(filename, FILENAME_MAX, "%s\\fibre-distance-tip.txt",
		                MUPControl->muscleDirectory);
		fibreDistTipFP_ = fopenpath(filename, "wb");
		MSG_ASSERT(fibreDistTipFP_ != NULL, "Cannot open debug file");

		slnprintf(filename, FILENAME_MAX, "%s\\fibre-distance-can.txt",
		                MUPControl->muscleDirectory);
		fibreDistCanFP_ = fopenpath(filename, "wb");
		MSG_ASSERT(fibreDistCanFP_ != NULL, "Cannot open debug file");
	}
#endif /* DEBUG_DISTANCE */


	/* open active motor unit list file */

	LogInfo("\n");
	LogInfo(">> %d Active motor units\n", MD->nActiveMotorUnits_);
	space = 0;
	for (i = 0; i < MD->nActiveMotorUnits_; i++)
	{
		LogInfo(" %5d", MD->activeMotorUnit_[i]->mu_id_);
		space++;
		if (space == 10)
		{
		    space = 0;
		    LogInfo("\n");
		}
	}
	LogInfo("\n");


	status = MUPGenerationLoop__(
		        MD,
		        MUPControl,
		        allMUPIds,
		        nMUPs
		    );


#ifdef DEBUG_DISTANCE
	fclose(fibreDistTipFP_);
	fclose(fibreDistCanFP_);
#endif /* DEBUG_DISTANCE */

	return status;
}

static int
locateValidMUPs(
		int *MUPIdList,
		int totalPossibleMUPs,
		const char *firings_dir,
		const char *MUPs_dir
	)
{
	int i;

	/**
	 * test in turn whether each MUP/FT pair exists, keeping
	 * only those which do, loading each into the list at the
	 * index location starting with MUP 1 loaded into location 0
	 */
	for (i = 1; i <= totalPossibleMUPs; i++)
	{
		if (statFilenameFromMask(
					firings_dir,
					"FTMU%d.dat",
					i))
		{

			if (statFilenameFromMask(
					MUPs_dir,
					"MUPData%04d.dat",
					i))
			{
				MUPIdList[i - 1] = i;
			}
		}
	}

	return 1;
}

static void printLogInfo(MUP *newMUP, int nTotalActiveFibres)
{
	int nFibres = newMUP->getNMFPs();

	if (nFibres > 0)
	{
		LogInfo("      - MUP contains %d fibres, %d `near' MFPs\n",
		                    nTotalActiveFibres, nFibres - 1);
	} else
	{
		LogInfo("      - MUP of %d fibres outside of uptake region\n",
		                    nTotalActiveFibres);
	}
	LogInfo("\n");
}


/*
 * ----------------------------------------------------------------
 * This is the procedure to calculate the motor unit action
 * potential (MUP).
 * ----------------------------------------------------------------
 * This procedure uses the following MFP-generation routines, which
 * calculate the individual action potentials:
 *     calculateSingleFibreMFP()
 *     calculateConcentricMFAP()
 *     calculateBipolarMFAP()
 *
 *     calculateSingleFibreMFPWithInitiation()
 *     calculateConcentricMFAPWithInitiation()
 *     calculateBipolarMFAPWithInitiation()

 */

static int
calculateMUP(
		MUP *newMUP,
		int MUPId,
		MUPControl *MUPControl,
		MuscleFibre **muscleFibre,
		int nTotalActiveFibres,
		NeedleInfo *needle,
		int mu_number,
		int *inUptakeAreaFlag,
		float zPlacementStdDev,
		int tipUptakeDistanceInMicrons,
		int canUptakeDistanceInMicrons
	)
{
	struct report_timer *reportTimer;
	double *convolutionResult;
		time_t lastTime, curTime;

	// float cond_delay;

	/* mm/ms <-> M/s conduction velocity of AP */
	float conductionVelocity_MMperMS;

	float diameterInMM;
	float zEndplateDistanceInMM;
	float radialSeparationInMM;
	float auxRadialSeparationInMM;

	/* */
	float FiberLengthInMM;
	float EndPlateLocationInMM;

	FiberLengthInMM = 100.0;
	EndPlateLocationInMM = 50.0;
	/* */

	double tempvar;
	int fibreIndex;

	extern struct globals *g;


	/* electrode relative x location of fibre in mm */
	float muscleFibreXLocationInMM;

	convolutionResult = sGetConvolutionBuffer(MUPControl->MUPLength);

		lastTime = time(NULL);
	reportTimer = startReportTimer(nTotalActiveFibres);
	for (fibreIndex = 0; fibreIndex < nTotalActiveFibres; fibreIndex++)
	{

		curTime = time(NULL);

		if (((fibreIndex + 1) % 250 == 0) || (curTime - lastTime > 1))
		{
			LogInfo("            Fibre %s\n",
		            reportTime(fibreIndex, reportTimer));
		}
		lastTime = curTime;

		/* in mm */
		zEndplateDistanceInMM = (float)
		        (needle->getZInMM()
		                + muscleFibre[fibreIndex]->mf_jShift_);

		/*
		 * First determing if we are "close enough" independently
		 * of what needle we are using
		 */
		tempvar = sqrt(SQR(needle->getXTipInMM()
		                - (muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM))
		        + SQR((needle->getYTipInMM() + .025)
		                - (muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM)));


		/* if we are not in the uptake area, skip to next fibre */
		if (tempvar > (tipUptakeDistanceInMicrons / 1000.0))
			continue;


		/*
		 * Now calculate values to control current equation
		 */
		if (MUPControl->electrodeType == 1)
		{
		    tempvar = SQR(needle->getXTipInMM()
		                - (muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM))
		        + SQR((needle->getYTipInMM() + .025)
		                - (muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM));
		    /*
		     * if less than fibre radius squared, set
		     * radialSeparationInMM equal to fibre radius
		     */
		    if (tempvar < SQR(muscleFibre[fibreIndex]->mf_diameter_ / 2000.0))
		        radialSeparationInMM = (float)
		                (muscleFibre[fibreIndex]->mf_diameter_ / 2000.0);
		    else
		        radialSeparationInMM = (float) sqrt(tempvar);


		} else if (MUPControl->electrodeType == 2)
		{

		    /*
		     * figure out the Y distance in MM, adjusting to be from
		     * the center of the cell to the needle tip center
		     *
		     * (adjustment is the distance to the center of the cell)
		     */
		    tempvar = fabs(
						(muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM)
							- needle->getYTipInMM()
					) + 0.0250;

		    /* if less than fibre radius set equal to fibre radius */
		    if (tempvar < muscleFibre[fibreIndex]->mf_diameter_
		                        / (2 * 1000.0))
			{
		        radialSeparationInMM = (float)
		                (muscleFibre[fibreIndex]->mf_diameter_
		                        / (2 * 1000.0));
		    } else
			{
		        radialSeparationInMM = (float) tempvar;
		    }

		    //float radial_dist;

		    // Major axis of concentric needle is 290 um
//		    if (fabs(
//						(muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM)
//						- needle->getXTipInMM()
//					) <= 0.290)
//			{
//		        radial_dist = radialSeparationInMM;
//		    } else
//			{
//		        radial_dist = (float) sqrt(
//		            SQR(fabs(muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM
//								- needle->getXTipInMM()) - 0.290)
//		                + SQR(radialSeparationInMM)
//		            );
//		    }

		} else if (MUPControl->electrodeType == 3 )
		{
		    //tempvar = (needle->getYTipInMM() + .025)
		    //            - (muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM);

		    /* adjustment is the distance to the center of the cell */
		    tempvar = fabs(
		            (muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM)
		                    - needle->getYTipInMM())
		            + 0.0250;

		    /*
		     * if distance less than fibre radius set equal to
			 * the fibre radius (in mm)
		     */
		    if (tempvar < (muscleFibre[fibreIndex]->mf_diameter_ / 2000.0))
			{
		        radialSeparationInMM = (float)
		                (muscleFibre[fibreIndex]->mf_diameter_
						 		/ 2000.0);
			} else
			{
		        radialSeparationInMM = (float) tempvar;
			}

//		    float radial_dist;
//		    // Radius of Monopolar electrode is 100 um
//		    if (fabs(muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM
//								- needle->getXTipInMM()) <= 0.100)
//		        radial_dist = radialSeparationInMM;
//		    else
//		        radial_dist = (float) sqrt(
//		            SQR(fabs(muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM
//								- needle->getXTipInMM()) - 0.100)
//		                + SQR(radialSeparationInMM)
//		            );

		} else if (MUPControl->electrodeType == 4)
		{
		    tempvar =
		        SQR((BIPOLE_SEP / 2)
						- (muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM
								- needle->getXTipInMM()))
		            + SQR((.025)
						- (muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM
								- needle->getYTipInMM()));

		    /*
		     * if less than fibre radius squared, set
		     * radialSeparationInMM equal to fibre radius
		     *
		     * in mm
		     */
		    if (tempvar < SQR(muscleFibre[fibreIndex]->mf_diameter_
									/ 2000.0))
			{
		        radialSeparationInMM = (float)
		                (muscleFibre[fibreIndex]->mf_diameter_
						 		/ 2000.0);
			} else
			{
		        radialSeparationInMM = (float) sqrt(tempvar);
			}

		    tempvar =
		        SQR((-BIPOLE_SEP / 2)
						- (muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM
								- needle->getXTipInMM()))
		            + SQR((.025)
						- (muscleFibre[fibreIndex]->mf_yCell_ / CELLS_PER_MM
								- needle->getYTipInMM()));

		    /*
		     * if less than fibre radius squared,
		     * set radialSeparationInMM equal to fibre radius in mm
		     */
		    if (tempvar < SQR(muscleFibre[fibreIndex]->mf_diameter_
		                        / 2000.0))
			{
		        auxRadialSeparationInMM = (float)
		                (muscleFibre[fibreIndex]->mf_diameter_
						 		/ 2000.0);
			} else
			{
		        auxRadialSeparationInMM = (float) sqrt(tempvar);
			}
		}

		/* in mm */
		diameterInMM = (float)
		            (muscleFibre[fibreIndex]->mf_diameter_ / 1000.0);

		/*
		 * velocity of wave, from Nandedkar
		 */
		conductionVelocity_MMperMS = (float)
		        (2.2 + 0.05 * (diameterInMM * 1000.0 - 25.0));


		/* set *inUptakeAreaFlag true */
		(*inUptakeAreaFlag) = 1;


		/* conduction delay in ms  */
		// cond_delay = zEndplateDistanceInMM
		//			/ conductionVelocity_MMperMS;

		if (MUPControl->electrodeType == 1)
		{


			if (g->generateMFPsWithoutInitiation)
			{
				if ( ! calculateSingleFibreMFP(
							MUPControl->MUPLength,
							convolutionResult,
							zEndplateDistanceInMM,
							radialSeparationInMM,
							diameterInMM,
							conductionVelocity_MMperMS
						))
				{
					return 0;
				}
			} else
			{
				if ( ! calculateSingleFibreMFPWithInitiation(
							MUPControl->MUPLength,
							convolutionResult,
							zEndplateDistanceInMM,
							radialSeparationInMM,
							diameterInMM,
							conductionVelocity_MMperMS,
							FiberLengthInMM,
							EndPlateLocationInMM
						))
				{
					return 0;
				}
			}

		} else if (MUPControl->electrodeType == 2
					|| MUPControl->electrodeType == 3)
		{

			/* in mm */
			muscleFibreXLocationInMM = (float)
					((muscleFibre[fibreIndex]->mf_xCell_ / CELLS_PER_MM)
							- needle->getXTipInMM());

			if (g->generateMFPsWithoutInitiation){
				if ( ! calculateConcentricMFAP(
							(float) muscleFibre[fibreIndex]->mf_xCell_,
							(float) muscleFibre[fibreIndex]->mf_yCell_,
							fibreIndex,
							MUPControl->MUPLength,
							MUPControl->electrodeType,
							convolutionResult,
							zEndplateDistanceInMM,
							radialSeparationInMM,
							diameterInMM,
							conductionVelocity_MMperMS,
							muscleFibreXLocationInMM
						))
					return 0;
			} else
			{
				if (! calculateConcentricMFAPWithInitiation(
							(float) muscleFibre[fibreIndex]->mf_xCell_,
							(float) muscleFibre[fibreIndex]->mf_yCell_,
							fibreIndex,
							MUPControl->MUPLength,
							MUPControl->electrodeType,
							convolutionResult,
							zEndplateDistanceInMM,
							radialSeparationInMM,
							diameterInMM,
							conductionVelocity_MMperMS,
							muscleFibreXLocationInMM,
							FiberLengthInMM,
							EndPlateLocationInMM
						))
					return 0;
			}

		} else if (MUPControl->electrodeType == 4)
		{
			if (g->generateMFPsWithoutInitiation)
			{
				if ( ! calculateBipolarMFAP(
						MUPControl->MUPLength,
						convolutionResult,
						zEndplateDistanceInMM,
						radialSeparationInMM,
						auxRadialSeparationInMM,
						diameterInMM,
						conductionVelocity_MMperMS
					))
					return 0;
			} else
			{
				if (! calculateBipolarMFAPWithInitiation(
					MUPControl->MUPLength,
					convolutionResult,
					zEndplateDistanceInMM,
					radialSeparationInMM,
					auxRadialSeparationInMM,
					diameterInMM,
					conductionVelocity_MMperMS,
					FiberLengthInMM,
					EndPlateLocationInMM
					))
					return 0;
			}
		}


		// now save the convolution result into
		// the MUP we are building up
		if (newMUP != NULL)
		{

			newMUP->addMFP(
					MUPId,
					MUPControl->MUPLength,
					convolutionResult,
					fibreIndex
				);

			if (g->recordMFPPeakToPeak)
			{
				FILE *tfp;
				tfp = fopen(MFPP2PLOGFILE_NAME, "a");
				fprintf(tfp, "%f\n",
						calcPeakToPeakDifferenceDouble(convolutionResult,
									MUPControl->MUPLength));
				fclose(tfp);
			}

			if (g->generateMFPsWithoutInitiation){
				if ( ! calculateCannulaMFAP(
						(float) muscleFibre[fibreIndex]->mf_xCell_,
						(float) muscleFibre[fibreIndex]->mf_yCell_,
						fibreIndex,
						MUPControl->MUPLength,
						convolutionResult,
						zEndplateDistanceInMM,
						(float) (muscleFibre[
										fibreIndex
									]->mf_xCell_ / CELLS_PER_MM),
						(float) (muscleFibre[
										fibreIndex
									]->mf_yCell_ / CELLS_PER_MM),
						needle,
						diameterInMM,
						conductionVelocity_MMperMS
					))
					return 0;
			}else{
				if (! calculateCannulaMFAPWithInitiation(
						(float) muscleFibre[fibreIndex]->mf_xCell_,
						(float) muscleFibre[fibreIndex]->mf_yCell_,
						fibreIndex,
						MUPControl->MUPLength,
						convolutionResult,
						zEndplateDistanceInMM,
						(float) (muscleFibre[
										fibreIndex
									]->mf_xCell_ / CELLS_PER_MM),
						(float) (muscleFibre[
										fibreIndex
									]->mf_yCell_ / CELLS_PER_MM),
						needle,
						diameterInMM,
						conductionVelocity_MMperMS,

						FiberLengthInMM,
						EndPlateLocationInMM

					))
					return 0;
			}

			newMUP->addCannulaMFP(
					MUPId,
					MUPControl->MUPLength,
					convolutionResult
				);

		}
	}
	deleteReportTimer(reportTimer);

	printLogInfo(newMUP, nTotalActiveFibres);

	return (1);
}

/*
 * Current constant used in all needle functions below.
 */
static double currentConstant(double fibreDiameterInMM)
{
		/*Intracellular conductivity mhos/mm */
	const double sigmai = 0.00101;

	/*
	 * From Nandedkar's paper
	 *
	 * 768z^3 e^{-2z} - 90
	 *
	 * which comes from 2z into Rosenfalk's original.
	 */
	return ((3072 * sigmai * M_PI) / 4.0) * SQR(fibreDiameterInMM);
}

/*
 * ----------------------------------------------------------------
 *
 * This procedure calculates the muscle fibre action potential
 * (mfap) that is seen by the electrode. The procedure is called
 * from MUP.
 *
 * zEndplateDistanceInMM   : distance of electrode from end plate
 * radialSeparationInMM     : how far electrode is from muscle fibre
 * conductionVelocity_MMperMS       : conduction velocity of muscle fibre
 * weightfn       : two working vectors for the convolution
 * current        : These have been passed in order to save
 *    : memory.  Otherwise, they would be
 *    : re-made on the stack every time they
 *    : are used in a lower subroutine
 * convolution    : the convolution of the response function
 *    : with the muscle fibre action potential
 *
 * This procedure uses the procedure convolve.
 */
static int
calculateSingleFibreMFP(
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float radialSeparationInMM,
		float diameterInMM,
		float conductionVelocity_MMperMS
	)
{
	double      *fft;
	double      *weightfn;
	double      *current;

		/*Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/*Radial conductivity mhos/mm */
	const double sigmar = 0.000063;
		/*Ratio of Sigmar to sigmaz* */
	const double kratio = sigmaz / sigmar;
	double weightfn_denom;
		/* the fractional multiplier - weightfn   */
	double weightfn_const;
		/* the fractional multiplier - currentfn  */
	double currentfn_const;
		/* sampling increment along z -axis  */
	float z_inc;
		/* position along z axis    */
	float z;
	int i, j;



	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);

		/*  Scale factor in weight fn eqtn. */
	weightfn_const = 1.0 / (4.0 * M_PI * sigmar);
		/*  Scale factor in current eqtn. */
	currentfn_const = currentConstant(diameterInMM);

	z = 0.0;
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);
	zEndplateDistanceInMM =
		        (float) (floor(zEndplateDistanceInMM / z_inc) * z_inc);

	weightfn[0] = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;

	for (j = 1; j <= MUPLength * 2; j++)
	{
		weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

		weightfn[j] = (double) (weightfn_const / weightfn_denom);

		current[j] = (double)
		    (-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) *
		     exp(-2.0 * z));
		z += z_inc;
	}

	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    DELTA_T_MUP
		);


#ifdef          UNUSED
	/* ramp the convolution artifact down to zero within 50 samples */
	adjust = convolution[50] / 49.0;
	for (i = 49; i > 0; i--)
		convolution[i] = adjust * i;
#endif


	/*
	 * adjust for convolution artifact by setting
	 * first 50 result values to the value of the 50th
	 */
	for (i = 49; i > 0; i--)
		convolution[i] = convolution[50];

	return 1;
}

/** This procedure uses the same code as used in the previous one
 ** with some added lines for the initiation part.
 **/

 static int
calculateSingleFibreMFPWithInitiation(
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float radialSeparationInMM,
		float diameterInMM,
		float conductionVelocity_MMperMS,
		float FiberLengthInMM,
		float EndPlateLocationInMM
	)
{
	double      *fft;
	double      *weightfn;
	double      *current;
	double		*convLeft;
	double		*weightfnLeft;
	double		*fftLeft;

		/*Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/*Radial conductivity mhos/mm */
	const double sigmar = 0.000063;
		/*Ratio of Sigmar to sigmaz* */
	const double kratio = sigmaz / sigmar;
	double weightfn_denom;
		/* the fractional multiplier - weightfn   */
	double weightfn_const;
		/* scale factor */
	double SignalDurationInTime;
		/* the fractional multiplier - currentfn  */
	double currentfn_const;
		/* sampling increment along z -axis  */
	float z_inc;
		/* position along z axis    */
	float z;
	int j;

	int N_left, N_right, max_N, NI;


	convLeft = sGetConvLeftBuffer(MUPLength);
	sGetFFTLeftBuffers(MUPLength, &fftLeft, &weightfnLeft);

	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);

	weightfn_const = scale_factor / (4.0 * M_PI * sigmar);
		/*  Scale factor in current eqtn. */
	currentfn_const = currentConstant(diameterInMM);

	z = 0.0;
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);
	zEndplateDistanceInMM =
		        (float) (floor(zEndplateDistanceInMM / z_inc) * z_inc);

	SignalDurationInTime = 5.0; /* In ms */
	N_left = (int) floor( EndPlateLocationInMM / z_inc);
	N_right = (int) floor(FiberLengthInMM / z_inc) - N_left;

	NI = (int) floor(SignalDurationInTime / DELTA_T_MUP);

	weightfn[0] = 0.0;
	weightfnLeft[0] = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;
	convLeft[0] = 0.0;


	max_N = MAX(N_right,N_left);

	for (j = 1 ; j <= max_N ; j++)
	{
		if (j<=N_right)
		{
			weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));
			weightfn[j] = (double) (weightfn_const / weightfn_denom);
		}
		if(j<=N_left)
		{
			weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(-z - zEndplateDistanceInMM));
			weightfnLeft[j] = (double) (weightfn_const / weightfn_denom);
		}
		if(j<=NI)
		{
			if (j==CurrCorr)
				current[j] = optZinit(z_inc,currentfn_const,NI);
			else
				current[j] = (double)
					(-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) *
					    exp(-2.0 * z));
		}

		z += z_inc; /* In MM */
	}

	convolve(fft,
			    weightfn, MUPLength * 2 ,
			    current, MUPLength * 2 ,
				1,
				convolution,
				z_inc
		);

	/*plot_mah(convolution,MUPLength*4+1);*/


	convolve(fftLeft,
			weightfnLeft, MUPLength * 2 ,
			current, MUPLength * 2 ,
			1,
			convLeft,
			z_inc
			);


	adjustConvArtifact(
				NI, N_left+N_right,
				N_right,
				N_left,
			    z_inc,
				/*diameterInMM,*/
				convLeft, convolution,
				current,
				weightfnLeft, weightfn
				);


   /* transforming from space domain into time domain */
   /* in convolve function, z_inc should be replaced by DELTA_T_MUP */
	for (j = 1; j <= MUPLength*2; j++)
	   convolution[j]=convolution[j]/conductionVelocity_MMperMS;

   /*plot_mah(convolution,MUPLength*2);*/

	return 1;



}


/*
 * ----------------------------------------------------------------
 * This procedure calculates the muscle fibre action potential
 * (mfap) that is seen by the bipolar electrode. The procedure
 * is called from MUP.
 *
 * zEndplateDistanceInMM   : distance of electrode from end plate
 * radialSeparationInMM     : how far electrode is from muscle fibre
 * auxRadialSeparationInMM    : how far electrode is from muscle fibre
 * conductionVelocity_MMperMS       : conduction velocity of muscle fibre
 * weightfn       : two working vectors for the convolution
 * current        : These have been passed in order to save
 *                        : memory.  Otherwise, they would be
 *                        : re-made on the stack every time they
 *                        : are used in a lower subroutine
 * convolution    : the convolution of the response function
 *                        : with the muscle fibre action potential
 *
 * This procedure uses the procedure convolve.
 */
static int
calculateBipolarMFAP(
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float radialSeparationInMM,
		float auxRadialSeparationInMM,
		float diameterInMM,
		float conductionVelocity_MMperMS
	)
{
	double *fft;
	double *weightfn;
	double *current;

		/* Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/* Radial conductivity mhos/mm */
	const double sigmar = 0.000063;


		/* Ratio of Sigmar to sigmaz */
	const double kratio = sigmaz / sigmar;

	double weightfn_denom;
		/* the fractional multiplier - weightfn   */
	double weightfn_const;
		/* the fractional multiplier - currentfn  */
	double currentfn_const;


	float z_inc;        /* sampling increment along z -axis  */
	float z;            /* position along z axis    */
	int i, j;


	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);


		/*  Scale factor in weight fn eqtn. */
	weightfn_const = 1.0 / (4.0 * M_PI * sigmar);

		/*  Scale factor in current eqtn. */
	currentfn_const = currentConstant(diameterInMM);





	z = 0.0;
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);
	zEndplateDistanceInMM = (float)
				(floor(zEndplateDistanceInMM / z_inc) * z_inc);

	weightfn[0] = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;

	for (j = 1; j <= MUPLength * 2; j++)
	{
		weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

		weightfn[j] = (double) (weightfn_const / weightfn_denom);

		current[j] = (double)
		    (-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) *
		     exp(-2.0 * z));
		z += z_inc;
	}

	z = 0.0;
	for (j = 1; j <= MUPLength * 2; j++)
	{
		weightfn_denom = sqrt(kratio * SQR(auxRadialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

		weightfn[j] -= (double) (weightfn_const / weightfn_denom);

		z += z_inc;
	}

	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    DELTA_T_MUP
		);

#ifdef          UNUSED
	/* ramp the convolution artifact down to zero within 50 samples */
	adjust = convolution[50] / 49.0;
	for (i = 49; i > 0; i--)
		convolution[i] = adjust * i;
#endif

	/*adjust for convolution artifact by setting
	   first 50 result values to the value of the 50th */

	for (i = 49; i > 0; i--)
		convolution[i] = convolution[50];


	return 1;
}




/** This procedure uses the same code as used by the previous one with some
 ** added lines for the initiation part.
 **/

static int
calculateBipolarMFAPWithInitiation(
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float radialSeparationInMM,
		float auxRadialSeparationInMM,
		float diameterInMM,
		float conductionVelocity_MMperMS,

		float FiberLengthInMM,
		float EndPlateLocationInMM
	)
{
	double *fft;
	double *weightfn;
	double *current;
	double		*convLeft;
	double		*weightfnLeft;
	double		*fftLeft;
	//double		*convRight;


		/* Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/* Radial conductivity mhos/mm */
	const double sigmar = 0.000063;


		/* Ratio of Sigmar to sigmaz */
	const double kratio = sigmaz / sigmar;

	double weightfn_denom;
		/* the fractional multiplier - weightfn   */
	double weightfn_const;
		/* the fractional multiplier - currentfn  */
	double currentfn_const;
		/* scale factor */
	double SignalDurationInTime;


	float z_inc;        /* sampling increment along z -axis  */
	float z;            /* position along z axis    */
	int i, j;

	int N_left, N_right, NI;

	int Limit;				/*  upper limit for the for loop **/


	convLeft = sGetConvLeftBuffer(MUPLength);
	//convRight = sGetConvRightBuffer(MUPLength);

	sGetFFTLeftBuffers(MUPLength, &fftLeft, &weightfnLeft);

	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);


		/*  Scale factor in weight fn eqtn. */
	weightfn_const = 1.0 / (4.0 * M_PI * sigmar);

		/*  Scale factor in current eqtn. */
	currentfn_const = currentConstant(diameterInMM);


	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);
	zEndplateDistanceInMM = (float)
				(floor(zEndplateDistanceInMM / z_inc) * z_inc);

	SignalDurationInTime = 5.0; /* In ms */
	N_left = (int) floor( EndPlateLocationInMM / z_inc);
	N_right = (int) floor(FiberLengthInMM / z_inc) - N_left;

	NI = (int) floor(SignalDurationInTime / DELTA_T_MUP);

	weightfn[0] = 0.0;
	weightfnLeft[0] = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;
	convLeft[0] = 0.0;



	for (i = 0; i < 2; i++)
	{
		Limit = i * N_left + (1-i) * N_right;

		z = 0.0;
		for (j = 1; j <= Limit; j++)
		{

			if (i==0){
				weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

				weightfn[j] = (double) (weightfn_const / weightfn_denom);


				weightfn_denom = sqrt(kratio * SQR(auxRadialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

				weightfn[j] -= (double) (weightfn_const / weightfn_denom);

			}

			if (i==1){
				weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z + zEndplateDistanceInMM));

				weightfnLeft[j] = (double) (weightfn_const / weightfn_denom);

				weightfn_denom = sqrt(kratio * SQR(auxRadialSeparationInMM)
		                        + SQR(z + zEndplateDistanceInMM));

				weightfnLeft[j] -= (double) (weightfn_const / weightfn_denom);
			}

			z += z_inc;
		}
	}

	z = 0.0;
	for ( j = 1; j <= NI; j++)
	{

		if (j==CurrCorr)
				current[j] = optZinit(z_inc,currentfn_const,MUPLength);

		else
			current[j] = (double)
		    (-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) *
		     exp(-2.0 * z));

		z += z_inc;
	}

	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    z_inc
		);

	plot_mah(convolution,N_right+10);


	convolve(fftLeft,
			weightfnLeft, MUPLength * 2,
			current, MUPLength * 2,
			1,
			convLeft,
			z_inc
		 );
	plot_mah(convLeft,N_right+N_left);

	adjustConvArtifact(
				NI, N_right+N_left,
				N_right,
				N_left,
			    z_inc,
				convLeft, convolution,
				current,
				weightfnLeft, weightfn
				);

   /* transforming from space domain into time domain */
   /* in convolve function, z_inc should be replaced by DELTA_T_MUP */

	for (j = 1; j <= MUPLength*2; j++)
	   convolution[j]=convolution[j]/conductionVelocity_MMperMS;

   /*plot_mah(convolution,MUPLength*2);*/



  /*  for (i = 0; i < 2; i++)
	{
		Limit = i * N_left + (1-i) * N_right;

		z = 0.0;
		for (j = 1; j <= Limit; j++)
		{

			if (i==0){
				weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

				weightfn[j] = (double) (weightfn_const / weightfn_denom);
			}

			if (i==1){
				weightfn_denom = sqrt(kratio * SQR(radialSeparationInMM)
		                        + SQR(z + zEndplateDistanceInMM));

				weightfnLeft[j] = (double) (weightfn_const / weightfn_denom);
			}

			z += z_inc;
		}
	}


	 z = 0.0;
	for ( j = 1; j <= NI; j++)
	{

		if (j==CurrCorr)
				current[j] = optZinit(z_inc,currentfn_const,MUPLength);

		else
			current[j] = (double)
		    (-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) *
		     exp(-2.0 * z));

		z += z_inc;
	}

	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    DELTA_T_MUP
		);

	plot_mah(convolution,N_right+10);


	convolve(fftLeft,
			weightfnLeft, MUPLength * 2,
			current, MUPLength * 2,
			1,
			convLeft,
			DELTA_T_MUP
		 );
	plot_mah(convLeft,N_right+N_left);

	adjustConvArtifact(
				NI, N_right+N_left,
				N_right,
				N_left,
			    z_inc,

				convLeft, convolution,
				current,
				weightfnLeft, weightfn
				);

	for (i = 0; i < 2; i++)
	{
		Limit = i * N_left + (1-i) * N_right;

		z = 0.0;
		for (j = 1; j <= Limit; j++)
		{

			if (i==0){
				weightfn_denom = sqrt(kratio * SQR(auxRadialSeparationInMM)
		                        + SQR(z - zEndplateDistanceInMM));

				weightfn[j] = (double) (weightfn_const / weightfn_denom);
			}

			if (i==1){
				weightfn_denom = sqrt(kratio * SQR(auxRadialSeparationInMM)
		                        + SQR(z + zEndplateDistanceInMM));

				weightfnLeft[j] = (double) (weightfn_const / weightfn_denom);
			}

			z += z_inc;

		}
	}

	plot_mah(current,NI);


	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convRight,
		    DELTA_T_MUP
		);

	plot_mah(convolution,N_right+10);


	convolve(fftLeft,
			weightfnLeft, MUPLength * 2,
			current, MUPLength * 2,
			1,
			convLeft,
			DELTA_T_MUP
		 );
	plot_mah(convLeft,N_right+N_left);

	adjustConvArtifact(
				NI, N_right+N_left,
				N_right,
				N_left,
			    z_inc,

				convLeft, convRight,
				current,
				weightfnLeft, weightfn
				);

	for (i=1 ; i<= MAX(N_right,N_left) ; i++)
		convolution[i] -= convRight[i];


	plot_mah(convolution,N_right+N_left);*/

	return 1;
}




/*
 * ----------------------------------------------------------------
 * This procedure calculates the muscle fibre action potential
 * (mfap) for a concentric needle that is seen by the electrode.
 *
 * The procedure is called from MUP.
 *
 *
 * zEndplateDistanceInMM                | distance of electrode from end plate
 * deltay                      : how far electrode is from muscle fibre
 * conductionVelocity_MMperMS            : conduction velocity of muscle fibre
 * weightfn            : two working vectors for the convolution
 * current                     : These are global       in order to save
 *                             : memory.  Otherwise, they would be
 *                             : re-made on the stack every time they
 *                             : are used in a lower subroutine
 * convolution         : the convolution of the response function
 *                             : with the muscle fibre action potential
 *                             |
 * muscleFibreXLocationInMM            : electrode relative fibre x location in mm
 *
 * This procedure uses the procedure convolve.
 */
static int
calculateConcentricMFAP(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		int electrodeType,
		double *convolution,
		float zEndplateDistanceInMM,
		float deltay,
		float diameterInMM,
		float conductionVelocity_MMperMS,
		float muscleFibreXLocationInMM
	)
{
	double *fft;
	double *weightfn;
	double *current;

		/*Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/*Radial conductivity mhos/mm */
	const double sigmar = 0.000063;
		/*Ratio of Sigmar to sigmaz* */
	const double kratio = sigmaz / sigmar;
		/*Semi major axis of ellipse(circle) */
	int semimajor;
		/*Semi minor axis of ellipse(circle) */
	int semiminor;
	float adjust;

	double B;
		/*  the fractional multiplier - weightfn   **/
	double A;
		/*  current vector constant  **/
	double currentfn_const;

	float x_posEnd;  /*  x position of the ends of the line electrode **/
	float x_negEnd;
	float delta_x_posEnd, delta_x_negEnd;

		/*  x coordinates on perimeter of ellipse **/
	float xellipse;

	float z_inc;        /*  increment along the z axis        **/
	float z;            /*  position along the z axis    **/
	float zellipse;     /*  position of point source **/
	float weightz;      /*  weighting function calculated at line z **/

	double ee;          /*  the exponential in the mfap fcn   **/
	int i, j, l;        /*  index variables        **/



	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);


	if (electrodeType == 2)
	{
		semimajor = 290;                /* major axis in microns */
		semiminor = 75;                 /* minor axis in microns */
	}
	if (electrodeType == 3)
	{
		semimajor = 100;                /* major axis in microns */
		semiminor = 100;                /* minor axis in microns */
	}


		/*  Scale factor in weight fn eqtn. */
	A = 1.0 / (4.0 * M_PI * sigmar) / sqrt(kratio);

		/*  Scale factor in current eqtn. */
	currentfn_const = currentConstant(diameterInMM);


	/*
	 * start zellipse one half "step" in from edge of
	 * ellipse (or circle); 6 steps will take us to one half step
	 * from other edge
	 */
	zellipse = (float) (-semiminor / 3.0 * 2.5);


		/*  Calculate z step size  in mm **/
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);

		/* adjust to integral number of z steps */
	zEndplateDistanceInMM = (float)
		        (floor(zEndplateDistanceInMM / z_inc) * z_inc);


	weightz = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;
	memset(weightfn, 0, sizeof(double) * MUPLength * 2);


	/* calculate 6 line integrals */
	for (l = 0; l < 6; l++)
	{

		/* calulate limits of integration */
		xellipse =
		    (float) (sqrt
		             (SQR(semimajor) -
		                    SQR(semimajor) * SQR(zellipse)
		                            / SQR(semiminor)));

		    /* one   ellipse relative x location in microns */
		x_posEnd = xellipse;
		    /* other ellipse relative x location in microns */
		x_negEnd = xellipse * (-1);


		    /* one   x distance from fibre to electrode in mm */
		delta_x_posEnd = (float)
		        ((x_posEnd / 1000.0) - muscleFibreXLocationInMM);
		    /* other x distance from fibre to electrode in mm */
		delta_x_negEnd = (float)
		        ((x_negEnd / 1000.0) - muscleFibreXLocationInMM);


		z = 0.0;
		for (j = 1; j <= MUPLength * 2; j++)
		{
		    B = SQR(deltay)
		            + SQR(zEndplateDistanceInMM - z
		                                - (zellipse / 1000.0))
		                        / kratio;

			if (delta_x_negEnd > 0)
			{
		        /*
		         * the fibre is on the negative co-ordinate side
		         * of the electrode
		         */
				weightz = (float) (log(
		                fabs((delta_x_posEnd
		                        + sqrt(SQR(delta_x_posEnd) + B)))
		                    / (delta_x_negEnd
		                        + sqrt(SQR(delta_x_negEnd) + B))
		                ));

			} else if (delta_x_posEnd < 0)
			{
		        /*
		         * the fibre is on the positive co-ordinate
		         * side of the electrode
		         */
				weightz = (float) (log(
		                fabs((-delta_x_negEnd
		                        + sqrt(SQR(delta_x_negEnd) + B)))
		                    / (-delta_x_posEnd
		                        + sqrt(SQR(delta_x_posEnd) + B))
		                ));

			} else
			{
		        /*
		         * the fibre is under the electrode,
		         * do the calc in two parts
		         */
				weightz = (float) (
						log(
							fabs((-delta_x_negEnd
		                            + sqrt(SQR(delta_x_negEnd) + B)))
		                        / sqrt(B)
								)
						+ log(
							fabs((delta_x_posEnd
		                            + sqrt(SQR(delta_x_posEnd) + B)))
								/ sqrt(B)
								)
							);
			}

			// divide out the length of the line electrode
		    weightfn[j] += (double)
		            (fabs(weightz /
		                    (fabs(x_posEnd - x_negEnd) / 1000.)));

		    z += z_inc;
		}

		    /* increment to next line integral */
		zellipse = (float) (zellipse + semiminor / 3.0);
		weightz = 0.0;
	}


	z = 0.0;
	for (i = 1; i <= MUPLength * 2; i++)
	{

		/*  exponential term  **/
		ee = exp(-2.0 * z);


		current[i] = (double)
		        (-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) * ee);

		z += z_inc;

		/* average weighting fn **/
		weightfn[i] = (double) (A * weightfn[i] / 6.);
	}

	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    DELTA_T_MUP
		);


	/*
	 * ramp the convolution artifact down to zero
	 * within the first 50 samples
	 */
	adjust = (float) (convolution[50] / 49.0);
	for (i = 49; i > 0; i--)
	{
		convolution[i] = adjust * i;
	}

	/* ramp the convolution down to zero within the last 50 samples */
	adjust = (float) (convolution[MUPLength * 2 - 50] / 49.0);
	for (i = 49; i >= 0; i--)
	{
		convolution[MUPLength * 2 - i] = adjust * i;
	}

	return 1;
}


/** This procedure uses the same code as used by the previous one with some
 ** added lines for the initiation part.
 **/


static int
calculateConcentricMFAPWithInitiation(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		int electrodeType,
		double *convolution,
		float zEndplateDistanceInMM,
		float deltay,
		float diameterInMM,
		float conductionVelocity_MMperMS,
		float muscleFibreXLocationInMM,

		float FiberLengthInMM,
		float EndPlateLocationInMM
	)
{
	double *fft;
	double *weightfn;
	double *current;
	double *weightfn_left;
	double *fft_left;
	double *convLeft;

		/*Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/*Radial conductivity mhos/mm */
	const double sigmar = 0.000063;
		/*Ratio of Sigmar to sigmaz* */
	const double kratio = sigmaz / sigmar;
		/*Semi major axis of ellipse(circle) */
	int semimajor;
		/*Semi minor axis of ellipse(circle) */
	int semiminor;
	float SignalDurationInTime;
	int NI;


	double B;
		/*  the fractional multiplier - weightfn   **/
	double A;
		/*  current vector constant  **/
	double currentfn_const;

	float x_posEnd;  /*  x position of the ends of the line electrode **/
	float x_negEnd;
	float delta_x_posEnd, delta_x_negEnd;

		/*  x coordinates on perimeter of ellipse **/
	float xellipse;

	float z_inc;        /*  increment along the z axis        **/
	float z;            /*  position along the z axis    **/
	float zellipse;     /*  position of point source **/
	float weightz;      /*  weighting function calculated at line z **/

	double ee;          /*  the exponential in the mfap fcn   **/
	int i, j, l;        /*  index variables        **/
	int N_left, N_right;/*  actual buffer size for wfunctions left & right **/
	int Limit;				/*  upper limit for the for loop **/



	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);
	convLeft = sGetConvLeftBuffer(MUPLength);
	sGetFFTLeftBuffers(MUPLength, &fft_left, &weightfn_left);


	if (electrodeType == 2)
	{
		semimajor = 290;                /* major axis in microns */
		semiminor = 75;                 /* minor axis in microns */
	}
	if (electrodeType == 3)
	{
		semimajor = 100;                /* major axis in microns */
		semiminor = 100;                /* minor axis in microns */
	}


		/*  Scale factor in weight fn eqtn. */
	A = scale_factor / (4.0 * M_PI * sigmar) / sqrt(kratio);

		/*  Scale factor in current eqtn. */
	currentfn_const = currentConstant(diameterInMM);


	/*
	 * start zellipse one half "step" in from edge of
	 * ellipse (or circle); 6 steps will take us to one half step
	 * from other edge
	 */
	zellipse = (float) (-semiminor / 3.0 * 2.5);


		/*  Calculate z step size  in mm **/
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);

		/* adjust to integral number of z steps */
	zEndplateDistanceInMM = (float)
		        (floor(zEndplateDistanceInMM / z_inc) * z_inc);

	SignalDurationInTime = 5.0; /* In ms */
	N_left = (int) floor( EndPlateLocationInMM / z_inc);
	N_right = (int) floor(FiberLengthInMM / z_inc) - N_left;

	NI = (int) floor(SignalDurationInTime / DELTA_T_MUP);


	weightz = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;
	convLeft[0] = 0.0;

	memset(weightfn, 0, sizeof(double) * MUPLength * 2);
	memset(weightfn_left, 0, sizeof(double) * MUPLength * 2);




	/* calculate 6 line integrals */
	for (l = 0; l < 6; l++)
	{

		/* calulate limits of integration */
		xellipse =
		    (float) (sqrt
		             (SQR(semimajor) -
		                    SQR(semimajor) * SQR(zellipse)
		                            / SQR(semiminor)));

		    /* one   ellipse relative x location in microns */
		x_posEnd = xellipse;
		    /* other ellipse relative x location in microns */
		x_negEnd = xellipse * (-1);


		    /* one   x distance from fibre to electrode in mm */
		delta_x_posEnd = (float)
		        ((x_posEnd / 1000.0) - muscleFibreXLocationInMM);
		    /* other x distance from fibre to electrode in mm */
		delta_x_negEnd = (float)
		        ((x_negEnd / 1000.0) - muscleFibreXLocationInMM);


		for (i = 0; i<2 ; i++)
		{
			Limit = i * N_left + (1-i) * N_right;

			z=0.0;

			for (j = 1; j <= Limit; j++)
			{


					B = SQR(deltay)
							+ SQR(zEndplateDistanceInMM + (2*i - 1) * z
												- (zellipse / 1000.0))
										/ kratio;


				if (delta_x_negEnd > 0)
				{
					/*
					 * the fibre is on the negative co-ordinate side
					 * of the electrode
					 */
					weightz = (float) (log(
							fabs((delta_x_posEnd
									+ sqrt(SQR(delta_x_posEnd) + B)))

								/ (delta_x_negEnd
									+ sqrt(SQR(delta_x_negEnd) + B))
							));


				} else if (delta_x_posEnd < 0)
				{
					/*
					 * the fibre is on the positive co-ordinate
					 * side of the electrode
					 */
					weightz = (float) (log(
							fabs((-delta_x_negEnd
									+ sqrt(SQR(delta_x_negEnd) + B)))
								/ (-delta_x_posEnd
									+ sqrt(SQR(delta_x_posEnd) + B))
							));

				} else
				{
					/*
					 * the fibre is under the electrode,
					 * do the calc in two parts
					 */
					weightz = (float) (
							log(
								fabs((-delta_x_negEnd
										+ sqrt(SQR(delta_x_negEnd) + B)))
									/ sqrt(B)
									)
							+ log(
								fabs((delta_x_posEnd
										+ sqrt(SQR(delta_x_posEnd) + B)))
									/ sqrt(B)
									)
								);
				}

				// divide out the length of the line electrode
				if (i==0)
					weightfn[j] += (double)
							(fabs(weightz /
									(fabs(x_posEnd - x_negEnd) / 1000.)));
				if (i==1)
					weightfn_left[j] += (double)
							(fabs(weightz /
									(fabs(x_posEnd - x_negEnd) / 1000.)));

				z += z_inc;
			}
		}

		    /* increment to next line integral */
		zellipse = (float) (zellipse + semiminor / 3.0);
		weightz = 0.0;
	}

	/*plot_mah(weightfn,N_right);

	plot_mah(weightfn_left,N_left);*/


	z = 0.0;
	for (j = 1; j <= MAX(N_right,N_left); j++)
	{

		if (j<=NI)
		{
		/*  exponential term  **/
		ee = exp(-2.0 * z);

		if (j==CurrCorr)
				current[j] = optZinit(z_inc,currentfn_const,MUPLength);
		else
			current[j] = (double)
					(-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) * ee);

		z += z_inc;
		}

		/* average weighting fn **/
		weightfn[j] = (double) (A * weightfn[j] / 6.);
		weightfn_left[j] = (double) (A * weightfn_left[j] / 6.);
	}

	/*plot_mah(weightfn,N_right+1);

	plot_mah(weightfn_left,N_left+1);*/


	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    z_inc
		);

	convolve(fft_left,
			weightfn_left, MUPLength * 2,
			current, MUPLength * 2,
			1,
			convLeft,
			z_inc
		 );


	/*plot_mah(convLeft,MUPLength*2);
	plot_mah(convolution,MUPLength*2);*/

	adjustConvArtifact(
				NI, N_right+N_left,
				N_right,
				N_left,
			    z_inc,
				/*diameterInMM,*/
				convLeft, convolution,
				current,
				weightfn_left, weightfn
				);

   /*plot_mah(convolution,MUPLength*2);*/

   /* transforming from space domain into time domain */
   /* in convolve function, z_inc should be replaced by DELTA_T_MUP */
	for (j = 1; j <= MUPLength*2; j++)
	   convolution[j]=convolution[j]/conductionVelocity_MMperMS;

   /*plot_mah(convolution,MUPLength*2);*/




	return 1;
}



/*
 * ----------------------------------------------------------------
 * This procedure calculates the muscle fibre action potential
 * seen by the cannula.  Based strongly on the concentric needle
 * logic above.
 */
static int
calculateCannulaMFAP(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float fibreLocXInMM,
		float fibreLocYInMM,
		NeedleInfo *needle,
		float fibreDiameterInMM,
		float conductionVelocity_MMperMS
	)
{
		/*Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/*Radial conductivity mhos/mm */
	const double sigmar = 0.000063;
		/*Ratio of Sigmar to sigmaz* */
	const double kratio = sigmaz / sigmar;
		/*Semi major axis of ellipse(circle) */


	double *fft;
	double *weightfn;
	double *current;
	float adjust;

	double B;
		/*  the fractional multiplier - weightfn   **/
	double A;
		/*  current vector constant  **/
	double currentfn_const;

	double distanceToShaftInMM;
	double xProj_deltaToTipInMM;
	double xProj_deltaToEndOfShaftInMM;

	float z_inc;        /*  increment along the z axis        **/
	float z;            /*  position along the z axis    **/
	float weightz;      /*  weighting function calculated at line z **/

	double ee;          /* the exponential in the mfap fcn **/
	int i;           /* index variables **/



	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);


		/*  Scale factor in weight fn eqtn. */
	A = (1.0 / (4.0 * M_PI * sigmar)) / sqrt(kratio);

		/*
		 * Scale factor in current eqtn.
		 * From Nandedkar '83
		 *
		 * Im(z) = [ (sigma_i pi d^2) / 4 ] * e''(z)
		 *
		 * e(z) = g(2z)
		 *      = 96 (2z)^3 e^[-2z] - 90
		 *                 Q(z) = 96 (2z)^3
		 *      = Q(z) e^[-2z] - 90
		 *
		 * e'(z) = [ 3 * 96 (2z)^2 ]   *   2 (z^0)
			 */
	currentfn_const = currentConstant(fibreDiameterInMM);

		/*  Calculate z step size  in mm **/
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);

		/* adjust to integral number of z steps */
	zEndplateDistanceInMM = (float)
		        (floor(zEndplateDistanceInMM / z_inc) * z_inc);


	weightz = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;
	memset(weightfn, 0, sizeof(double) * MUPLength * 2);


	/**
	 * get distances from fibre to ends of cannula
	 */
	needle->getProjectedNeedleDistances(
		        &xProj_deltaToTipInMM,
		            &xProj_deltaToEndOfShaftInMM,
		            &distanceToShaftInMM,
		        fibreLocXInMM,
		            fibreLocYInMM
		    );

	//if (distanceToShaftInMM < fibreDiameterInMM / 2.0)
		//distanceToShaftInMM = fibreDiameterInMM / 2.0;


	/*
	 * calculate a line integral
	 *
	 * X_0 = fibre
	 * X_1, X_2 = ends of line
	 *
	 * Q = frac{x_2 - x_0 + \sqrt{ (x_2 - x_0)^2 +B } }
	 * 		{x_1 - x_0 + \sqrt{ (x_1 - x_0)^2 + B}}
	 *
	 * weightfn = frac{A * ln(Q)}
	 * 				{x_2 - x_1}
	 * 				## divide out the length of the electrode
	 * 				## to obtain the average value
	 *
	 * True if x_0 < x_1 < x_2
	 * 				## valid outside the bounds of the electrode
	 *
	 *
	 *
	 * Q' = frac{x_1 - x_0 + \sqrt{ (x_1 - x_0)^2 + B}}
	 *		{x_2 - x_0 + \sqrt{ (x_2 - x_0)^2 +B } }
	 *
	 * weightfn = frac{A * ln(1/Q)}
	 * 						{x_2 - x_1}
	 * True if x_0 < x_2 < x_1
	 *
	 * -----------------
	 *
	 * B = (y-y_0)^2 + frac{(z-z_0)^2}{k}
	 *
	 * k = anisotrophy ratio = frac{\sigma_z}{sigma_r}
	 *
	 * A = 1/(4 \pi \sigma_r \sqrt{k})
	 *
	 * \sigma_r = radial conductivity
	 *
	 */

	z = 0.0;
	for (i = 1; i <= MUPLength * 2; i++)
	{

		B = SQR(distanceToShaftInMM)
		        + SQR(zEndplateDistanceInMM - z) / kratio;

		if (xProj_deltaToTipInMM > 0)
		{
		    /*
		     * the fibre is out past the tip
		     */
		    weightz = (float) (log(
		            fabs((xProj_deltaToEndOfShaftInMM
		                + sqrt(SQR(xProj_deltaToEndOfShaftInMM) + B)))
		            / (xProj_deltaToTipInMM
		                        + sqrt(SQR(xProj_deltaToTipInMM) + B))
		            ));

		} else if (xProj_deltaToEndOfShaftInMM < 0)
		{
		    /*
		     * the fibre is out past the end of the cannula
		     * (not likely, except in model testing)
		     */
		    weightz = (float) (log(
		            fabs((-xProj_deltaToTipInMM
		                        + sqrt(SQR(xProj_deltaToTipInMM) + B)))
		                / (-xProj_deltaToEndOfShaftInMM
		                    + sqrt(SQR(xProj_deltaToEndOfShaftInMM)
		                        + B))
		            ));

		} else
		{
		    /*
		     * the fibre is "under" the cannula, do the calc in
		     * two parts
		     */
		    weightz = (float) (
		            log(
		                fabs((-xProj_deltaToTipInMM
		                        + sqrt(SQR(xProj_deltaToTipInMM) + B)))
		                    / sqrt(B)
		                    )
		            + log(
		                fabs((xProj_deltaToEndOfShaftInMM
		                        + sqrt(SQR(xProj_deltaToEndOfShaftInMM)
		                            + B)))
		                    / sqrt(B)
		                    )
		                );
		}

		weightfn[i] = (double)
		        (fabs(weightz /
		                fabs(needle->getCannulaLengthInMM())
		            ));

		z += z_inc;
	}

	z = 0.0;
	for (i = 1; i <= MUPLength * 2; i++)
	{

		/*  exponential term  **/
		ee = exp(-2.0 * z);

		current[i] = (double)
		        (-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) * ee);

		z += z_inc;

		/* scaled weighting fn **/
		weightfn[i] = (double) (A * weightfn[i]);
	}



	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    DELTA_T_MUP
		);


	/*
	 * ramp the convolution artifact down to zero
	 * within the first 50 samples
	 */
	adjust = (float) (convolution[50] / 49.0);
	for (i = 49; i > 0; i--)
	{
		convolution[i] = adjust * i;
	}

	/* ramp the convolution down to zero within the last 50 samples */
	adjust = (float) (convolution[MUPLength * 2 - 50] / 49.0);
	for (i = 49; i >= 0; i--)
	{
		convolution[MUPLength * 2 - i] = adjust * i;
	}

	return 1;
}



/*
 * ----------------------------------------------------------------
 * This procedure calculates the muscle fibre action potential
 * seen by the cannula.  Based strongly on the concentric needle
 * logic above.
 */
static int
calculateCannulaMFAPWithInitiation(
		float xLoc, float yLoc,
		int fibreIndex,
		int MUPLength,
		double *convolution,
		float zEndplateDistanceInMM,
		float fibreLocXInMM,
		float fibreLocYInMM,
		NeedleInfo *needle,
		float fibreDiameterInMM,
		float conductionVelocity_MMperMS,

		float FiberLengthInMM,
		float EndPlateLocationInMM
	)
{
		/*Axial conductivity  mhos/mm */
	const double sigmaz = 0.00033;
		/*Radial conductivity mhos/mm */
	const double sigmar = 0.000063;
		/*Ratio of Sigmar to sigmaz* */
	const double kratio = sigmaz / sigmar;
		/*Semi major axis of ellipse(circle) */


	double *fft;
	double *weightfn;
	double *current;


	double *convLeft;
	double *fft_left;
	double *weightfn_left;

	double B;
		/*  the fractional multiplier - weightfn   **/
	double A;
		/*  current vector constant  **/
	double currentfn_const;

	double distanceToShaftInMM;
	double xProj_deltaToTipInMM;
	double xProj_deltaToEndOfShaftInMM;

	float z_inc;        /*  increment along the z axis        **/
	float z;            /*  position along the z axis    **/
	float weightz;      /*  weighting function calculated at line z **/

	double ee;          /* the exponential in the mfap fcn **/
	int i,j;           /* index variables **/
	int NI, N_right, N_left, Limit;
	double SignalDurationInMS;



	sGetFFTBuffers(MUPLength, &fft, &weightfn, &current);
	convLeft = sGetConvLeftBuffer(MUPLength);
	sGetFFTLeftBuffers(MUPLength, &fft_left, &weightfn_left);


		/*  Scale factor in weight fn eqtn. */
	A = (scale_factor / (4.0 * M_PI * sigmar)) / sqrt(kratio);

		/*
		 * Scale factor in current eqtn.
		 * From Nandedkar '83
		 *
		 * Im(z) = [ (sigma_i pi d^2) / 4 ] * e''(z)
		 *
		 * e(z) = g(2z)
		 *      = 96 (2z)^3 e^[-2z] - 90
		 *                 Q(z) = 96 (2z)^3
		 *      = Q(z) e^[-2z] - 90
		 *
		 * e'(z) = [ 3 * 96 (2z)^2 ]   *   2 (z^0)
			 */
	currentfn_const = currentConstant(fibreDiameterInMM);

		/*  Calculate z step size  in mm **/
	z_inc = (float) (conductionVelocity_MMperMS * DELTA_T_MUP);

		/* adjust to integral number of z steps */
	zEndplateDistanceInMM = (float)
		        (floor(zEndplateDistanceInMM / z_inc) * z_inc);

	SignalDurationInMS = 5.0; /* In ms */
	N_left = (int) floor( EndPlateLocationInMM / z_inc);
	N_right = (int) floor(FiberLengthInMM / z_inc) - N_left;

	NI = (int) floor(SignalDurationInMS / DELTA_T_MUP);


	weightz = 0.0;
	current[0] = 0.0;
	convolution[0] = 0.0;
	convLeft[0] = 0.0;

	memset(weightfn, 0, sizeof(double) * MUPLength * 2);
	memset(weightfn_left, 0, sizeof(double) * MUPLength * 2);


	/**
	 * get distances from fibre to ends of cannula
	 */
	needle->getProjectedNeedleDistances(
		        &xProj_deltaToTipInMM,
		            &xProj_deltaToEndOfShaftInMM,
		            &distanceToShaftInMM,
		        fibreLocXInMM,
		            fibreLocYInMM
		    );

	//if (distanceToShaftInMM < fibreDiameterInMM / 2.0)
		//distanceToShaftInMM = fibreDiameterInMM / 2.0;

	/*
	 * calculate a line integral
	 *
	 * X_0 = fibre
	 * X_1, X_2 = ends of line
	 *
	 * Q = frac{x_2 - x_0 + \sqrt{ (x_2 - x_0)^2 +B } }
	 * 		{x_1 - x_0 + \sqrt{ (x_1 - x_0)^2 + B}}
	 *
	 * weightfn = frac{A * ln(Q)}
	 * 				{x_2 - x_1}
	 * 				## divide out the length of the electrode
	 * 				## to obtain the average value
	 *
	 * True if x_0 < x_1 < x_2
	 * 				## valid outside the bounds of the electrode
	 *
	 *
	 *
	 * Q' = frac{x_1 - x_0 + \sqrt{ (x_1 - x_0)^2 + B}}
	 *		{x_2 - x_0 + \sqrt{ (x_2 - x_0)^2 +B } }
	 *
	 * weightfn = frac{A * ln(1/Q)}
	 * 						{x_2 - x_1}
	 * True if x_0 < x_2 < x_1
	 *
	 * -----------------
	 *
	 * B = (y-y_0)^2 + frac{(z-z_0)^2}{k}
	 *
	 * k = anisotrophy ratio = frac{\sigma_z}{sigma_r}
	 *
	 * A = 1/(4 \pi \sigma_r \sqrt{k})
	 *
	 * \sigma_r = radial conductivity
	 *
	 */


	for (j=0 ; j <2 ; j++)
	{

		Limit = j * N_left + (1-j) * N_right ;

		z = 0.0;

		for (i = 1; i <= Limit; i++)
		{


			B = SQR(distanceToShaftInMM)
					+ SQR(zEndplateDistanceInMM + (2*j - 1) * z) / kratio;

			if (xProj_deltaToTipInMM > 0)
			{
				/*
				 * the fibre is out past the tip
				 */
				weightz = (float) (log(
						fabs((xProj_deltaToEndOfShaftInMM
							+ sqrt(SQR(xProj_deltaToEndOfShaftInMM) + B)))
						/ (xProj_deltaToTipInMM
									+ sqrt(SQR(xProj_deltaToTipInMM) + B))
						));

			} else if (xProj_deltaToEndOfShaftInMM < 0)
			{
				/*
				 * the fibre is out past the end of the cannula
				 * (not likely, except in model testing)
				 */
				weightz = (float) (log(
						fabs((-xProj_deltaToTipInMM
									+ sqrt(SQR(xProj_deltaToTipInMM) + B)))
							/ (-xProj_deltaToEndOfShaftInMM
								+ sqrt(SQR(xProj_deltaToEndOfShaftInMM)
									+ B))
						));

			} else
			{
				/*
				 * the fibre is "under" the cannula, do the calc in
				 * two parts
				 */
				weightz = (float) (
						log(
							fabs((-xProj_deltaToTipInMM
									+ sqrt(SQR(xProj_deltaToTipInMM) + B)))
								/ sqrt(B)
								)
						+ log(
							fabs((xProj_deltaToEndOfShaftInMM
									+ sqrt(SQR(xProj_deltaToEndOfShaftInMM)
										+ B)))
								/ sqrt(B)
								)
							);
			}

			if (j==0)
				weightfn[i] = (double)
					(fabs(weightz /
							fabs(needle->getCannulaLengthInMM())
						));
			if (j==1)
				weightfn_left[i] = (double)
					(fabs(weightz /
							fabs(needle->getCannulaLengthInMM())
						));

			z += z_inc;
		}
	}

	z = 0.0;

	for (i = 1; i <= MAX(N_right,N_left); i++)
	{

		/*  exponential term  **/
		ee = exp(-2.0 * z);

		if(i<=NI)
		{
			if (i==CurrCorr)
				current[i] = optZinit(z_inc,currentfn_const,MUPLength);
			else
				current[i] = (double)
					(-currentfn_const * z * (1.5 - 3.0 * z + SQR(z)) * ee);
		}

		z += z_inc;

		/* scaled weighting fn **/
		weightfn[i] = (double) (A * weightfn[i]);
		weightfn_left[i] = (double) (A * weightfn_left[i]);
	}

	/*plot_mah(weightfn,N_right);
	plot_mah(weightfn_left,N_left+10);*/



	convolve(fft,
		    weightfn, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convolution,
		    z_inc
		);

	/*plot_mah(convolution, MUPLength * 2 );*/

	convolve(fft_left,
		    weightfn_left, MUPLength * 2,
		    current, MUPLength * 2,
		    1,
		    convLeft,
		    z_inc
		);
	/*plot_mah(convLeft,MUPLength*4);
	plot_mah(current,NI);*/


   adjustConvArtifact(
				NI, N_left+N_right,
				N_right,
				N_left,
			    z_inc,
				/*fibreDiameterInMM,*/
				convLeft, convolution,
				current,
				weightfn_left, weightfn
				);

   /*plot_mah(convolution,MUPLength*2);*/
   /* transforming from space domain into time domain */
   /* in convolve function, z_inc should be replaced by DELTA_T_MUP */
   for (j = 1; j <= MUPLength*2; j++)
	   convolution[j]=convolution[j]/conductionVelocity_MMperMS;

   /*plot_mah(convolution,MUPLength*2);*/

   return 1;
}



static int sConvolutionBufferMUPLength = 0;
static double *sConvolutionBuffer = NULL;


static void sCleanConvolutionBuffer()
{
	if (sConvolutionBuffer != NULL)
	{
		ckfree(sConvolutionBuffer);
		sConvolutionBuffer = NULL;
	}
}

static double *sGetConvolutionBuffer(int MUPLength)
{
	if (sConvolutionBuffer == NULL
		        || sConvolutionBufferMUPLength < MUPLength)
				{

		sCleanConvolutionBuffer();

		sConvolutionBuffer =
		    (double *) ckalloc((MUPLength * 4 + 1) * (sizeof(double)));
		sConvolutionBufferMUPLength = MUPLength;

		MSG_ASSERT(sConvolutionBuffer != NULL,
		        "Failed allocating convolution buffer");
	}
	memset(sConvolutionBuffer, 0,
		        (MUPLength * 4 + 1) * (sizeof(double)));

	return sConvolutionBuffer;
}

/* New part*/
static int sConvLeftBufferMUPLength = 0;
static double *sConvLeftBuffer = NULL;


static void sCleanConvLeftBuffer()
{
	if (sConvLeftBuffer != NULL)
	{
		ckfree(sConvLeftBuffer);
		sConvLeftBuffer = NULL;
	}
}

static double *sGetConvLeftBuffer(int MUPLength)
{
	if (sConvLeftBuffer == NULL
		        || sConvLeftBufferMUPLength < MUPLength)
				{

		sCleanConvLeftBuffer();

		sConvLeftBuffer =
		    (double *) ckalloc((MUPLength * 4 + 1) * (sizeof(double)));
		sConvLeftBufferMUPLength = MUPLength;

		MSG_ASSERT(sConvLeftBuffer != NULL,
		        "Failed allocating convLeft buffer");
	}
	memset(sConvLeftBuffer, 0,
		        (MUPLength * 4 + 1) * (sizeof(double)));

	return sConvLeftBuffer;
}

/* */


/* Bipolar added Buffers */

#ifdef  DEAD_CODE
//static int sConvRightBufferMUPLength = 0;
static double *sConvRightBuffer = NULL;


static void sCleanConvRightBuffer()
{
	if (sConvRightBuffer != NULL)
	{
		ckfree(sConvRightBuffer);
		sConvRightBuffer = NULL;
	}
}

static double *sGetConvRightBuffer(int MUPLength)
{
	if (sConvRightBuffer == NULL
		        || sConvRightBufferMUPLength < MUPLength)
	{

		sCleanConvRightBuffer();

		sConvRightBuffer =
		    (double *) ckalloc((MUPLength * 4 + 1) * (sizeof(double)));
		sConvRightBufferMUPLength = MUPLength;

		MSG_ASSERT(sConvRightBuffer != NULL,
		        "Failed allocating convLeft buffer");
	}
	memset(sConvRightBuffer, 0,
		        (MUPLength * 4 + 1) * (sizeof(double)));

	return sConvRightBuffer;
}
#endif

/* Bipolar added buffers end */

static int sFFTBufferMUPLength = 0;
static double *sFFTBuffer = NULL;
static double *sWeightBuffer = NULL;
static double *sCurrentBuffer = NULL;

static void sCleanFFTBuffers()
{
	if (sFFTBuffer != NULL)
	{
		ckfree(sFFTBuffer);
		sFFTBuffer = NULL;
	}
	if (sWeightBuffer != NULL)
	{
		ckfree(sWeightBuffer);
		sWeightBuffer = NULL;
	}
	if (sCurrentBuffer != NULL)
	{
		ckfree(sCurrentBuffer);
		sCurrentBuffer = NULL;
	}
}

static void sGetFFTBuffers(
		    int MUPLength,
		    double **fftBuffer,
		    double **weightBuffer,
		    double **currentBuffer
		)
{
	if (sFFTBufferMUPLength < MUPLength)
	{

		sCleanFFTBuffers();


		sFFTBuffer = (double *)
		        ckalloc((MUPLength * 4 + 1) * (sizeof(double)));
		MSG_ASSERT(sFFTBuffer != NULL,
		        "Failed allocating FFT buffer");

		sWeightBuffer = (double *)
		        ckalloc((MUPLength * 2 + 1) * (sizeof(double)));
		MSG_ASSERT(sWeightBuffer != NULL,
		        "Failed allocating weight buffer");

		sCurrentBuffer = (double *)
		        ckalloc((MUPLength * 2 + 1) * (sizeof(double)));
		MSG_ASSERT(sCurrentBuffer != NULL,
		        "Failed allocating current buffer");
	}

	memset(sFFTBuffer, 0,
		        (MUPLength * 4 + 1) * (sizeof(double)));

	memset(sWeightBuffer, 0,
		        (MUPLength * 2 + 1) * (sizeof(double)));

	memset(sCurrentBuffer, 0,
		        (MUPLength * 2 + 1) * (sizeof(double)));

	*fftBuffer          = sFFTBuffer;
	*weightBuffer       = sWeightBuffer;
	*currentBuffer      = sCurrentBuffer;
}


static void sCleanBuffers()
{
	sCleanConvolutionBuffer();
	sCleanFFTBuffers();
}

/* new part*/

static int sFFTLeftBufferMUPLength = 0;
static double *sFFTLeftBuffer = NULL;
static double *sWeightLeftBuffer = NULL;


static void sCleanFFTLeftBuffers()
{
	if (sFFTLeftBuffer != NULL)
	{
		ckfree(sFFTLeftBuffer);
		sFFTLeftBuffer = NULL;
	}
	if (sWeightLeftBuffer != NULL)
	{
		ckfree(sWeightLeftBuffer);
		sWeightLeftBuffer = NULL;
	}

}


static void sGetFFTLeftBuffers(int MUPLength,
		double **fftLeftBuffer,
		double **weightLeftBuffer
	)
{
	if (sFFTLeftBufferMUPLength < MUPLength)
	{

		sCleanFFTLeftBuffers();


		sFFTLeftBuffer = (double *)
				ckalloc((MUPLength * 4 + 1) * (sizeof(double)));
		MSG_ASSERT(sFFTLeftBuffer != NULL,
				"Failed allocating Left FFT buffer");

		sWeightLeftBuffer = (double *)
				ckalloc((MUPLength * 2 + 1) * (sizeof(double)));
		MSG_ASSERT(sWeightLeftBuffer != NULL,
				"Failed allocating Left weight buffer");

	}

	memset(sFFTLeftBuffer, 0,
			(MUPLength * 4 + 1) * (sizeof(double)));

	memset(sWeightLeftBuffer, 0,
			(MUPLength * 2 + 1) * (sizeof(double)));

	*fftLeftBuffer          = sFFTLeftBuffer;
	*weightLeftBuffer       = sWeightLeftBuffer;

}

static void sCleanLeftBuffers()
{
	sCleanConvLeftBuffer();
	sCleanFFTLeftBuffers();
}

/*
 * Load the EMG values from a saved set of files
 */
int loadMUPs(
		int **MUPIdList,
		int totalNumberOfMUPs
	)
{
	int i;

	*MUPIdList = (int *) ckalloc(sizeof(int) * totalNumberOfMUPs);
	for (i = 0; i < totalNumberOfMUPs; i++)
	{
		(*MUPIdList)[i] = (-1);
	}

	/* Fill in the list with valid IDs */
	if ( ! locateValidMUPs(
						(*MUPIdList),
						totalNumberOfMUPs,
						g->firings_dir,
						g->MUPs_dir))
	{
		return 0;
	}

	return 1;
}

