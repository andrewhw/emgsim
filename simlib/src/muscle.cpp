/**
 * Lay out muscle motor unit territories, and assign fibres
 * to them.
 *
 * A detection area is selected at the centre of the muscle and
 * filled with fibres.  Each fibre is assigned to a motor unit
 * (MU), on the condition that the fibres location falls within
 * the motor units territory.  The diameter of each fibre is
 * normally distributed (mean = 55um, range = 25 to 85 um),
 * while the neuromuscular junction location is also normally
 * distributed with 2 standard deviations equal to 2.5mm ie
 * randomly shifted -2.5 to 2.5 mm.
 *
 * $Id: muscle.cpp 28 2020-09-25 20:00:58Z andrew $
 */

#include "os_defs.h"

#ifndef MAKEDEPEND
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <string.h>
# include <math.h>
# include <errno.h>
# ifndef OS_WINDOWS_NT
#  if !defined(OS_BSD) && !defined(OS_DARWIN)
#   include <values.h>
#  endif
# endif
#endif

#define PRIVATE public
#include "Simulator.h"
#include "MuscleData.h"
#include "NeedleInfo.h"

#include "SimulatorControl.h"
#include "SimulatorConstants.h"

#include "muscle.h"

#include "rTreeIndex.h"

#include "tclCkalloc.h"
#include "listalloc.h"
#include "bitstring.h"
#include "logwrite.h"
#include "random.h"
#include "pathtools.h"
#include "plottools.h"
#include "stringtools.h"
#include "mathtools.h"
#include "reporttimer.h"
#include "error.h"
#include "massert.h"
#include "log.h"

#include "3Circle.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif



//#define DEBUG_LOG_MUSCLE
//#define DEBUG_LAYOUT
//#define DEBUG_MAHDIEH_NEEDLE_INFO

MuscleParameters *getMuscleParameters(
		int baseNumberOfMotorUnits,
		float neuropathicInvolvement,
		float myopathicInvolvement
	)
{
	/**
	 * return handle to static (singleton) control structure
	 */
	static MuscleParameters sParams;

	sParams.numMotorUnits = baseNumberOfMotorUnits;

	return &sParams;
}

MuscleParameters::MuscleParameters()
{
	numFibreLayoutProbabilityFunctions = 0;
	fibreLayoutProbabilityFunctionList = NULL;
	fibreLayoutProbabilityFunctionProbabilities = NULL;

	numFibreLayoutWeightingFunctions = 0;
	fibreLayoutWeightingFunctionList = NULL;
	fibreDenominatorWeightingFunctionList = NULL;
	fibreLayoutWeightingFunctionWeights = NULL;

	fibreLayoutWeightingNoiseFactor = 0;
}

MuscleParameters::~MuscleParameters()
{
	if (fibreLayoutProbabilityFunctionList != NULL)
	{
		ckfree(fibreLayoutProbabilityFunctionList);
		ckfree(fibreLayoutProbabilityFunctionProbabilities);
	}
	if (fibreLayoutWeightingFunctionList != NULL)
	{
		ckfree(fibreLayoutWeightingFunctionList);
		ckfree(fibreDenominatorWeightingFunctionList);
		ckfree(fibreLayoutWeightingFunctionWeights);
	}
}

void
MuscleParameters::setMuscleParamDefaultValues(int newNumMotorUnits)
{
	int i;

	numMotorUnits = newNumMotorUnits;

	fibreDensity = 10.0;
	areaPerFibre = 0.0025f;
	minMUDiam = 2.0f;
	maxMUDiam = 8.0f;
//    rangeOfTwitchTensions = 16.0f;
	modelLayoutDistanceInUM = (-1);
	fibreLayoutWeightingNoiseFactor = 0.03f;


	if (fibreLayoutWeightingFunctionList != NULL)
	{
		ckfree(fibreLayoutWeightingFunctionList);
		ckfree(fibreDenominatorWeightingFunctionList);
		ckfree(fibreLayoutWeightingFunctionWeights);
	}
	numFibreLayoutWeightingFunctions = getNumFibreLayoutWeightingFunctions();
	fibreLayoutWeightingFunctionList = (MFL_WeightingFunction **)
				ckalloc(sizeof(MFL_WeightingFunction *)
									* numFibreLayoutWeightingFunctions);
	fibreDenominatorWeightingFunctionList =
				(MFL_WeightingDenominatorFunction **)
				ckalloc(sizeof(MFL_WeightingDenominatorFunction *)
									* numFibreLayoutWeightingFunctions);
	fibreLayoutWeightingFunctionWeights = (float *)
				ckalloc(sizeof(float *) * numFibreLayoutWeightingFunctions);
	for (i = 0; i < numFibreLayoutWeightingFunctions; i++)
	{
		fibreLayoutWeightingFunctionList[i]
					= getFibreLayoutWeightingFunction(i);
		fibreDenominatorWeightingFunctionList[i]
					= getFibreLayoutWeightingDenominatorFunction(i);
		fibreLayoutWeightingFunctionWeights[i]
					= getFibreLayoutWeightingFunctionWeighting(i);
	}


	if (fibreLayoutProbabilityFunctionList != NULL)
	{
		ckfree(fibreLayoutProbabilityFunctionList);
		ckfree(fibreLayoutProbabilityFunctionProbabilities);
	}
	numFibreLayoutProbabilityFunctions
				= getNumFibreLayoutProbabilityFunctions();
	fibreLayoutProbabilityFunctionList
				= (MFL_ProbabilityFunction **)
				ckalloc(sizeof(MFL_ProbabilityFunction *)
									* numFibreLayoutProbabilityFunctions);
	fibreLayoutProbabilityFunctionProbabilities = (float *)
				ckalloc(sizeof(float *) * numFibreLayoutProbabilityFunctions);
	for (i = 0; i < numFibreLayoutProbabilityFunctions; i++)
	{
		fibreLayoutProbabilityFunctionList[i]
					= getFibreLayoutProbabilityFunction(i);
		fibreLayoutProbabilityFunctionProbabilities[i]
					= getFibreLayoutProbabilityFunctionProbability(i);
	}

}


int
countMotorUnitsIntersectingCentre(MuscleData *MD)
{
	int nMotorUnitsIntersectingCentre;
	int i;

	/**
	 ** this section checks the number of motor unit territories
	 ** that include the muscle centre in their territory areas
	 **/
	nMotorUnitsIntersectingCentre = 0;
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		if (MD->motorUnit_[i]->mu_loc_r_mm_ <=
		                (MD->motorUnit_[i]->mu_expected_diameter_mm_ / 2.0))
		    nMotorUnitsIntersectingCentre++;
	}

	return nMotorUnitsIntersectingCentre;
}


#define		NUM_MOTOR_UNITS_SAMPLED		10000

#ifdef  DEBUG_LAYOUT
static int
plotFibreDensitiesByMU(
		MuscleData *MD,
		const char *muscleDirectory
//		,
//		float sampleRadiusInMM
	)
{
	struct rTreeResultList rtreeResults;
	Rect searchRect;
	MuscleFibre *currentFibre;
	MotorUnit *curMotorUnit;
	float xThrow, yThrow;
	float throwRadiusInMM;
//    float muscleRadius_mah;
	int numFibresInBoundingBox;
	int *countsByMotorUnitIndex = NULL;
	float *areaByMotorUnitIndex = NULL;
	int fibreMUId;
	int numHits;
	int sanityCheck = 0;
	float muFibreDensitySamples[NUM_MOTOR_UNITS_SAMPLED];
	double distance, xDiff, yDiff;
	double variance, mean;
	double max, oldMax;
	float compare;
	int status = 0;
	int i;
//    muscleRadius_m = MD->getMuscleDiameter()/2.0;

	float sampleRadiusInMM = (float) 2.0;

	memset(&rtreeResults, 0, sizeof(rtreeResults));

	if ( ! buildFibreRTree(MD) )
		return 0;

	/** allocate the list of counts, and set all to zero */
	countsByMotorUnitIndex = (int *)
				ckalloc(sizeof(int) * (MD->getNumMotorUnits()));
	areaByMotorUnitIndex = (float *)
				ckalloc(sizeof(float) * (MD->getNumMotorUnits()));


	throwRadiusInMM = (float) ((MD->muscleDiameter_ / 2.0) - 0.5);

	numHits = 0;
	while ((numHits < NUM_MOTOR_UNITS_SAMPLED) && (sanityCheck < 10000))
	{

		sanityCheck++;

		LogInfo(" - numHits %d (< %d), sanityCheck %d (< %d)\n",
				numHits, NUM_MOTOR_UNITS_SAMPLED, sanityCheck, 10000);

		xThrow = floatSignedRangeRandom(throwRadiusInMM);
		yThrow = floatSignedRangeRandom(throwRadiusInMM);

		/**
		 * We "throw" a (sampleRadiusInMM) circle; we use
		 * the r-tree to prune out all fibres not in the
		 * bounding box of the circle
		 *
		 */
		searchRect.boundary[0]
				= (float) (xThrow - sampleRadiusInMM) * CELLS_PER_MM;
		searchRect.boundary[1]
				= (float) (yThrow - sampleRadiusInMM) * CELLS_PER_MM;
		searchRect.boundary[2]
				= (float) (xThrow + sampleRadiusInMM) * CELLS_PER_MM;
		searchRect.boundary[3]
				= (float) (yThrow + sampleRadiusInMM) * CELLS_PER_MM;


		/** use the R-Tree to get list of possible Fibres's */
		memset(&rtreeResults, 0, sizeof(rtreeResults));
		numFibresInBoundingBox = RTreeSearch(
				MD->fibreRTreeRoot_,
				&searchRect,
				FibreRTreeSearchCallback__,
				(void *) &rtreeResults
			);


		/** clear counts */
		memset(countsByMotorUnitIndex, 0,
				sizeof(int) * (MD->getNumMotorUnits()));
		for (i = 0; i < MD->getNumMotorUnits(); i++)
		{
			areaByMotorUnitIndex[i] = (-1);
		}


		/**
		 * For each fibre, check if it is indeed within our
		 * radius.  If so, add the motor unit from the fibre
		 * to our list,
		 */
		for (i = 0; i < numFibresInBoundingBox; i++)
		{

			currentFibre = MD->getFibre(rtreeResults.results_[i]);
			if (currentFibre == NULL)
				continue;

			/** skip fibre which are too far away */
			xDiff = xThrow - currentFibre->getXLocationInMM();
			yDiff = yThrow - currentFibre->getYLocationInMM();
			distance = sqrt(SQR(xDiff) + SQR(yDiff));

			if (distance > sampleRadiusInMM)
				continue;


			fibreMUId = currentFibre->getMotorUnit();
			curMotorUnit = MD->getMotorUnitFromMasterList( fibreMUId - 1 );

			MSG_ASSERT((fibreMUId - 1) < MD->getNumMotorUnits(), "Bound error");
			if (areaByMotorUnitIndex[ fibreMUId - 1 ] < 0)
			{
				areaByMotorUnitIndex[ fibreMUId - 1 ] =
					(float) calculateAreaOfIntersectionOfThreeCircles(
								0, 0,
								MD->getMuscleDiameter()/2.0f,
								xThrow, yThrow, sampleRadiusInMM,
								curMotorUnit->getXLocationInMM(),
								curMotorUnit->getYLocationInMM(),
								curMotorUnit->getDiameter() / 2.0f);
				compare =
							(float) calculateAreaOfIntersectionOfTwoCircles(
								xThrow, yThrow, sampleRadiusInMM,
								curMotorUnit->getXLocationInMM(),
								curMotorUnit->getYLocationInMM(),
								curMotorUnit->getDiameter() / 2.0f);

			}
			if (areaByMotorUnitIndex[ fibreMUId - 1 ] < 0.5)
				continue;

			countsByMotorUnitIndex[ fibreMUId - 1 ]++;
		}

		/** clear out the R-Tree search results, if any */
		if (rtreeResults.results_ != NULL)
			ckfree(rtreeResults.results_);
		memset(&rtreeResults, 0, sizeof(rtreeResults));



		/** for each unit with more than 0 fibres, save the data */
		for (i = 0; i < MD->getNumMotorUnits(); i++)
		{

			if (countsByMotorUnitIndex[i] > 0)
			{
				MSG_ASSERT(areaByMotorUnitIndex[i] >= 0,
							"Internal calculation error");

				if (areaByMotorUnitIndex[i] > 0)
				{
					muFibreDensitySamples[numHits++] =
								(((float) countsByMotorUnitIndex[i])
								 		/ areaByMotorUnitIndex[i]);

					if (numHits == NUM_MOTOR_UNITS_SAMPLED)
						break;
				}
			}
		}
	}

	if (sanityCheck >= 1000000)
	{
		LogCrit("Sanity check failed\n");
		goto CLEANUP;
	}

	/**
	 * now we have the number of counts per motor unit per sample,
	 * and can calculate the variance/std dev and mean, and turn
	 * this into some output
	 */
	mean = variance = 0;
	oldMax = max = 0;
	for (i = 0; i < numHits; i++)
	{
		mean += muFibreDensitySamples[i];
		if (max < muFibreDensitySamples[i])
		{
			max = muFibreDensitySamples[i];
			MSG_ASSERT(max >= oldMax, "Overflow");
			oldMax = max;
		}
	}
	mean = mean / (float) numHits;

	for (i = 0; i < numHits; i++)
	{
		variance += fabs(mean - muFibreDensitySamples[i]);
	}

	LogInfo("\n");
	LogInfo("Fibre Density Mean      : %s\n", niceDouble(mean));
	LogInfo("              Std. Dev. : %s\n", niceDouble(sqrt(variance)));
	LogInfo("\n");


//	{
//		GSimplePlot *plot;
//		char scriptname[FILENAME_MAX];
//		char outputname[FILENAME_MAX];
//		int *counts, index;
//		int nBins;
//
//		nBins = (int) (max + 0.5) + 1;
//		counts = (int *) ckalloc(nBins * sizeof(int));
//		memset(counts, 0, nBins * sizeof(int));
//
//		slnprintf(scriptname, FILENAME_MAX,
//				"%s/plots/createMUFibreDensityPlot",
//				muscleDirectory);
//		slnprintf(outputname, FILENAME_MAX,
//				"muFibreDensityPlot_post");
//
//
//		for (i = 0; i < numHits; i++)
//		{
//			index = (int) ((muFibreDensitySamples[i]) + 0.5);
//			MSG_ASSERT(index < nBins, "Number of bins exceeded");
//			counts[index]++;
//		}
//
//		plot = gHistPltCreate(
//				scriptname,
//				outputname,
//				"Motor Unit Muscle Fibre Density",
//				"Motor Unit Muscle Fibre/mm^{2}", "Number");
//
//		gHistPltAddSeries(plot,
//				1, NULL, "MUFibreDensity_post.txt");
//
//		for (i = 0; i < nBins; i++)
//		{
//			gHistPltAddPointToSeries(
//				plot, 1,
//				i, counts[i]
//			);
//		}
//		gHistPltSetMeanInSeries(plot, 1, mean);
//
//		gHistPltClose(plot);
//		ckfree(counts);
//	}
	status = 1;

CLEANUP:
	if (countsByMotorUnitIndex != NULL)
		ckfree(countsByMotorUnitIndex);
	if (areaByMotorUnitIndex != NULL)
		ckfree(areaByMotorUnitIndex);

	return status;
}


static int dumpMuLayout(MuscleData *MD, const char *muscleDirectory)
{
	FILE *centroidFP, *radiusFP;
	FILE *matlabFP;
	GFP *gfp;
	char gnufile[FILENAME_MAX];
	char filename[FILENAME_MAX];
	double x, y;
	double rx, ry;
	int i;
	double theta;
	double distance;

	slnprintf(gnufile, FILENAME_MAX,
			"%s/plots/createLayoutPlot", muscleDirectory);
	gfp = gPltCreate(gnufile, "mu-layout", NULL, "simtext");

	gPltWriteSetupLine(gfp,
		    "    set title \"Muscle Layout (in mm)\" "
		    "font \"HelviticaBold\"\n");
	gPltWriteSetupLine(gfp,
		    "    set ylabel \"Y\" font \"HelviticaBold\"\n");
	gPltWriteSetupLine(gfp,
		    "    set xlabel \"X\" font \"HelviticaBold\"\n");
	gPltWriteSetupLine(gfp,
		    "    set nokey\n");
	gPltSetSquare(gfp);
	//gPltWriteSetupLine(gfp, "    set key outside\n");
	gPltWriteSetupLine(gfp, "\n");
	gPltWriteBodyLine(gfp,
		    "    plot \"${DIR}/cell-layout.txt\" with points, \\\n");



	slnprintf(filename, FILENAME_MAX,
			"%s/plots/MU-layout-C.txt", muscleDirectory);
	centroidFP = fopenpath(filename, "w");


	/** put the muscle boundary ring in */
	slnprintf(filename, FILENAME_MAX,
			"%s/plots/muscleDiameter.txt", muscleDirectory);
	radiusFP = fopenpath(filename, "w");
	gPltWriteBodyLine(gfp,
		"      \"${DIR}/muscleDiameter.txt\" with lines 1, \\\n");

	for (theta = 0; theta <= M_PI * 2.0; theta = theta + (M_PI / 100))
	{
		rx = CARTESIAN_X_FROM_POLAR(MD->muscleDiameter_ / 2.0, theta);
		ry = CARTESIAN_Y_FROM_POLAR(MD->muscleDiameter_ / 2.0, theta);
		fprintf(radiusFP, "%f %f\n", rx, ry);
		if (ferror(radiusFP))
		{
		    LogCrit("Write to debug file '%s' failed -- aborting\n",
		            filename);
		    return 0;
		}
	}
	fclose(radiusFP);


	/** add in all the motor unit territories */
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		slnprintf(filename, FILENAME_MAX,
				"%s/plots/MU-layout-R%d.txt", muscleDirectory, i);
		radiusFP = fopenpath(filename, "w");

		x = CARTESIAN_X_FROM_POLAR(
						MD->motorUnit_[i]->mu_loc_r_mm_,
						MD->motorUnit_[i]->mu_loc_theta_
					);
		y = CARTESIAN_Y_FROM_POLAR(
						MD->motorUnit_[i]->mu_loc_r_mm_,
						MD->motorUnit_[i]->mu_loc_theta_
					);

		gPltWriteBodyLine(gfp,
		        "      \"${DIR}/MU-layout-R%d.txt\" with lines 3", i);

		if (i < MD->nMotorUnitsInMuscle_ - 1)
		{
		    gPltWriteBodyLine(gfp, ", \\\n");
		} else
		{
		    gPltWriteBodyLine(gfp, "\n");
		}


		for (theta = 0; theta <= M_PI * 2.0; theta = theta + (M_PI / 100))
		{
			rx = CARTESIAN_X_FROM_POLAR(
							MD->motorUnit_[i]->mu_diameter_mm_ / 2.0, theta
					) + x;
			ry = CARTESIAN_Y_FROM_POLAR(
							MD->motorUnit_[i]->mu_diameter_mm_ / 2.0, theta
					) + y;

		    distance = sqrt(SQR(rx) + SQR(ry));
		    if (distance <= MD->muscleDiameter_ / 2.0)
			{
		        fprintf(radiusFP, "%f %f\n", rx, ry);
		    }
		    if (ferror(radiusFP))
			{
		        LogCrit("Write to debug file '%s' failed -- aborting\n",
		                filename);
		        return 0;
		    }
		}
		fclose(radiusFP);
	}

	gPltClose(gfp);
	fclose(centroidFP);

#if 0
#ifndef OS_WINDOWS_NT
	{
		gWaitList *waitList = gPltCreateWaitList();
		gPltRunV(waitList, gnufile, NULL);
		gPltWaitAll(waitList);
	}
#endif
#endif

	/** dump the same things for Matlab */
	{
		slnprintf(filename, FILENAME_MAX,
				"%s/plots/createLayoutPlot.m", muscleDirectory);
		matlabFP = fopenpath(filename, "w");

		fprintf(matlabFP, "cell_layout = load('cell-layout.txt');\n");
		fprintf(matlabFP, "muscleDiameter = load('muscleDiameter.txt');\n");
		fprintf(matlabFP, "needle = load('needle-layout.txt');\n");
		fprintf(matlabFP, "MU_layout_C = load('MU-layout-C.txt');\n");
		for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
		{
		    fprintf(matlabFP,
		        "MU_layout_R%d = load('MU-layout-R%d.txt');\n", i, i);
		}
		fprintf(matlabFP, "\nclf;\n");

		fprintf(matlabFP,
		        "plot(cell_layout(:,1), cell_layout(:,2), 'r.');\n");
		fprintf(matlabFP,
		        "hold;\n");
		fprintf(matlabFP,
		        "plot(needle(:,1), needle(:,2), 'k-');\n");
		fprintf(matlabFP,
		        "plot(muscleDiameter(:,1), muscleDiameter(:,2), 'r-');\n");
		fprintf(matlabFP,
		        "plot(MU_layout_C(:,1), MU_layout_C(:,2), 'g^');\n");

		for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
		{
		    fprintf(matlabFP,
		        "plot(MU_layout_R%d(:,1), MU_layout_R%d(:,2), 'b-');\n",
		                i, i);
		}

		fprintf(matlabFP, "\n");
		fprintf(matlabFP, "filename = 'layout.eps';\n");
		fprintf(matlabFP, "print( '-depsc', filename );\n");
		fclose(matlabFP);
	}
	return 1;
}

#endif /* DEBUG_LAYOUT */

/**
 * Lay out the motor unit centroids so that the appropriate
 * density is found in the muscle center using our "seed-scatter"
 * plan
 */
static int
gridInitialLayoutOfMotorUnitCentroids(
		MuscleData *MD,
		float muscleRadiusInMM
	)
{
	double theta;
	double tmpX, tmpY;
	float *centerX, *centerY;
	double tmpDistanceInMM;
	BITSTRING muChosenBitstring;
	BITSTRING centerBitstring;
	int halfRow, numCentreRows, numCentres;
	int muCounter, layoutIteration, muIndex;
	int gridCentreResetCount, gridIndex;
	int muUnchosenCount;
	double gridSpacing;
	int i, j, k;


	/**
	 * define locations of motor unit centroids.
	 * Centroid defined based on a random radial distance theta
	 * and a random angle theta.
	 *
	 * theta ranges from zero to
	 * (radius of the muscle) - (radius of the motor unit territory).
	 */
	//LogInfo("Using GRID layout to define motor unit centers\n");

	muChosenBitstring = ALLOC_BITSTRING(MD->nMotorUnitsInMuscle_);
	ZERO_BITSTRING(muChosenBitstring, MD->nMotorUnitsInMuscle_);


	/**
	 * Set up the grid of "throwing" centers
	 *
	 * We will have 2*halfRow + 1 points per row, in a square.
	 *
	 * We want the points to extend somewhat out the sides of the
	 * area, so the grid spacing is therefore
	 */
	halfRow = 8;
	numCentreRows = (2 * halfRow) + 1;
	numCentres = numCentreRows * numCentreRows ;
	gridSpacing = (muscleRadiusInMM / (halfRow * 0.8));

	centerX = (float *) ckalloc(sizeof(float) * numCentres);
	centerY = (float *) ckalloc(sizeof(float) * numCentres);
	centerBitstring = ALLOC_BITSTRING(numCentres);

	k = 0;
	for (i = (-halfRow); i <= halfRow; i++)
	{
		for (j = (-halfRow); j <= halfRow; j++)
		{
			centerX[k] = (float) (i * gridSpacing);
			centerY[k] = (float) (j * gridSpacing);
			k++;
		}
	}
	{
		FILE *tfp;
		tfp = fopen("GridCentres.txt", "w");
		for (i = 0; i < numCentres; i++)
		{
			fprintf(tfp, "%f %f\n", centerX[i], centerY[i]);
		}
		fclose(tfp);
	}

	/**
	 * Keep track of the grid centers we have tried in the
	 * center bitstring
	 */
	ZERO_BITSTRING(centerBitstring, numCentres);

	/**
	 * Loop over the grid until we are done laying out the MU's
	 */
	muCounter = 0;
	gridCentreResetCount = 0;
	layoutIteration = 0;
	//LogInfo("    Grid Muscle Layout Iteration %d\n", layoutIteration);
	while (muCounter < MD->nMotorUnitsInMuscle_)
	{

		/**
		 * find an unassigned MU, linearly probing
		 * for next unused MU to ensure a match
		 */
		muIndex = intRangeRandom(
		                        MD->nMotorUnitsInMuscle_ - 1);
		while (GET_BIT(muChosenBitstring, muIndex) != 0)
				muIndex = (muIndex + 1) % MD->nMotorUnitsInMuscle_;

		muUnchosenCount = 0;

		/**
		 * now iterate through the grid centers with this
		 * MU, resetting the bitmap of choices when necessary
		 */
		do
		{
			/** choose an unused center */
			gridIndex = intRangeRandom(numCentres - 1);
			while (GET_BIT(centerBitstring, gridIndex) != 0)
				gridIndex = (gridIndex + 1) % numCentres;


			/** flag this bit as used */
			SET_BIT(centerBitstring, gridIndex, 1);

			/**
			 * if we have reached the number of centers, then
			 * reset the list again
			 */
			gridCentreResetCount++;
			if (gridCentreResetCount >= numCentres)
			{
				gridCentreResetCount = 0;
				ZERO_BITSTRING(centerBitstring, numCentres);
				//LogInfo("    Grid Muscle Layout Iteration %d\n",
				//			layoutIteration);
				++layoutIteration;
			}

			/** ensure grid position is inside of muscle */
//			tmpDistanceInMM = SQR(centerX[gridIndex]) + SQR(centerY[gridIndex]);
//			if (tmpDistanceInMM > SQR(muscleRadiusInMM))
//			{
//				continue;
//			}


			/**
			 * get a copy of the center for "throwing",
			 * and add a random "throw" distance to it
			 * */
			tmpX = centerX[gridIndex];
			tmpY = centerY[gridIndex];

			tmpX += gauss01() * (gridSpacing * 2.0);
			tmpY += gauss01() * (gridSpacing * 2.0);

			/**
			 * If the layout position chosen is not
			 * within the circle, we re-choose from
			 * another grid center
			 */
			tmpDistanceInMM = CARTESIAN_DISTANCE(0, 0, tmpX, tmpY);

			/** if we are in the muscle, we are done, so break */
			if (tmpDistanceInMM < muscleRadiusInMM)
			{
//				LogInfo("      MU %d placed at radius %f (< %f)\n",
//						muIndex, tmpDistanceInMM, muscleRadiusInMM);
				break;
			}

			/**
			 * how often have we done this?  We want to
			 * sanity check that we are not having too
			 * many problems placing centers
			 */
			muUnchosenCount++;
			if (muUnchosenCount > 1000)
			{

				//LogInfo("    Grid Muscle Layout Sanity Check Placement\n");

				/**
				 * try a placement with gaussian from
				 * the center
				 */
				tmpX = gauss01() * muscleRadiusInMM / 3.0;
				tmpY = gauss01() * muscleRadiusInMM / 3.0;

				tmpDistanceInMM = CARTESIAN_DISTANCE(0, 0,  tmpX, tmpY);

				/**
				 * if we are totally at a loss, plug it
				 * in the center
				 */
				if (tmpDistanceInMM > muscleRadiusInMM)
				{
					//LogInfo("Forced location 0,0 for MU %d\n", muIndex);
					tmpX = tmpY = 0.0;
				}
				break;
			}

		} while (1);

//		LogInfo("      MU %d (%f, %f) placed from center %d (%f, %f)\n",
//				muIndex, tmpX, tmpY,
//				gridIndex, centerX[gridIndex], centerY[gridIndex]);

		/**
		 * we have found a home for the MU, so record the location
		 * MU as chosen
		 */
		SET_BIT(muChosenBitstring, muIndex, 1);

		            /** rectangular to polar conversion */
		theta = POLAR_THETA_FROM_CARTESIAN(tmpX, tmpY);
		MD->motorUnit_[muIndex]->mu_loc_r_mm_ = (float) tmpDistanceInMM;
		MD->motorUnit_[muIndex]->mu_loc_theta_ = (float) theta;
		{
			double tx, ty;

			tx = CARTESIAN_X_FROM_POLAR(
							MD->motorUnit_[muIndex]->mu_loc_r_mm_,
						MD->motorUnit_[muIndex]->mu_loc_theta_
					);
			ty = CARTESIAN_Y_FROM_POLAR(
							MD->motorUnit_[muIndex]->mu_loc_r_mm_,
						MD->motorUnit_[muIndex]->mu_loc_theta_
					);
			if ((fabs(tx - tmpX) > 0.00001) || (fabs(ty - tmpY) > 0.00001))
			{
				MSG_FAIL("Bad polar conversion");
			}

		}

		muCounter++;
	}

	FREE_BITSTRING(muChosenBitstring);
	FREE_BITSTRING(centerBitstring);
	ckfree(centerX);
	ckfree(centerY);

	return 1;
}


/**
 * Lay out the motor unit centroids so that the appropriate
 * density is found in the muscle center using a vector/length
 * plan
 */
static int
randomInitialLayoutOfMotorUnitCentroids(
		MuscleData *MD,
		float muscleRadius
	)
{
	double tmp;
	int i;

	/**
	 * define locations of motor unit centroids.
	 * Centroid defined based on a random radial distance theta
	 * and a random angle theta.
	 *
	 * theta ranges from zero to
	 * (radius of the muscle) - (radius of the motor unit territory).
	 */
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{

		tmp = fabs((double) floatSignedRangeRandom(1.0));
		MD->motorUnit_[i]->mu_loc_r_mm_ = (float)
		        ((muscleRadius -
		                (MD->motorUnit_[i]->mu_diameter_mm_ / 2.0))
		                * sqrt(tmp));

		MD->motorUnit_[i]->mu_loc_theta_ = (float)
		        fabs( (double) floatSignedRangeRandom(
		                        (float) (2.0 * M_PI) ) );
	}

	return 1;
}


static double
calculateRequiredMotorUnitDensity(MuscleParameters *muscleParams)
{
	return (1.0 /
		    ((double) muscleParams->fibreDensity
		                * (double) muscleParams->areaPerFibre));
}

/**
 * determine whether the density is correct
 */
static int
compareMotorUnitDensityWithTolerance(
		MuscleData *MD,
		MuscleParameters *muscleParams,
		double muscleRadius,
		double tolerance,
		double *densityFound,
		double densityRequired
	)
{
	int nMotorUnitsIntersectingCentre;
	double desiredMUDensity;

	nMotorUnitsIntersectingCentre =
		        countMotorUnitsIntersectingCentre(MD);

	desiredMUDensity = calculateRequiredMotorUnitDensity(muscleParams);

	*densityFound = nMotorUnitsIntersectingCentre;


	/** if we are too low, return -ve */
	if (nMotorUnitsIntersectingCentre <=
		        (desiredMUDensity - (desiredMUDensity * tolerance)))
		return (-1);


	/** if we are too high, return +ve */
	if (nMotorUnitsIntersectingCentre >=
		        (desiredMUDensity + (desiredMUDensity * tolerance)))
		return 1;

	/** return 0 on valid compare */
	return 0;
}


/**
 * Fix up the motor unit density in the center by moving MU's in/out
 * density is found in the muscle center
 */
static int
adjustMotorUnitDensity(
		MuscleData *MD,
		MuscleParameters *muscleParams,
		float muscleRadius
	)
{
	int adjustmentLoopCounter;
	int comparisonResult;
	double densityFound;
	double densityDesired;
	int i;

	/**
	 * If the muscle does not have (muscle fibre density) - 5% to
	 * (muscle fibre density) + 5% motor units that include the muscle
	 * centre than each motor unit centoid location is moved
	 * closer or farther from the muscle centre.
	 *
	 * This is performed until a suitable motor unit density is
	 * attained
	 */

	densityDesired = calculateRequiredMotorUnitDensity(muscleParams);

	adjustmentLoopCounter = 1;

	/**
	 * First push the MU's together if need be . . .
	 */
	comparisonResult = compareMotorUnitDensityWithTolerance(
		                        MD,
								muscleParams,
		                        muscleRadius,
		                        0.05,
		                        &densityFound,
		                        densityDesired);

	while ((comparisonResult != 0) && (adjustmentLoopCounter < 1000))
	{


		if (comparisonResult < 0)
		{

		    LogInfo("MU density too low (%f < f), adjusting . . .\n",
		                densityFound, densityDesired);

		    /** pull in motor unit centres */
		    for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
			{
		        MD->motorUnit_[i]->mu_loc_r_mm_ = (float)
		                (MD->motorUnit_[i]->mu_loc_r_mm_
		                        - (muscleRadius * .01));
		    }
		    adjustmentLoopCounter = adjustmentLoopCounter + 1;
		    comparisonResult = compareMotorUnitDensityWithTolerance(
		                        MD, muscleParams,
								muscleRadius, 0.05, &densityFound,
		                        densityDesired);
		}



		/**
		 * But recover from too much of a push . . .
		 */
		if (comparisonResult > 0)
		{

		    LogInfo("MU density too high (%f > %f), adjusting . . .\n",
		                densityFound, densityDesired);

		    for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
			{

		        /** pull the MU out . . . */
		        MD->motorUnit_[i]->mu_loc_r_mm_ = (float)
		                (MD->motorUnit_[i]->mu_loc_r_mm_
		                        + (muscleRadius * .01));

		        /** clip any MU's which now extend out of the muscle */
		        if (MD->motorUnit_[i]->mu_loc_r_mm_ >
		                (muscleRadius -
		                        MD->motorUnit_[i]->mu_diameter_mm_ / 2.))
				{
		            MD->motorUnit_[i]->mu_loc_r_mm_ = (float)
		                    (muscleRadius -
		                        MD->motorUnit_[i]->mu_diameter_mm_ / 2.);
		        }
		    }
		    adjustmentLoopCounter = adjustmentLoopCounter + 1;
		    comparisonResult = compareMotorUnitDensityWithTolerance(
		                        MD, muscleParams,
								muscleRadius, 0.05, &densityFound,
		                        densityDesired);
		}
	}

	if (adjustmentLoopCounter > 1)
	{
		LogInfo("\n");
		LogInfo("Muscle Density Adjusted!\n");
		LogInfo("MU territory centroid adjusted in %d attempts\n",
		                adjustmentLoopCounter);
		LogInfo("\n");
	}


	return 1;
}


/**
 * Lay out the motor unit centroids so that the appropriate
 * density is found in the muscle center
 */
static int
layoutMotorUnitCentroids(
		MuscleData *MD,
		MuscleParameters *muscleParams,
		float *muscleDiameter,
		int motorUnitLayoutType,
		double *muDensityFound,
		double *muDensityRequired
	)
{
	double muscleRadius;
	int nMotorUnitsIntersectingCentre;
	int comparisonResult;
	int status = 1;
	const int MAX_TRIES = 10;
	int sanityCheck = MAX_TRIES;

	/**
	 * define locations of motor unit centroids.
	 * Centroid defined based on a random radial distance r
	 * and a random angle theta.
	 *
	 * r ranges from zero to
	 * (radius of the muscle) - (radius of the motor unit territory).
	 */
	LogInfo("Defining motor unit centers\n");
	muscleRadius = (*muscleDiameter) / 2.0;

	if (motorUnitLayoutType == RANDOM_MU_LAYOUT)
	{
		LogInfo("Using RANDOM MU layout algorithm\n");
		randomInitialLayoutOfMotorUnitCentroids(MD,
				(float) muscleRadius);

		if ( ! adjustMotorUnitDensity(MD,
				muscleParams, (float) muscleRadius) )
		    status = 0;

	} else if (motorUnitLayoutType == GRID_MU_LAYOUT)
	{
		LogInfo("Using GRID MU layout algorithm\n");

		comparisonResult = (-1);

		while (comparisonResult != 0)
		{

			if (sanityCheck-- < 0)
			{
				LogInfo("Could not get valid muscle density in %d tries\n",
						MAX_TRIES);
				return 0;
			}

		    status = 1;
		    gridInitialLayoutOfMotorUnitCentroids(MD,
					(float) muscleRadius);

		    *muDensityRequired =
					calculateRequiredMotorUnitDensity(muscleParams);
		    comparisonResult = compareMotorUnitDensityWithTolerance(
					MD, muscleParams,
					muscleRadius,
					0.05,
					muDensityFound,
					*muDensityRequired);


		    /**
		     * area = PI * r^2
		     *
		     * difference in density \propto difference in area
		     */
		    if (comparisonResult != 0)
			{
		        double densityFoundRatio;
		        double areaRatio;
		        double sqrtAreaRatio;

		        status = 0;

		        LogInfo( "%s -- density was %f, required %f\n",
		                "Retrying MU layout",
		                *muDensityFound,
		                *muDensityRequired);


		        /**
		         * calculate the portion of the required density
		         * we now have
		         */
		        densityFoundRatio =
		                ((*muDensityFound) / (*muDensityRequired));

		        /** convert from density to area */
		        areaRatio = (double) 1.0 / (double) densityFoundRatio;

		        /**
		         * trim it down to not over-correct
		         */
		        areaRatio = ((areaRatio - 1.0) * 0.75) + 1.0;

		        sqrtAreaRatio = sqrt(areaRatio);

#ifndef OS_WINDOWS_NT
		        if (isnan(sqrtAreaRatio))
				{
		            LogCrit(" - Muscle cannot be adjusted by a NAN\n");
		            LogCrit(" - sqrtAreaRatio     : %f\n",
		                    sqrtAreaRatio);
		            LogCrit(" - areaRatio         : %f\n",
		                    areaRatio);
		            LogCrit(" - densityFoundRatio : %f\n",
		                    densityFoundRatio);
		            LogCrit(" - muDensityFound    : %f\n",
		                    *muDensityFound);
		            LogCrit(" - muDensityRequired : %f\n",
		                    *muDensityRequired);
		            return 0;
		        }
#endif

		        /**
		         * scale by the sqrt of this relative to the diameter
		         *
		         *  r  =    r       * sqrt(differenceRatio)
		         * 2r  =    r       * sqrt(differenceRatio) * 2
		         * 2r  = ((2r) / 2) * sqrt(differenceRatio) * 2
		         * 2r  = (2r)       * sqrt(differenceRatio)
		         *
		         */
		        (*muscleDiameter) *= (float) sqrt(areaRatio);

				LogInfo("\t Muscle Diameter has changed to %f\n",
		                *muscleDiameter);
		    }
		}

	} else
	{
		LogError("Unknown Layout Type\n");
	}

	nMotorUnitsIntersectingCentre =
		        countMotorUnitsIntersectingCentre(MD);

	LogInfo("There are %d motor units %s\n",
		        nMotorUnitsIntersectingCentre,
		        "that pass over the muscle centre.");

	return status;
}

//static int
//MUrTreeSearchCallback__(int id, void *voidResultList)
//{
//    struct rTreeResultList *result
//				= (struct rTreeResultList *) voidResultList;
//
//    listMkCheckSize(
//                result->nEntries_ + 1,
//                (void **) &result->results_,
//                &result->nBlocks_,
//                16,
//                sizeof(int), __FILE__, __LINE__);
//    result->results_[result->nEntries_++] = id;
//
//    return 1; /** keep searching */
//}

static float
calculateDistanceToFibre(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int chosenMUIndex
	)
{
	double muCentreX, muCentreY;

	muCentreX = (float) CARTESIAN_X_FROM_POLAR(
				MD->motorUnit_[chosenMUIndex]->mu_loc_r_mm_,
				MD->motorUnit_[chosenMUIndex]->mu_loc_theta_);

	muCentreY = (float) CARTESIAN_Y_FROM_POLAR(
				MD->motorUnit_[chosenMUIndex]->mu_loc_r_mm_,
				MD->motorUnit_[chosenMUIndex]->mu_loc_theta_);

	return (float) CARTESIAN_DISTANCE(muCentreX, muCentreY,
				xLocationOfFibreInMM, yLocationOfFibreInMM);
}

double
fibreLayoutWeighting_D_WeightByPercentExpected(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float totalMissingFraction = 0;
	float curMissingFraction;
	int i;

	/**
	 * calculate the total number of missing fibres
	 */
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction =
					MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_
				/ (float) MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_expectedNumFibres_;
		totalMissingFraction += curMissingFraction;
	}

	return totalMissingFraction;
}


double
fibreLayoutWeighting_WeightByPercentExpected(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList,
		double totalMissingFraction
	)
{
	float curMissingFraction;

	if (totalMissingFraction == 0)
	{
		return (-1);
	}


	/**
	 * return the portion we represent of the total
	 */
	curMissingFraction =
					MD->motorUnit_[
							muID - 1
						]->mu_nFibres_
				/ (float) MD->motorUnit_[
							muID - 1
						]->mu_expectedNumFibres_;

	return 1 - (curMissingFraction / totalMissingFraction);
}


double
fibreLayoutWeighting_D_WeightByNumFibres(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	int totalNumberOfFibres = 0;
	int nFibres;
	int i;

	if (numEligibleMotorUnits == 0)
	{
		return (-1);
	}

	/** calculate the total number of missing fibres */
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		nFibres = MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_;
		if (nFibres == 0)
			nFibres = 1;
		totalNumberOfFibres += nFibres;
	}

	return totalNumberOfFibres;
}


double
fibreLayoutWeighting_WeightByNumFibres(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList,
		double totalNumberOfFibres
	)
{
	if (numEligibleMotorUnits == 0)
	{
		return (-1);
	}
	if (totalNumberOfFibres <= 0)
	{
		return (-1);
	}

	/**
	 * return 1 - the fraction we are of the missing total
	 */
	return 1.0 -
			((float) MD->motorUnit_[ muID - 1 ]->mu_nFibres_ /
			 				totalNumberOfFibres);
}

double
fibreLayoutWeighting_D_WeightByNumFibresMax(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	return 0;
}

double
fibreLayoutWeighting_WeightByNumFibresMax(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList,
		double denominator
	)
{
	int maxNumberOfFibres;
	int i;

	if (numEligibleMotorUnits == 0)
	{
		return (-1);
	}

	/** calculate the total number of missing fibres */
	maxNumberOfFibres = MD->motorUnit_[
							eligibleMotorUnitList[0] - 1
						]->mu_nFibres_;
	for (i = 1; i < numEligibleMotorUnits; i++)
	{
		if (maxNumberOfFibres < MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_)
						{
			maxNumberOfFibres = MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_;
		}
	}

	if (maxNumberOfFibres == 0)
	{
		return (-1);
	}


	/**
	 * return 1 - the fraction we are of the missing total
	 */
	return maxNumberOfFibres - MD->motorUnit_[ muID - 1 ]->mu_nFibres_;
}

double
fibreLayoutWeighting_D_WeightByDistance(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float distance;
	float distanceSum;
	float minDistance = FLT_MAX;
	//int minDistanceIndex = (-1);
	int i;

	/** calculate the sum of all the distances */
	distanceSum = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		distance = calculateDistanceToFibre(
						MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
						eligibleMotorUnitList[i] - 1);
		//distance = distance * distance;
		if (distance < minDistance)
		{
			minDistance = distance;
			//minDistanceIndex = i;
		}

		distanceSum += (float) (distance / (MD->motorUnit_[
							eligibleMotorUnitList[0] - 1
						]->mu_expected_diameter_mm_ / 2.0));
	}

//   LogInfo("MU %d has min distance %f\n",
//					eligibleMotorUnitList[minDistanceIndex] - 1,
//				minDistance);

	return distanceSum;
}


double
fibreLayoutWeighting_WeightByDistance(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList,
		double distanceSum
	)
{
	float distance;

	distance = calculateDistanceToFibre(
						MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
						muID - 1);

	return 1.0 - (
				(
					distance /
						(MD->motorUnit_[ muID - 1 ]
							->mu_expected_diameter_mm_ / 2.0)
				) / distanceSum
			);
//    return 1.0 - (
//				(
//					SQR(distance) /
//						SQR(MD->motorUnit_[ muID - 1 ]
//							->mu_expected_diameter_mm_ / 2.0)
//				) / distanceSum
//			);
}


inline static float
calc_WeightByMissingOverExpected(
		MuscleData *MD,
		int muIndex
	)
{
	return
		( MD->motorUnit_[ muIndex ]->mu_expectedNumFibres_
				- MD->motorUnit_[ muIndex ]->mu_nFibres_ )
		/ (float) MD->motorUnit_[ muIndex ]->mu_expectedNumFibres_;
}

double
fibreLayoutWeighting_D_WeightByMissingOverExpected(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float sumM;
	int i;


	/** calculate all the distances */
	sumM = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		sumM += calc_WeightByMissingOverExpected(
						MD, eligibleMotorUnitList[i] - 1
					);
	}

	return sumM;
}


double
fibreLayoutWeighting_WeightByMissingOverExpected(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList,
		double sumM
	)
{
	double fraction;

	fraction = (calc_WeightByMissingOverExpected(MD, muID - 1) / sumM);
	return fraction;
}


double
fibreLayoutWeighting_D_WeightByK(
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	return 1.0;
}


double
fibreLayoutWeighting_WeightByK(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList,
		double denominator
	)
{
	return 1.0 / (float) numEligibleMotorUnits;
}

typedef struct LayoutWeightingFunctionData
{
	const char *name_;
	const char *description_;
	MFL_WeightingFunction *function_;
	MFL_WeightingDenominatorFunction *denominator_;
	float defaultProb_;
} LayoutWeightingFunctionData;

static int sNumberOfWeigtingLayoutFunctions = (-1);
static LayoutWeightingFunctionData sWeightingFunctionDefinitions[] = {
		{
			"Weight By Number of Fibres (Load)",
			"(1 - (n_j)/(totalFibres)) / (#MU - 1)",
			fibreLayoutWeighting_WeightByNumFibres,
			fibreLayoutWeighting_D_WeightByNumFibres,
			(-1)
		},
//		{
//			"Weight By Missing/Expected",
//			"P_M_j = M_j / Sum_i=i^k M_i; M_i = #missing/#expected",
//			fibreLayoutWeighting_WeightByMissingOverExpected,
//			fibreLayoutWeighting_D_WeightByMissingOverExpected,
//			0.0f
//		},
		{
			"Weight By Distance",
			"Be more likely to adopt nearby MU's",
			fibreLayoutWeighting_WeightByDistance,
			fibreLayoutWeighting_D_WeightByDistance,
			0.5f //0.25f
		},
		{
			"Weight By % Expected Fibres (Genetic Bias)",
			"Weight by # of \"lacking\" fibres normalized by size",
			fibreLayoutWeighting_WeightByPercentExpected,
			fibreLayoutWeighting_D_WeightByPercentExpected,
			0.3f
		},
		{
			"Weight By K",
			"Weight by Number of Eleigible MU's",
			fibreLayoutWeighting_WeightByK,
			fibreLayoutWeighting_D_WeightByK,
			0.1f
		},
		{
			NULL,
			NULL,
			NULL,
			NULL,
			(-1)
		}
	};


OS_EXPORT int
getNumFibreLayoutWeightingFunctions()
{
	if (sNumberOfWeigtingLayoutFunctions < 0)
	{
		double totalProb = 0;
		int i, fillInProbIndex = (-1);

		for (i = 0; sWeightingFunctionDefinitions[i].name_ != NULL; i++)
		{
			if (sWeightingFunctionDefinitions[i].defaultProb_ < 0)
			{
				MSG_ASSERT(fillInProbIndex == (-1),
						"Internal error filling in probs for layout funcs");
				fillInProbIndex = i;
			} else
			{
				totalProb += sWeightingFunctionDefinitions[i].defaultProb_;
			}
		}

		sNumberOfWeigtingLayoutFunctions = i;

		if (fillInProbIndex >= 0)
		{
			sWeightingFunctionDefinitions[fillInProbIndex].defaultProb_ =
						(float) (1.0 - totalProb);
			totalProb += sWeightingFunctionDefinitions[fillInProbIndex].defaultProb_;
		}
		MSG_ASSERT(fabs(totalProb - 1.0) < 0.00001,
						"Probability in layout functions bad");
	}

	return sNumberOfWeigtingLayoutFunctions;
}


OS_EXPORT MFL_WeightingFunction *
getFibreLayoutWeightingFunction(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutWeightingFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sWeightingFunctionDefinitions[index].function_;
}


OS_EXPORT MFL_WeightingDenominatorFunction *
getFibreLayoutWeightingDenominatorFunction(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutWeightingFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sWeightingFunctionDefinitions[index].denominator_;
}

OS_EXPORT float
getFibreLayoutWeightingFunctionWeighting(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutWeightingFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sWeightingFunctionDefinitions[index].defaultProb_;
}

OS_EXPORT const char *
getFibreLayoutWeightingFunctionDescription(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutWeightingFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sWeightingFunctionDefinitions[index].description_;
}

OS_EXPORT const char *
getFibreLayoutWeightingFunctionName(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutWeightingFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sWeightingFunctionDefinitions[index].name_;
}


static double
calculateLayoutWeighting(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		int numFibreLayoutWeightingFunctions,
		MFL_WeightingFunction **functionList,
		float *functionWeighting,
		double *denominatorList,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int muID,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	double curLayoutWeighting;
	double sumLayoutWeighting;
	int i;

	sumLayoutWeighting = 0;

	for (i = 0; i < numFibreLayoutWeightingFunctions; i++)
	{
//		LogInfo("%*sCalculating weighting %d: %s\n", 12, "", i,
//						sWeightingFunctionDefinitions[i].name_);
//		MD->validate();
		if (functionWeighting[i] > 0)
		{
			curLayoutWeighting = (*functionList[i])(
					totalDistanceToChosenCentroid,
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					muID,
					numEligibleMotorUnits, eligibleMotorUnitList,
					denominatorList[i]
				);
//		MD->validate();
			if (curLayoutWeighting > 0)
				sumLayoutWeighting += (curLayoutWeighting * functionWeighting[i]);
		}
//		LogInfo("%*sDone %s\n", 12, "", sWeightingFunctionDefinitions[i].name_);
	}

//    MD->validate();
	return sumLayoutWeighting;
}

int
fibreLayoutProbability_Uniform(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	int chosenMUIndex;

	(*rTreeIdListIndexChoice) = intRangeRandom(numEligibleMotorUnits);

	chosenMUIndex = eligibleMotorUnitList[(*rTreeIdListIndexChoice)] - 1;

	(*totalDistanceToChosenCentroid) = calculateDistanceToFibre(
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					chosenMUIndex
			);

	return chosenMUIndex;
}

/**
 * If we aren't choosing uniformly, we want to weight
 * the choice so that the the motor units which are
 * the most short of fibres are more likely to get
 * this fibre
 */
int
fibreLayoutProbability_NeedNFibres(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	int totalMissingFibres = 0;
	int sumMissingFibres = 0;

	int chosenMUIndex;
	double choice;
	int i;

	/** calculate the total */
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		totalMissingFibres +=
					(MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_expectedNumFibres_
					- MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_);
	}

	if (totalMissingFibres == 0)
	{
		return (-1);
	}

	/** make a choice, [0,1] */
	choice = localRandomDouble();

	/** now accumulate until covering the choice */
	(*rTreeIdListIndexChoice) = (-1);
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		sumMissingFibres +=
					(MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_expectedNumFibres_
					- MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_);
		if ((sumMissingFibres / totalMissingFibres) >= choice)
		{
			(*rTreeIdListIndexChoice) = i;
			break;
		}
	}

	/**
	 * sanity check -- if roundoff caused us to not
	 * find a fibre, then simply jump back to the top
	 * of the loop and re-choose
	 */
	if ((*rTreeIdListIndexChoice) < 0)
		return (-1);


	chosenMUIndex = eligibleMotorUnitList[*rTreeIdListIndexChoice] - 1;
	(*totalDistanceToChosenCentroid) = calculateDistanceToFibre(
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					chosenMUIndex
			);

	return chosenMUIndex;
}

int
fibreLayoutProbability_WeightByPercentExpected(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float totalMissingFraction = 0;
	float curMissingFraction;
	float sumMissingFractions = 0;

	int chosenMUIndex;
	double choice;
	int i;

	/** calculate the total */
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction =
					MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_
				/ (float) MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_expectedNumFibres_;
		totalMissingFraction += curMissingFraction;
	}

	if (totalMissingFraction == 0)
	{
		return (-1);
	}

	/** make a choice, [0,1] */
	choice = localRandomDouble();

	/** now accumulate until covering the choice */
	(*rTreeIdListIndexChoice) = (-1);
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction =
					MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_
				/ (float) MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_expectedNumFibres_;
		sumMissingFractions += curMissingFraction;
		if ((sumMissingFractions / totalMissingFraction) >= choice)
		{
			(*rTreeIdListIndexChoice) = i;
			break;
		}
	}

	/**
	 * sanity check -- if roundoff caused us to not
	 * find a fibre, then simply jump back to the top
	 * of the loop and re-choose
	 */
	if ((*rTreeIdListIndexChoice) < 0)
		return (-1);


	chosenMUIndex = eligibleMotorUnitList[(*rTreeIdListIndexChoice)] - 1;
	(*totalDistanceToChosenCentroid) = calculateDistanceToFibre(
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					chosenMUIndex
			);

	return chosenMUIndex;
}


int
fibreLayoutProbability_WeightByNumFibres(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float totalMissingFraction;
	float curMissingFraction;
	float sumMissingFractions;
	int totalNumberOfFibres;

	int chosenMUIndex;
	double choice;
	int i;

	/** calculate the total number of missing fibres */
	totalNumberOfFibres = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		totalNumberOfFibres += MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_;
	}

	if (totalNumberOfFibres == 0)
	{
		return (-1);
	}


	/** total the missing portion for normalization */
	totalMissingFraction = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction = (float)
			(( 1.0 - (
					( MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_
					/ (float) totalNumberOfFibres) ) )
			/ (
					numEligibleMotorUnits
			));

		totalMissingFraction += curMissingFraction;
	}

	// MSG_ASSERT(totalMissingFraction == 1, "count normalization bad");



	/** make a choice, [0,1] */
	choice = localRandomDouble();

	/** now accumulate until covering the choice */
	(*rTreeIdListIndexChoice) = (-1);
	sumMissingFractions = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction =
			( 1 - (
					( MD->motorUnit_[
							eligibleMotorUnitList[i] - 1
						]->mu_nFibres_
					/ (float) totalNumberOfFibres) ) )
			/ (
					numEligibleMotorUnits - 1
			);

		sumMissingFractions += curMissingFraction;
		if ((sumMissingFractions / totalMissingFraction) >= choice)
		{
			(*rTreeIdListIndexChoice) = i;
			break;
		}
	}


	/**
	 * sanity check -- if roundoff caused us to not
	 * find a fibre, then simply jump back to the top
	 * of the loop and re-choose
	 */
	if ((*rTreeIdListIndexChoice) < 0)
		return (-1);


	chosenMUIndex = eligibleMotorUnitList[*rTreeIdListIndexChoice] - 1;
	(*totalDistanceToChosenCentroid) = calculateDistanceToFibre(
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					chosenMUIndex
			);

	return chosenMUIndex;
}


int
fibreLayoutProbability_WeightByDistance(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float *distance;
	float distanceSumSquared;
	float curMissingFraction;
	float totalMissingFraction;
	float sumMissingFractions;

	int chosenMUIndex = (-1);
	double choice;
	int i;


	distance = (float *) ckalloc(sizeof(float) * numEligibleMotorUnits);
	/** calculate all the distances */
	distanceSumSquared = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		distance[i] = calculateDistanceToFibre(
						MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
						eligibleMotorUnitList[i] - 1);
		distanceSumSquared += (distance[i] * distance[i]);
	}

	totalMissingFraction = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction = (float)
			(( 1.0 - ( SQR(distance[i]) / distanceSumSquared ) )
			/ (
					numEligibleMotorUnits
			));

		totalMissingFraction += curMissingFraction;
	}


	// MSG_ASSERT(totalMissingFraction == 1, "distance normalization bad");

	/** make a choice, [0,1] */
	choice = localRandomDouble();

	/** now accumulate until covering the choice */
	(*rTreeIdListIndexChoice) = (-1);
	sumMissingFractions = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curMissingFraction =
			( 1 - ( SQR(distance[i]) / distanceSumSquared ) )
			/ (
					numEligibleMotorUnits - 1
			);

		sumMissingFractions += curMissingFraction;
		if (sumMissingFractions >= choice)
		{
			(*rTreeIdListIndexChoice) = i;
			break;
		}
	}


	/**
	 * sanity check -- if roundoff caused us to not
	 * find a fibre, then simply jump back to the top
	 * of the loop and re-choose
	 */
	if ((*rTreeIdListIndexChoice) < 0)
		goto CLEANUP;


	chosenMUIndex = eligibleMotorUnitList[*rTreeIdListIndexChoice] - 1;
	(*totalDistanceToChosenCentroid) = distance[*rTreeIdListIndexChoice];
	ckfree(distance);

CLEANUP:
	return chosenMUIndex;
}


inline static float
calcProb_WeightByMissingOverExpected(
		MuscleData *MD,
		int muIndex
	)
{
	return
		( MD->motorUnit_[ muIndex ]->mu_expectedNumFibres_
				- MD->motorUnit_[ muIndex ]->mu_nFibres_ )
		/ (float) MD->motorUnit_[ muIndex ]->mu_expectedNumFibres_;
}

int
fibreLayoutProbability_WeightByMissingOverExpected(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	float sumM;
	float curProbability;
	float totalProbability;
	float sumProbabilities;

	int chosenMUIndex = (-1);
	double choice;
	int i;


	/** calculate all the distances */
	sumM = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		sumM += calcProb_WeightByMissingOverExpected(
						MD, eligibleMotorUnitList[i] - 1
					);
	}

	/** calculate the total probability */
	totalProbability = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		totalProbability +=
					calcProb_WeightByMissingOverExpected(
						MD, eligibleMotorUnitList[i] - 1
					) / sumM;
	}


//    MSG_ASSERT(fabs(1.0 - totalProbability) < FLT_MIN,
//					"missing/expected normalization bad");



	/** make a choice, [0,1] */
	choice = localRandomDouble();


	/** now accumulate until covering the choice */
	(*rTreeIdListIndexChoice) = (-1);
	sumProbabilities = 0;
	for (i = 0; i < numEligibleMotorUnits; i++)
	{
		curProbability = calcProb_WeightByMissingOverExpected(
						MD, eligibleMotorUnitList[i] - 1
					);

		sumProbabilities += curProbability;
		if ((sumProbabilities / totalProbability) >= choice)
		{
			(*rTreeIdListIndexChoice) = i;
			break;
		}
	}


	/**
	 * sanity check -- if roundoff caused us to not
	 * find a fibre, then simply jump back to the top
	 * of the loop and re-choose
	 */
	if ((*rTreeIdListIndexChoice) < 0)
		goto CLEANUP;


	chosenMUIndex = eligibleMotorUnitList[*rTreeIdListIndexChoice] - 1;
	(*totalDistanceToChosenCentroid) = calculateDistanceToFibre(
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					chosenMUIndex
			);

CLEANUP:
	return chosenMUIndex;
}


typedef struct LayoutProbabilityFunctionData
{
	const char *name_;
	const char *description_;
	MFL_ProbabilityFunction *function_;
	float defaultProb_;
} LayoutProbabilityFunctionData;

static int sNumberOfProbabilityLayoutFunctions = (-1);
static LayoutProbabilityFunctionData sProbabilityFunctionDefinitions[] = {
		{
			"Uniform",
			"Choose an MU randomly among those covering the fibre",
			fibreLayoutProbability_Uniform,
			0.1f
		},
		{
			"Weight By Number of Fibres",
			"(1 - (n_j)/(totalFibres)) / (#MU - 1)",
			fibreLayoutProbability_WeightByNumFibres,
			0.2f
		},
		{
			"Weight By Missing/Expected",
			"P_M_j = M_j / Sum_i=i^k M_i; M_i = #missing/#expected",
			fibreLayoutProbability_WeightByMissingOverExpected,
			0.2f
		},
		{
			"Weight By Distance",
			"Be more likely to adopt nearby MU's",
			fibreLayoutProbability_WeightByDistance,
			(-1)
		},
		{
			"Weight By % Missing Fibres",
			"Weight by # of \"lacking\" fibres normalized by size",
			fibreLayoutProbability_WeightByPercentExpected,
			0.0f
		},
		{
			"Need N Fibres",
			"Weight MU choice by number of \"lacking\" fibres",
			fibreLayoutProbability_NeedNFibres,
			0.0
		},
		{
			NULL,
			NULL,
			NULL,
			(-1)
		}
	};


OS_EXPORT int
getNumFibreLayoutProbabilityFunctions()
{
	if (sNumberOfProbabilityLayoutFunctions < 0)
	{
		double totalProb = 0;
		int i, fillInProbIndex = (-1);

		for (i = 0; sProbabilityFunctionDefinitions[i].name_ != NULL; i++)
		{
			if (sProbabilityFunctionDefinitions[i].defaultProb_ < 0)
			{
				MSG_ASSERT(fillInProbIndex == (-1),
						"Internal error filling in probs for layout funcs");
				fillInProbIndex = i;
			} else
			{
				totalProb += sProbabilityFunctionDefinitions[i].defaultProb_;
			}
		}

		sNumberOfProbabilityLayoutFunctions = i;

		if (fillInProbIndex >= 0)
		{
			sProbabilityFunctionDefinitions[fillInProbIndex].defaultProb_ =
						(float) (1.0 - totalProb);
			totalProb += sProbabilityFunctionDefinitions[fillInProbIndex].defaultProb_;
		}
		MSG_ASSERT(fabs(totalProb - 1.0) < 0.00001,
						"Probability in layout functions bad");
	}

	return sNumberOfProbabilityLayoutFunctions;
}


OS_EXPORT MFL_ProbabilityFunction *
getFibreLayoutProbabilityFunction(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutProbabilityFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sProbabilityFunctionDefinitions[index].function_;
}

OS_EXPORT float
getFibreLayoutProbabilityFunctionProbability(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutProbabilityFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sProbabilityFunctionDefinitions[index].defaultProb_;
}

OS_EXPORT const char *
getFibreLayoutProbabilityFunctionDescription(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutProbabilityFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sProbabilityFunctionDefinitions[index].description_;
}

OS_EXPORT const char *
getFibreLayoutProbabilityFunctionName(int index)
{
	int maxFunction;

	maxFunction = getNumFibreLayoutProbabilityFunctions();
	MSG_ASSERT(index >= 0 && index < maxFunction, "index out of range");

	return sProbabilityFunctionDefinitions[index].name_;
}

static int
performProbabilityFibreLayout(
		float *totalDistanceToChosenCentroid,
		MuscleData *MD,
		int nLayoutFunctions,
		MFL_ProbabilityFunction **functionList,
		float *functionProbability,
		float xLocationInMM,
		float yLocationInMM,
		int *rTreeIdListIndexChoice,
		int numEligibleMotorUnits,
		int *eligibleMotorUnitList
	)
{
	double layoutChoice;
	double sumProbability = 0;
	int functionIndex;

	/* pick a motor unit randomly from the list */
	layoutChoice = localRandomDouble();
	for (functionIndex = 0; functionIndex < nLayoutFunctions; functionIndex++)
	{
		sumProbability += functionProbability[functionIndex];
		if (sumProbability >= layoutChoice)
		{
			return (*functionList[functionIndex])(
					totalDistanceToChosenCentroid,
					MD, xLocationInMM, yLocationInMM,
					rTreeIdListIndexChoice,
					numEligibleMotorUnits, eligibleMotorUnitList
				);
		}
	}

	/**
	 * if we didn't find anything in the loop for some reason, return (-1);
	 */
	return -1;
}



static int
chooseMUForFibreUsingProbabilityMethod(
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		MuscleData *MD,
		int numFibreLayoutProbabilityFunctions,
		MFL_ProbabilityFunction **layoutFunctionList,
		float *layoutFunctionProbabilities,
		int excludeMU
	)
{
	MotorUnit *currentMU;
	double xDiff, yDiff, distance;
	float totalDistToCentroidInMM;
	int numEligibleMotorUnits;
	int *eligibleMotorUnitList;
	int rTreeIdListIndexChoice;
	int deleteMU;
	int candidateMUIndex;
	int chosenMUIndex;
	int i;



	eligibleMotorUnitList = (int *)
				ckalloc(MD->getNumMotorUnits() * sizeof(int));
	numEligibleMotorUnits = 0;
	for (i = 0; i < MD->getNumMotorUnits(); i++)
	{
		currentMU = MD->getMotorUnitFromMasterList(i);
		if (currentMU == NULL)
		{
			continue;
		}
		xDiff = xLocationOfFibreInMM - currentMU->getXLocationInMM();
		yDiff = yLocationOfFibreInMM - currentMU->getYLocationInMM();
		distance = sqrt(SQR(xDiff) + SQR(yDiff));
		if (distance <= currentMU->getDiameter() * (1.25 / 2.0))
		{
			eligibleMotorUnitList[numEligibleMotorUnits++] = currentMU->getID();
		}
	}


	/** if we found nothing, just return */
	if (numEligibleMotorUnits == 0)
	{
		ckfree(eligibleMotorUnitList);
		return (-1);
	}


	/**
	 * Iterate over the list in the R-tree until we find
	 * a unit, or the list is exhausted.
	 *
	 * If we find a unit, we break to the end of this loop
	 * with "chosenMUIndex" set.
	 */
	chosenMUIndex = (-1);
	while ( numEligibleMotorUnits > 0 )
	{

		deleteMU = 0;

		candidateMUIndex = performProbabilityFibreLayout(
						&totalDistToCentroidInMM,
						MD,
						numFibreLayoutProbabilityFunctions,
						layoutFunctionList,
						layoutFunctionProbabilities,
						xLocationOfFibreInMM, yLocationOfFibreInMM,
						&rTreeIdListIndexChoice,
						numEligibleMotorUnits, eligibleMotorUnitList
					);

		if ((candidateMUIndex < 0) || (rTreeIdListIndexChoice < 0))
		{
			/** force to uniform */
			candidateMUIndex = fibreLayoutProbability_Uniform(
							&totalDistToCentroidInMM,
						MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
						&rTreeIdListIndexChoice,
						numEligibleMotorUnits, eligibleMotorUnitList
					);
			if ((candidateMUIndex < 0) || (rTreeIdListIndexChoice < 0))
			{
				LogCrit(
				"Uniform choice failed to find any motor units for fibre\n");
				goto CLEANUP;
			}
		}


		if (candidateMUIndex == excludeMU)
		{
			deleteMU = 1;
		}


		/**
		 * Is the fibre location within the MU territory?
		 */
		if ( (!deleteMU) && (totalDistToCentroidInMM <=
				MD->motorUnit_[candidateMUIndex]->mu_diameter_mm_ / 2))
		{

			chosenMUIndex = candidateMUIndex;
			goto CLEANUP;

		} else
		{
			deleteMU = 1;
		}

		if (deleteMU)
		{

			/**
			 * shrink the list by one, so we know when to quit!
			 */
			MSG_ASSERT(rTreeIdListIndexChoice >= 0,
							"rTreeIdListIndexChoice < 0");
			for (i = rTreeIdListIndexChoice + 1; i < numEligibleMotorUnits; i++)
			{
				eligibleMotorUnitList[i-1] = eligibleMotorUnitList[i];
			}
			numEligibleMotorUnits--;
		}
	}

CLEANUP:
	ckfree(eligibleMotorUnitList);
	return chosenMUIndex;
}


static int
chooseMUForFibreUsingWeightingMethod(
		float xLocationOfFibreInMM,
		float yLocationOfFibreInMM,
		MuscleData *MD,
		int numFibreLayoutWeightingFunctions,
		MFL_WeightingFunction **layoutFunctionList,
		MFL_WeightingDenominatorFunction **denominatorFunctionList,
		float *layoutFunctionWeightings,
		int excludeMU,
		float weightingLayoutNoiseFactor
	)
{
	MotorUnit *currentMU;
	float totalDistToCentroidInMM;
	float curMUWeight, bestMUWeight;
	float randomWeightFactor;
	int numEligibleMotorUnits;
	int *eligibleMotorUnitList;
	int chosenMUIndex = (-1);
	double denominatorValues[8];
	double xDiff, yDiff, distance;
	int i;

	MSG_ASSERT(numFibreLayoutWeightingFunctions < 8,
					"Too many function for internal buffer");


	eligibleMotorUnitList = (int *)
				ckalloc(MD->getNumMotorUnits() * sizeof(int));
	numEligibleMotorUnits = 0;
	for (i = 0; i < MD->getNumMotorUnits(); i++)
	{
		currentMU = MD->getMotorUnitFromMasterList(i);
		if (currentMU == NULL)
		{
			continue;
		}
		xDiff = xLocationOfFibreInMM - currentMU->getXLocationInMM();
		yDiff = yLocationOfFibreInMM - currentMU->getYLocationInMM();
		distance = sqrt(SQR(xDiff) + SQR(yDiff));
		if (distance <= currentMU->getDiameter() * (1.25 / 2.0))
		{
			eligibleMotorUnitList[numEligibleMotorUnits++] = currentMU->getID();
		}
	}


	/** if we found nothing, just return */
	if (numEligibleMotorUnits == 0)
	{
		ckfree(eligibleMotorUnitList);
		return (-1);
	}


	/**
	 * build up the list of all the denominators required
	 * by the layout function calculations
	 */
	for (i = 0; i < numFibreLayoutWeightingFunctions; i++)
	{
		if (layoutFunctionWeightings[i] > 0)
		{
			denominatorValues[i] = (*denominatorFunctionList[i])(
					MD, xLocationOfFibreInMM, yLocationOfFibreInMM,
					numEligibleMotorUnits, eligibleMotorUnitList
				);
		} else
		{
			denominatorValues[i] = 0;
		}
	}

	/**
	 * Iterate over the list in the R-tree assigning
	 * weights for each one, then take the one with
	 * the highest weight.  If a unit is invalid,
	 * it will be assigned the weight of (-1)
	 */
	bestMUWeight = (-1);
	for (i = 0; i < numEligibleMotorUnits; i++)
	{

		if (eligibleMotorUnitList[i] != excludeMU)
		{
			curMUWeight = (float) calculateLayoutWeighting(
							&totalDistToCentroidInMM,
							MD,
							numFibreLayoutWeightingFunctions,
							layoutFunctionList,
							layoutFunctionWeightings,
							denominatorValues,
							xLocationOfFibreInMM, yLocationOfFibreInMM,
							eligibleMotorUnitList[i],
							numEligibleMotorUnits,
							eligibleMotorUnitList
						);
			if (curMUWeight >= 0)
			{
				if (weightingLayoutNoiseFactor > 0)
				{
					randomWeightFactor =
							floatSignedRangeRandom(weightingLayoutNoiseFactor)
							+ 1.0f;
					curMUWeight = curMUWeight * randomWeightFactor;
				}
				if (curMUWeight > bestMUWeight)
				{
					bestMUWeight = curMUWeight;
					chosenMUIndex = eligibleMotorUnitList[i] - 1;
				}
			}
		}
	}

//    MD->validate();

	if ((bestMUWeight < 0) || (chosenMUIndex < 0))
	{
		LogInfo(
		"        No MU found for fibre with %d MU intersections -- weight %f\n",
						numEligibleMotorUnits, bestMUWeight);
		return (-1);
	}

//    LogInfo("        Choosing MU %d with weighting %f\n",
//					chosenMUIndex, bestMUWeight);

	ckfree(eligibleMotorUnitList);
	return chosenMUIndex;
}


/**
 * Add the fibres to a new R-tree so we can find them
 * by distance
 */
int
buildFibreRTree(MuscleData *MD)
{
	Rect bbox;
	MuscleFibre *currentFibre;
	int i;

	if (MD->fibreRTreeRoot_ != NULL)
		return 1;

	//LogInfo("Building Fibre R-Tree\n");
	MD->fibreRTreeRoot_ = RTreeNewIndex();
	for (i = 0; i < MD->getTotalNumberOfFibres(); i++)
	{

		//if (i > 0 && i % 1000 == 0)
		//{
		//	LogInfo(" - fibre %d of %d\n", i, MD->getTotalNumberOfFibres());
		//}

		currentFibre = MD->getFibre(i);
		if (currentFibre == NULL)
		    continue;

		bbox.boundary[0] = currentFibre->getXCell() - (float) 0.5;
		bbox.boundary[1] = currentFibre->getYCell() - (float) 0.5;
		bbox.boundary[2] = currentFibre->getXCell() + (float) 0.5;
		bbox.boundary[3] = currentFibre->getYCell() + (float) 0.5;

		/** we must offset the ids by 1 as 0 is an invalid id */
		RTreeInsertRect(&bbox,
			   i + 1,
			   &MD->fibreRTreeRoot_, 0);
	}
	//LogInfo("Done R-Tree\n");

	return 1;
}

/**
 * Search the nearby fibres for active fibres; put the
 * needle in the centroid of the three closest active
 * fibres from different motor units
 */
int
seekNeedleToNearbyFibres(
		MuscleData *MD,
		float minMetricThreshold
	)
{
	struct rTreeResultList rtreeResults;
	Rect searchRect;
	int numEligibleFibres;
	MuscleFibre *currentFibre;
	MotorUnit *currentMU;
	BITSTRING activeMUBitstring;
	MuscleFibre *closestFibres[3] = { NULL, NULL, NULL };
	double closestFibreMetric[3];
	double xTipInCells, yTipInCells;
	double distanceInCells;
	double distanceInUM;
	double metric;
	double locationXInMM, locationYInMM;
	double yInMMOfHighestFibre;
	double tmpY;
	int highestFibreIndex;
	int fibreMUId;
	int handledFibre;
	int numFibreFound;
	int numCandidatesFound = 0;
	int i, j;

	const double MAX_NEEDLE_MOVEMENT_IN_UM = 800;
	const double MIN_METRIC_THRESHOLD = 4;

	if ( ! buildFibreRTree(MD) )
		return 0;

	LogInfo("Seeking needle to active fibres\n");

	xTipInCells = (MD->needle_->getXTipInMM() * CELLS_PER_MM);
	yTipInCells = (MD->needle_->getYTipInMM() * CELLS_PER_MM);

	/** move up to MAX_NEEDLE_MOVEMENT_IN_UM in any direction */
	searchRect.boundary[0] = (float) (xTipInCells
				- ((MAX_NEEDLE_MOVEMENT_IN_UM / 1000.0) * CELLS_PER_MM));
	searchRect.boundary[1] = (float) (yTipInCells
				- ((MAX_NEEDLE_MOVEMENT_IN_UM / 1000.0) * CELLS_PER_MM));
	searchRect.boundary[2] = (float) (xTipInCells
				+ ((MAX_NEEDLE_MOVEMENT_IN_UM / 1000.0) * CELLS_PER_MM));

	/** in Y, do not search above the needle tip */
	searchRect.boundary[3] = (float) (yTipInCells);


	/** use the R-Tree to get list of possible MU's */
	memset(&rtreeResults, 0, sizeof(rtreeResults));
	numEligibleFibres = RTreeSearch(
				MD->fibreRTreeRoot_,
				&searchRect,
				FibreRTreeSearchCallback__,
				(void *) &rtreeResults
			);

	/**
	 * Record which MU's are active
	 */
	activeMUBitstring = ALLOC_BITSTRING(MD->getNumMotorUnits() + 1);
	ZERO_BITSTRING(activeMUBitstring, MD->getNumMotorUnits() + 1);
	for (i = 0; i < MD->getNumActiveMotorUnits(); i++)
	{
		currentMU = MD->getActiveMotorUnit(i);
		if (currentMU != NULL)
		{
			SET_BIT(activeMUBitstring, currentMU->getID(), 1);
		}
	}

	/**
	 * search through this list for the closest fibres
	 * from active MU's
	 */
	LogInfo("Searching for active fibres near needle tip\n");
	for (i = 0; i < numEligibleFibres; i++)
	{

		currentFibre = MD->getFibre(rtreeResults.results_[i]);
		if (currentFibre == NULL)
			continue;

		fibreMUId = currentFibre->getMotorUnit();

		/** check that we got an active one */
		if (GET_BIT(activeMUBitstring, fibreMUId) == 0)
			continue;

		distanceInCells = CARTESIAN_DISTANCE(
						xTipInCells, yTipInCells,
						currentFibre->getXCell(),
						currentFibre->getYCell()
					);
		distanceInUM = (distanceInCells / CELLS_PER_MM) * 1000.0;

		/** don't consider fibres which are "too close" */
		if (distanceInUM < 50.0)
			continue;

		numCandidatesFound++;

		/** calculate metric : r / d */
		metric = (currentFibre->getDiameter() / 2.0) / distanceInUM;

		handledFibre = 0;
		for (j = 0; j < 3; j++)
		{

			/** if we see a blank, save this fibre */
			if (closestFibres[j] == NULL)
			{
				closestFibres[j] = currentFibre;
				closestFibreMetric[j] = metric;
				handledFibre = 1;
				break;
			}

			/**
			 * if we see the MU we belong to, overwrite
			 * if we are closer
			 */
			if (closestFibres[j]->getMotorUnit() == fibreMUId)
			{
				/**
				 * if we see this MU, we have "handled"
				 * the fibre, regardless whether we
				 * keep it
				 */
				handledFibre = 1;
				if (metric < closestFibreMetric[j])
				{
					closestFibres[j] = currentFibre;
					closestFibreMetric[j] = metric;
				}
				break;
			}
		}

		/**
		 * if we haven't "handled" the fibre, it may still
		 * be closer than one we have from a different MU
		 */
		if (handledFibre == 0)
		{
			for (j = 0; j < 3; j++)
			{
				if (metric < closestFibreMetric[j])
				{
					closestFibres[j] = currentFibre;
					closestFibreMetric[j] = metric;
				}
			}
		}
	}

	/**
	 * Now we have three fibres which are active and close
	 */
	numFibreFound = 0;
	j = 0;
	while ((j < 3) && (closestFibres[j] != NULL))
	{
		LogInfo("   Best nearby fibre[%d] : MU %3d, metric %f\n",
				j,
				closestFibres[j]->getMotorUnit(),
				closestFibreMetric[j]);
		numFibreFound++;
		j++;
	}

	LogInfo("Searched %d active fibres within %s um; "
							"seeking needle to \"best %d\"\n",
				numCandidatesFound,
				niceDouble(MAX_NEEDLE_MOVEMENT_IN_UM),
				numFibreFound);

	/** find the fibre with the greatest Y */
	if (numFibreFound == 0)
	{
		LogCrit("No fibre found to seek to -- aborting study\n");
		goto CLEANUP; //return 0;
	}


	yInMMOfHighestFibre = closestFibres[0]->getYCell() / CELLS_PER_MM;
	highestFibreIndex = 0;
	for (j = 1; j < numFibreFound; j++)
	{
		tmpY = closestFibres[j]->getYCell() / CELLS_PER_MM;
		if (yInMMOfHighestFibre < tmpY)
		{
			yInMMOfHighestFibre = tmpY;
			highestFibreIndex = j;
		}
	}


	/** if our metric is too low, then move to just one of them */
	if (numFibreFound > 1 &&
					closestFibreMetric[highestFibreIndex] < minMetricThreshold)
	{
		numFibreFound = 1;
	}

	/**
	 * figure out how to locate the needle
	 */
	switch (numFibreFound)
	{
		case 0:
			LogInfo(
				"No active fibres found near needle -- not moving needle\n");
			goto CLEANUP;
			OS_BREAK;

		case 1:
			LogInfo(
				"Only 1 active fibre found near needle -- "
						"moving needle to fibre\n");
			locationXInMM =  closestFibres[0]->getXCell() / CELLS_PER_MM;
			locationYInMM = (closestFibres[0]->getYCell() / CELLS_PER_MM) + 0.05;
			break;


		case 2:
			LogInfo(
				"Only 2 active fibres found near needle -- "
						"moving to midpoint\n");
			locationXInMM =
						((closestFibres[0]->getXCell() / CELLS_PER_MM)
						+(closestFibres[1]->getXCell() / CELLS_PER_MM)) / 2.0;
			locationYInMM = yInMMOfHighestFibre + 0.05;
			break;


		case 3:
			LogInfo(
				"Moving needle to midpoint of 3 fibres\n");
			locationXInMM =
						((closestFibres[0]->getXCell() / CELLS_PER_MM)
						+(closestFibres[1]->getXCell() / CELLS_PER_MM)
						+(closestFibres[2]->getXCell() / CELLS_PER_MM)) / 3.0;
			locationYInMM = yInMMOfHighestFibre + 0.05;

			/** now calculate the metric */
			metric = 0;
			for (j = 0; j < 3; j++)
			{

				distanceInCells = CARTESIAN_DISTANCE(
						locationXInMM * CELLS_PER_MM,
						locationYInMM * CELLS_PER_MM,
						closestFibres[j]->getXCell(),
						closestFibres[j]->getYCell()
					);
				distanceInUM = (distanceInCells / CELLS_PER_MM) * 1000.0;

				metric += (distanceInUM /
						(closestFibres[j]->getDiameter() / 2.0f));
			}
			metric /= 3.0f;

			/**
			 * if that didn't make us happy, move to above the "highest"
			 * fibre
			 */
			if (metric > MIN_METRIC_THRESHOLD)
			{
				LogInfo("Threshold failed!  %f > %f\n",
									metric, MIN_METRIC_THRESHOLD);
				locationXInMM =
						(closestFibres[highestFibreIndex]->getXCell() / CELLS_PER_MM)
						+ 0.05;
			}
			break;

		default:
			MSG_FAIL("Bad case in switch");
			OS_BREAK;
	}

	/**
	 * Move the needle after reporting how far we are moving it
	 */
	distanceInUM = CARTESIAN_DISTANCE(
					MD->needle_->getXTipInMM(),
					MD->needle_->getYTipInMM(),
					locationXInMM, locationYInMM
				);

	LogInfo("Moving needle %f mm towards active fibre\n", distanceInUM);

	MD->needle_->set(
					(float) locationXInMM, (float) locationYInMM,
				MD->needle_->getZInMM(),
				MD->needle_->getCannulaLengthInMM(),
				MD->needle_->getRadiusInMicrons(),
				MD->needle_->getSlope()
			);


CLEANUP:
	if (rtreeResults.results_ != NULL)
		ckfree(rtreeResults.results_);
	FREE_BITSTRING(activeMUBitstring);
	return 1;
}

OS_EXPORT int
plowMuscleFibres(
		int emgFileId,
		const char *outputDir,
		MuscleData *MD,
		float canPhysicalRadiusInUM
	)
{
	float xLocationOfFibreInCells, yLocationOfFibreInCells;
	float xLocationOfFibreInMM, yLocationOfFibreInMM;
	double xRightTipSide;
	double B_cannulaPerpindicular;
	double B_cannula;
	double xIntersection, yIntersection;
	double xDifference, yDifference;
	double M_cannulaPerpindicular;
	MuscleFibre *fibre;
	float distance;
	int totalFibres;
	int i;

	totalFibres = MD->getTotalNumberOfFibres();

	MD->needle_->getNeedleLocations(
					&xRightTipSide, NULL,
				NULL, NULL,
				NeedleInfo::AboveTip_BelowCannula);

	for (i = 0; i < totalFibres; i++)
	{

		fibre = MD->getFibre(i);
		if (fibre == NULL)
			continue;

		xLocationOfFibreInMM = fibre->getXCell() / CELLS_PER_MM;
		yLocationOfFibreInMM = fibre->getYCell() / CELLS_PER_MM;
		xLocationOfFibreInCells = fibre->getXCell();
		yLocationOfFibreInCells = fibre->getYCell();


		/**
		 * if we are below the tip, we are fine
		 * where we are
		 */
		if (yLocationOfFibreInMM < MD->getNeedleInfo()->getYTipInMM())
		{
			continue;
		}


		/**
		 * ---------------------------------------------------
		 * Calculate whether we are inside the cannula or not.
		 * ---------------------------------------------------
		 */

		/**
		 * raise a perpinducular to the cannula perpindicular
		 * is simply 1/slope
		 */
		M_cannulaPerpindicular = (-1.0 / MD->needle_->getSlope());


		/**
		 * calculate where this perpindiucular intersects the
		 * y axis if the line goes through the fibre of interest.
		 * B = y - Mx
		 */
		B_cannulaPerpindicular = yLocationOfFibreInMM -
					M_cannulaPerpindicular * xLocationOfFibreInMM;


		/**
		 * Figure out where B for the cannula itself is, based
		 * on the tip and the slope
		 */
		B_cannula = MD->needle_->getYTipInMM() -
							(MD->needle_->getSlope() * MD->needle_->getXTipInMM());

		/**
		 * Now figure out where this line and the center of
		 * the cannula intersect; we then can determine whether
		 * we are within one radius of the center of the cannula
		 *
		 * y = MC x + BC  <-> equation of cannula
		 * y = MP x + BP  <-> equation of perpindicular
		 *
		 * MC x + BC  =  MP x + BP
		 * MC x - MP x = BP - BC
		 * x (MC - MP) = BP - BC
		 *
		 * .'.
		 * x = (BP - BC) / (MC - MP)
		 */
		xIntersection =
					(B_cannulaPerpindicular - B_cannula)
				/ (MD->needle_->getSlope() - M_cannulaPerpindicular);
		yIntersection = M_cannulaPerpindicular * xIntersection
				+ B_cannulaPerpindicular;

		xDifference = xLocationOfFibreInMM - xIntersection;
		yDifference = yLocationOfFibreInMM - yIntersection;


		/**
		 * Calculate distance and convert to um (the unit in which
		 * the physical and model distances are specified)
		 */
		distance = (float)
					(sqrt(SQR(xDifference) + SQR(yDifference)) * 1000.0);


		/** if we are inside the cannula, relocate (plow) the fibre */
		if (distance <= canPhysicalRadiusInUM)
		{

			/**
			 * if we are above the needle, we will project
			 * along the cannula perpindicular
			 */
			if (yDifference > 0)
			{
				xLocationOfFibreInCells = (float)
					((xIntersection -
						(canPhysicalRadiusInUM *
							fabs(cos(atan(M_cannulaPerpindicular)))
						)
					) * CELLS_PER_MM);

				yLocationOfFibreInCells = (float)
					((yIntersection -
						(canPhysicalRadiusInUM *
							fabs(sin(atan(M_cannulaPerpindicular)))
						)
					) * CELLS_PER_MM);

			} else
			{
				/**
				 * we need to see if we are near the tip or not
				 */
				if (xIntersection < xRightTipSide)
				{
					yLocationOfFibreInCells = (float)
							((MD->getNeedleInfo()->getYTipInMM()
								   		- fibre->getDiameter()) / 2.0);

				} else
				{
					/** simply project right/down */
					xLocationOfFibreInCells = (float)
						((xIntersection +
							(canPhysicalRadiusInUM *
								fabs(cos(atan(M_cannulaPerpindicular)))
							)
						) * CELLS_PER_MM);
					yLocationOfFibreInCells = (float)
						((yIntersection +
							(canPhysicalRadiusInUM *
								fabs(sin(atan(M_cannulaPerpindicular)))
							)
						) * CELLS_PER_MM);
				}
			}

			/**
			 * Now update the fibre location
			 */
			fibre->setCellLocation(
					xLocationOfFibreInCells,
					yLocationOfFibreInCells
				);
		}
	}

	/** write out contraction-specific fibre locations */
	{
		char tmpFilename[FILENAME_MAX];
		slnprintf(tmpFilename, FILENAME_MAX,
				"%s\\MF-plowed%d.dat",
				outputDir,
				emgFileId);
		MD->writeMFInfo(tmpFilename);
	}

#ifdef  DEBUG_MAHDIEH_NEEDLE_INFO
	/**  write out needle-related information about the MUs */
	{
		char tmpFilenameM[FILENAME_MAX];
		slnprintf(tmpFilenameM, FILENAME_MAX,
				"%s\\NeedleRelated-MUInfo%d.dat",
				outputDir,
				emgFileId);
		MD->writeNeedleRelatedMUInfo(tmpFilenameM);
	}
#endif

	return 1;
}


#ifdef  DEBUG_LAYOUT
static int
plotMeanFibreDiameters(
		const char *muscleDirectory,
		int numMUs,
		double *meanDiameters
	)
{
	GSimplePlot *plot;
	char scriptname[FILENAME_MAX];
	char outputname[FILENAME_MAX];
	int i;


	slnprintf(scriptname, FILENAME_MAX,
			"%s/plots/muMeanPoints", muscleDirectory);
	slnprintf(outputname, FILENAME_MAX,
			"muMeanPoints");

	plot = gPointPltCreate(
			scriptname,
			outputname,
			"Motor Unit Mean Fibre Diameter By MU Index",
			"Motor Unit Index",
			"Motor Unit Mean Fibre Diameter ({/Symbol m}m)");

	gPointPltAddSeries(plot,
			1, NULL, "muMeanPoints.txt");

	for (i = 0; i < numMUs; i++)
	{
		gPointPltAddPointToSeries(
			plot, 1,
			i, meanDiameters[i]);
	}

	gPointPltClose(plot);

	return 1;
}


static int
plotFibreDiameterHistogram(
		MuscleData *MD,
		const char *muscleDirectory
	)
{
	GSimplePlot *plot;
	MuscleFibre *currentFibre;
	char scriptname[FILENAME_MAX];
	char outputname[FILENAME_MAX];
	int *counts, index, maxCount = 0;
	double range, fibreMinDiameter, fibreMaxDiameter;
	double binStep, d;
	double mean;
	int nBins = 72;
	char *shadowFunction;
	int i;


	shadowFunction = "s * exp((- (x - mu)*(x-mu) ) / (2 * sigma * sigma))";


	fibreMinDiameter = FIBRE_DIAMETER_MEAN - (3.0 * FIBRE_DIAMETER_STD_DEV);
	fibreMaxDiameter = FIBRE_DIAMETER_MEAN + (5.0 * FIBRE_DIAMETER_STD_DEV);

	range = fibreMaxDiameter;
	binStep = (range / (double) nBins);

	counts = (int *) ckalloc(nBins * sizeof(int));
	memset(counts, 0, nBins * sizeof(int));

	slnprintf(scriptname, FILENAME_MAX,
					"%s/plots/createFibreDiameterHist",
					muscleDirectory);
	slnprintf(outputname, FILENAME_MAX, "fibreDiameterHist");


	mean = 0.0;
	for (i = 1; i < MD->getTotalNumberOfFibres(); i++)
	{
		currentFibre = MD->getFibre(i);
		d = currentFibre->getDiameter();
		index = (int) (d / binStep);
		if (index >= 0 && index < nBins)
		{
			counts[index]++;
			if (maxCount < counts[index])
				maxCount = counts[index];
		}
		mean += d;
	}
	mean = mean / (float) MD->getTotalNumberOfFibres();

	plot = gHistPltCreate(
			scriptname,
			outputname,
			"Muscle Fibre Diameter Histogram",
			"Muscle Fibre Diameter", "Number");

	gHistPltAddSeries(plot,
			1, NULL, "overallFibreDiameterFrequency.txt");

	for (i = 0; i < nBins; i++)
	{
		gHistPltAddPointToSeries(
			plot, 1,
			(i * binStep),
			counts[i]
		);
	}


	gHistPltSetMeanInSeries(plot, 1, mean);
	gPltWriteSetupLine(plot->gfp, "mu = %g\n", mean);
	gPltWriteSetupLine(plot->gfp, "sigma = %g\n", FIBRE_DIAMETER_STD_DEV);
	gPltWriteSetupLine(plot->gfp, "pi = %g\n", M_PI);
	gPltWriteSetupLine(plot->gfp, "s = %d\n", maxCount);
	gSimplePltAddFunctionToPlot(plot, shadowFunction);

	gHistPltClose(plot);
	ckfree(counts);

	return 1;
}
#endif


static int
calculateMUMeanDiameters(
		double *meanDiameters,
		MuscleData *MD,
		const char *muscleDirectory
	)
{
	double MN;
	long numMUs;
	double *estimatedMeanDelta;
	double n_i;
	double fibreDiameterRange;
	double fibreMaxDiameter, fibreMinDiameter;
	int status = 0;
	int i;


	fibreMinDiameter = FIBRE_DIAMETER_MEAN - (1.0 * FIBRE_DIAMETER_STD_DEV);
	fibreMaxDiameter = FIBRE_DIAMETER_MEAN + (1.0 * FIBRE_DIAMETER_STD_DEV);

	numMUs = MD->getNumMotorUnits();
	MN = (FIBRE_DIAMETER_MEAN * MD->getTotalNumberOfFibres());
	LogInfo("MN = %g\n", MN);

	estimatedMeanDelta = (double *) ckalloc(sizeof(double) * numMUs);

	fibreDiameterRange = (fibreMaxDiameter - fibreMinDiameter);

	/**
	 * produce an inital ranking for M based on the relationship
	 * between the fibres in the current MU with the fibres in the
	 * previous MU
	 */
	estimatedMeanDelta[0] = 0;
	for (i = 1; i < numMUs; i++)
	{
		n_i = (double) MD->getMotorUnitFromMasterList(i)->getNumFibres();
		estimatedMeanDelta[i] =
				(n_i  / (double) (MD->getTotalNumberOfFibres()))
					* fibreDiameterRange;
	}


	/**
	 * Create a set of estimated means from the scaled deltas and
	 * the minimum mean
	 */
	meanDiameters[0] = fibreMinDiameter;
	for (i = 1; i < numMUs; i++)
	{
		meanDiameters[i] = meanDiameters[i-1] + estimatedMeanDelta[i];
	}



	LogInfo("Range of means [%f - %f] (desired [%f - %f])\n",
			meanDiameters[0], meanDiameters[numMUs-1],
			fibreMinDiameter, fibreMaxDiameter);


	{
		double checkSum = 0;
		for (i = 0; i < numMUs; i++)
		{
			checkSum +=
					(meanDiameters[i]
						* MD->getMotorUnitFromMasterList(i)->getNumFibres());
		}
		LogInfo("Mean checksum(%g) - MN = %g\n", checkSum,  checkSum - MN);
		LogInfo("Population mean %g - desired mean %g == %g\n",
				checkSum / MD->getTotalNumberOfFibres(),
				(double) FIBRE_DIAMETER_MEAN,
				(checkSum / MD->getTotalNumberOfFibres())
						- (double) FIBRE_DIAMETER_MEAN
			);
	}


#ifdef  DEBUG_LAYOUT
	/** plot the raw data */
	plotMeanFibreDiameters(muscleDirectory, numMUs, meanDiameters);
#endif

	status = 1;

//CLEANUP:
	ckfree(estimatedMeanDelta);
	return status;
}

static int
setFibreDiameterAndShift(
		MuscleData *MD,
		const char *muscleDirectory
	)
{
	MotorUnit *currentMU;
	MuscleFibre *currentFibre;
	double *fibreDiameters = NULL;
	double newDiameter;
	double *meanDiameter = NULL;
	double jShift, jShiftBase;
	double currentFibreStdDev;
	int numMotorUnits;
	//int numFibres;
	int status = 0;
	int nFibresSet = 0;
	int i, j;

	/** get the range of MU sizes */
	/* muSizeRange = MD->maxMotorUnitDiameter_ - MD->minMotorUnitDiameter_; */

	/**
	 * Get the mean fibre diameters for each MU
	 */
	numMotorUnits = MD->getNumMotorUnits();
	meanDiameter = (double *) ckalloc(sizeof(double) * (numMotorUnits));
	MSG_ASSERT(meanDiameter != NULL, "malloc failed");

	LogInfo("Calculating mean fibre diameters for all motor units\n");
	if ( ! calculateMUMeanDiameters(meanDiameter, MD, muscleDirectory) )
	{
		LogError("Cannot calculate mean diameters for MU's\n");
		goto CLEANUP;
	}


	/**
	 * -----------------------------------------------
	 * select motor unit neuromuscular junction
	 * location and diameter
	 *
	 * This data is needed below if we need to relocate
	 * the fibre
	 * -----------------------------------------------
	 *
	 * Originally, this was a uniform distribution over
	 * a 5mm end-plate region for the muscle.  The new
	 * idea is that a single neuron terminus is localized.
	 * so an individual motor unit is driven using a very
	 * localized input.
	 *
	 * We are therefore using a Normal distribution with
	 * 125 um distribution, as in
	 * Stashuk '92 "Sim of EleMyo Signals"
	 *
	 * Question:  But this next line does not take into account
	 * the rest of the MU?  This just gives us a narrow
	 * shift for this entire muscle!
	 * Answer:  This is ok because shifts of whole neurons
	 * simply manifest as a uniform displacement in the
	 * arrival time & so really do not affect our model.
	 *
	 * Think:  There may in future be a desire to account
	 * for motor unit size in this parameter -- we need
	 * to think this carefully though, as there is already
	 * a greater variance simply because more fibres are
	 * chosen
	 */

	LogInfo("Setting up fibre sub-distributions\n");
	/**
	 * Use the means calculated above to set sub-distributions
	 * for fibre diameters with a smaller variance
	 */
	for (i = 0; i < MD->getNumMotorUnits(); i++)
	{
		currentMU = MD->getMotorUnitFromMasterList(i);
		//numFibres = currentMU->getNumFibres();


		/**
		 * calculate the base offset of this fibre set within the NMJ
		 * region.  This will group all the fibres together (having
		 * no effect within the MU), but producing satellites in
		 * neuropathies
		 *
		 * Don't do this for now as we get a new jShift in neuropathy
		 * based on the adoptive NMJ location.
		 *
		 * We therefore need a better model for satellite potentials
		 */
		// jShiftBase = floatNormalizedRandom() * (5.0 - (0.125 * 3));
		jShiftBase = 0;


		MSG_ASSERT(meanDiameter[i] > 0, "zero mean diameter!");
		/**
		 * motor unit std dev = m_i * COV
		 *
		 * COV = muscle sd / muscle mean
		 */
		currentFibreStdDev =
						meanDiameter[i] *
						((FIBRE_DIAMETER_STD_DEV / FIBRE_DIAMETER_MEAN)*0.5);

		for (j = 0; j < currentMU->getNumFibres(); j++)
		{

			currentFibre = currentMU->getFibre(j);
			if (currentFibre == NULL)
				continue;

			/**
			 * produce a local shift on top of the base
			 * grouping for this MU
			 */
			jShift = (gauss01() *
					currentMU->getDiameter() / 16.0 ) + jShiftBase;
//(					currentMU->getDiameter() / 40.0 ) + jShiftBase;
			currentFibre->mf_jShift_ = (float) jShift;

			newDiameter = 0;
			while (newDiameter < 25.0)
			{
				newDiameter = (float)
					(
						(gauss01()* currentFibreStdDev)
							+ meanDiameter[i]
					);
			}

			currentFibre->mf_healthyDiameter_ =
							currentFibre->mf_diameter_ = (float) newDiameter;

/*
			{
				MuscleFibre *fibreFromMuscle;
				int foundFibre = 0;
				int ii;

				for (ii = 0; ii < MD->getTotalNumberOfFibres(); ii++)
				{
					fibreFromMuscle = MD->getFibre(ii);
					if (fibreFromMuscle == currentFibre)
					{
						foundFibre = 1;
						break;
					}
				}
				if (foundFibre == 0)
				{
					LogCrit("Cannot locate current fibre in muscle!\n");
					return 0;
				}
			}
*/

			nFibresSet++;
		}
	}
	if (nFibresSet == 0)
	{
		LogCrit("No fibres found!\n");
	}

	{
		double d;
		double meanFibreDiameter = 0;
		double minFibreDiameter = 0;
		double maxFibreDiameter = 0;

		meanFibreDiameter =
				maxFibreDiameter =
				minFibreDiameter =
						MD->getFibre(0)->getDiameter();
		for (i = 1; i < MD->getTotalNumberOfFibres(); i++)
		{
			currentFibre = MD->getFibre(i);
			if (currentFibre == NULL)
				continue;
			d = currentFibre->getDiameter();
			meanFibreDiameter += d;
			if (minFibreDiameter > d)
				minFibreDiameter = d;
			if (maxFibreDiameter < d)
				maxFibreDiameter = d;
		}
		meanFibreDiameter = meanFibreDiameter / MD->getTotalNumberOfFibres();
		LogInfo("Mean fibre diameter = %f\n", meanFibreDiameter);
		LogInfo(" Min fibre diameter = %f\n", minFibreDiameter);
		LogInfo(" Max fibre diameter = %f\n", maxFibreDiameter);
	}

#ifdef  DEBUG_LAYOUT
    plotFibreDiameterHistogram(MD, muscleDirectory);
#endif

	status = 1;

CLEANUP:
	if (meanDiameter != NULL)
	{
		ckfree(meanDiameter);
	}
	if (fibreDiameters != NULL)
	{
		ckfree(fibreDiameters);
	}

	return status;
}


static int
allocateFibres(
		MuscleData *MD,
		int modelLayoutDistanceInUM
	)
{
	struct report_timer *reportTimer;
	float xLocationOfFibreInMM, yLocationOfFibreInMM;
	float xLocationOfFibreInCells, yLocationOfFibreInCells;
	double squaredDistance;
	MuscleFibre *newFibre;
	int beginCellX, endCellX;
	int beginCellY, endCellY;
	float muscleRadius, maxModelRadius;
	int muscleRadiusInCells;
	int nCellsAssigned = 0;
	time_t startTime;
	int i, j, k, l, ll;


	/**
	 * set up the "grid" based on the size indicated
	 * in the globals structure
	 */
	MD->yDetect_ = (int) ceil((modelLayoutDistanceInUM / 1000.0) * CELLS_PER_MM);
	MD->xDetect_ = (int) ceil(
		        ((modelLayoutDistanceInUM + 300) / 1000.0)
		        * CELLS_PER_MM);

	muscleRadius = (float) (MD->muscleDiameter_ / 2.0);
	muscleRadiusInCells = (int) ceil(muscleRadius * CELLS_PER_MM);

	/**
	 * we used to trim to the getCannulaLength(),
	 * but now we don't want to
	 */
	maxModelRadius = muscleRadius;

	/**
	 * We will scan from the bottom left of the "grid"
	 * all the way up to the top + right of the muscle.
	 *
	 * For each fibre above the needle tip, we will prune
	 * out those fibres which are too far from the cannla
	 * to be important.
	 */
	beginCellY = (-MD->yDetect_);
	endCellY = (int) ((MD->muscleDiameter_ / 2.0) * CELLS_PER_MM);

	beginCellX = (-MD->xDetect_);
	endCellX = (int) ((MD->muscleDiameter_ / 2.0) * CELLS_PER_MM);


	if (beginCellY < -muscleRadiusInCells)
		beginCellY = -muscleRadiusInCells;
	if (endCellY > muscleRadiusInCells)
		endCellY = muscleRadiusInCells;

	if (beginCellX < -muscleRadiusInCells)
		beginCellX = -muscleRadiusInCells;
	if (endCellX > muscleRadiusInCells)
		endCellX = muscleRadiusInCells;

	LogInfo("Allocating fibres from (%3d, %3d) to (%3d, %3d)\n",
		        beginCellX, beginCellY,
		        endCellX, endCellY);


	reportTimer = startReportTimer(
				(endCellX - beginCellX) * (endCellY - beginCellY)
			);
	startTime = time(NULL);

	k = l = ll = 0;
	for (i = beginCellX; i < endCellX; i++)
	{
		for (j = beginCellY; j < endCellY; j++)
		{
			l++;

			/**
			 * Fibres are "kept" if they are within an MU
			 * boundary, and are within the muscle radius.
			 */

		    /** convert locations to mm */
		    xLocationOfFibreInMM = i / CELLS_PER_MM;
		    yLocationOfFibreInMM = j / CELLS_PER_MM;
			xLocationOfFibreInCells = (float) i;
			yLocationOfFibreInCells = (float) j;


		    /**
			 * If the fibre is inside the muscle radius, then
			 * determine whether it is inside of any motor unit
			 * territory.  If either of these is untrue, we
			 * simply forget about this fibre
			 */
			squaredDistance = SQR(xLocationOfFibreInMM)
						+ SQR(yLocationOfFibreInMM);
			if (squaredDistance <= SQR(maxModelRadius))
			{

		        nCellsAssigned++;
		        k = k + 1;


				/**
				 * allocate new fibre, now that we are sure we
				 * will need it
				 */
				newFibre = new MuscleFibre(
									xLocationOfFibreInCells,
									yLocationOfFibreInCells
								);

				/**
				 * Fill in 'dummy' fibre diameter and shift for
				 * now -- these are assigned after we know what
				 * motor unit the fibre is in
				 */
				newFibre->mf_jShift_ = (-1);
				newFibre->mf_healthyDiameter_ = newFibre->mf_diameter_ = (-1);
				MD->addFibre(newFibre);
			}
		}
		if ((l > (ll + 5000)) && ((time(NULL) - startTime) > 5))
		{
			LogInfo("    Fibre position %s\n", reportTime(l, reportTimer));
			ll = l;
		}
	}

	deleteReportTimer(reportTimer);

	LogInfo("Created %d fibres in muscle of %f mm diameter\n",
		        nCellsAssigned, MD->muscleDiameter_);

	if (nCellsAssigned > 0)
		return 1;

	LogCrit("No cells assigned to motor units!\n");
	return 0;
}


static int
assignFibresToMotorUnitsUsingProbabilityMethod(
		const char *muscleDirectory,
		MuscleData *MD,
		int modelLayoutDistanceInUM,
		int numFibreLayoutProbabilityFunctions,
		MFL_ProbabilityFunction **layoutFunctionList,
		float *fibreProbabilties
	)
{
	struct report_timer *reportTimer;
	time_t startTime;
	MuscleFibre *currentFibre;
	int nCellsAssigned = 0;
	int fibreIndex;
	int initialIndex;
	int chosenMUIndex;
	BITSTRING bitstring;
	int i;


#ifdef DEBUG_LAYOUT
	char filename[FILENAME_MAX];
	FILE *tmpfp;
	FILE *needlefp;


	slnprintf(filename, FILENAME_MAX,
			"%s/plots/cell-layout.txt", muscleDirectory);
	tmpfp = fopen(filename, "w");
	slnprintf(filename, FILENAME_MAX,
			"%s/plots/needle-layout.txt", muscleDirectory);
	needlefp = fopen(filename, "w");
	fprintf(needlefp, "%.6f %.6f\n",
		        MD->needle_->getXTipInMM(),
		        MD->needle_->getYTipInMM());

	if (ferror(needlefp))
	{
		LogCrit("Write to debug file '%s' failed -- aborting\n",
		            filename);
		return 0;
	}
	fprintf(needlefp, "%.6f %.6f\n",
		        MD->needle_->getXCannulaTerminusInMM(),
		        MD->needle_->getYCannulaTerminusInMM());
	if (ferror(needlefp))
	{
		LogCrit("Write to debug file '%s' failed -- aborting\n",
		            filename);
		return 0;
	}
	fclose(needlefp);
#endif /* DEBUG_LAYOUT */


	LogInfo("Assigning %d fibres among %d motor units using probability method\n",
					MD->getTotalNumberOfFibres(), MD->getNumMotorUnits());


	bitstring = ALLOC_BITSTRING(MD->getTotalNumberOfFibres());
	ZERO_BITSTRING(bitstring, MD->getTotalNumberOfFibres());

	reportTimer = startReportTimer(MD->getTotalNumberOfFibres());
	startTime = time(NULL);

	for (i = 0; i < MD->getTotalNumberOfFibres(); i++)
	{


//		MD->validate();
		if ((i % 1000 == 0) && ((time(NULL) - startTime) > 10))
		{
			LogInfo("    Fibre %s\n", reportTime(i, reportTimer));
		}

		/**
		 * choose a random initial point, and from there
		 * search the bitstring until we find a fibre which
		 * has not be allocated yet
		 */
		initialIndex = fibreIndex = intRangeRandom(MD->getTotalNumberOfFibres());
		while (GET_BIT(bitstring, fibreIndex))
		{
			fibreIndex = (fibreIndex + 1) %MD->getTotalNumberOfFibres();
			MSG_ASSERT(fibreIndex != initialIndex, "Wrap in fibre search");
		}
		SET_BIT(bitstring, fibreIndex, 1);

		currentFibre = MD->getFibre(fibreIndex);
		if (currentFibre == NULL)
			continue;

//		LogInfo("    Assigning fibre %5d at (%6.1f, %6.1f)\n",
//						fibreIndex,
//						currentFibre->getXCell(),
//						currentFibre->getYCell());



		/**
		 * find an MU for this fibre.  If this is negative
		 * there are no covering MU's (so we just forget
		 * the fibre)
		 */
		chosenMUIndex = chooseMUForFibreUsingProbabilityMethod(
						currentFibre->getXCell() / CELLS_PER_MM,
						currentFibre->getYCell() / CELLS_PER_MM,
						MD,
						numFibreLayoutProbabilityFunctions,
						layoutFunctionList,
						fibreProbabilties,
						(-1)
					);

		if (chosenMUIndex >= 0)
		{

			currentFibre->mf_motorUnit_ =
						MD->motorUnit_[chosenMUIndex]->mu_id_;
			MD->motorUnit_[chosenMUIndex]->addFibre(currentFibre);
			if (MD->motorUnit_[chosenMUIndex]->getNumFibres() >= 25)
				MD->motorUnit_[chosenMUIndex]->recalculateCentroid();
			nCellsAssigned++;
//			MD->validate();

		} else
		{
			/** no motor unit wanted this fibre, so delete it */
			delete MD->masterFibreList_[i];
			MD->masterFibreList_[i] = NULL;
		}
	}

	deleteReportTimer(reportTimer);

	FREE_BITSTRING(bitstring);

#ifdef DEBUG_LAYOUT
	fclose(tmpfp);
#endif

	if (nCellsAssigned > 0)
		return 1;

	LogCrit("No cells assigned to motor units!\n");
	return 0;
}


static int
assignFibresToMotorUnitsUsingWeightingMethod(
		const char *muscleDirectory,
		MuscleData *MD,
		int modelLayoutDistanceInUM,
		int numFibreLayoutWeightingFunctions,
		MFL_WeightingFunction **layoutFunctionList,
		MFL_WeightingDenominatorFunction **denominatorFunctionList,
		float *fibreProbabilties,
		double weightingLayoutNoiseFactor
	)
{
	struct report_timer *reportTimer;
	time_t startTime;
	MuscleFibre *currentFibre;
	int nCellsAssigned = 0;
	int fibreIndex;
	int initialIndex;
	int chosenMUIndex;
	BITSTRING bitstring;
	int i;


#ifdef DEBUG_LAYOUT
	char filename[FILENAME_MAX];
	FILE *tmpfp;
	FILE *needlefp;


	slnprintf(filename, FILENAME_MAX,
			"%s/plots/cell-layout.txt", muscleDirectory);
	tmpfp = fopen(filename, "w");
	slnprintf(filename, FILENAME_MAX,
			"%s/plots/needle-layout.txt", muscleDirectory);
	needlefp = fopen(filename, "w");
	fprintf(needlefp, "%.6f %.6f\n",
		        MD->needle_->getXTipInMM(),
		        MD->needle_->getYTipInMM());

	if (ferror(needlefp))
	{
		LogCrit("Write to debug file '%s' failed -- aborting\n",
		            filename);
		return 0;
	}
	fprintf(needlefp, "%.6f %.6f\n",
		        MD->needle_->getXCannulaTerminusInMM(),
		        MD->needle_->getYCannulaTerminusInMM());
	if (ferror(needlefp))
	{
		LogCrit("Write to debug file '%s' failed -- aborting\n",
		            filename);
		return 0;
	}
	fclose(needlefp);
#endif /* DEBUG_LAYOUT */


	LogInfo("Assigning %d fibres among %d motor units using weighting method\n",
					MD->getTotalNumberOfFibres(), MD->getNumMotorUnits());


	bitstring = ALLOC_BITSTRING(MD->getTotalNumberOfFibres());
	ZERO_BITSTRING(bitstring, MD->getTotalNumberOfFibres());

	reportTimer = startReportTimer(MD->getTotalNumberOfFibres());
	startTime = time(NULL);

	for (i = 0; i < MD->getTotalNumberOfFibres(); i++)
	{


//		MD->validate();
		if ((i % 1000 == 0) && ((time(NULL) - startTime) > 5))
		{
			LogInfo("    Fibre %s\n", reportTime(i, reportTimer));
		}

		/**
		 * choose a random initial point, and from there
		 * search the bitstring until we find a fibre which
		 * has not be allocated yet
		 */
		initialIndex = fibreIndex
					= intRangeRandom(MD->getTotalNumberOfFibres());
		while (GET_BIT(bitstring, fibreIndex))
		{
			fibreIndex = (fibreIndex + 1) %MD->getTotalNumberOfFibres();
			MSG_ASSERT(fibreIndex != initialIndex, "Wrap in fibre search");
		}
		SET_BIT(bitstring, fibreIndex, 1);

		currentFibre = MD->masterFibreList_[fibreIndex];
		if (currentFibre == NULL)
			continue;

//		LogInfo("    Assigning fibre %5d at (%6.1f, %6.1f)\n",
//						fibreIndex,
//						currentFibre->getXCell(),
//						currentFibre->getYCell());



		/**
		 * find an MU for this fibre.  If this is negative
		 * there are no covering MU's (so we just forget
		 * the fibre)
		 */
		chosenMUIndex = chooseMUForFibreUsingWeightingMethod(
						currentFibre->getXCell() / CELLS_PER_MM,
						currentFibre->getYCell() / CELLS_PER_MM,
						MD,
						numFibreLayoutWeightingFunctions,
						layoutFunctionList,
						denominatorFunctionList,
						fibreProbabilties,
						(-1),
						(float) weightingLayoutNoiseFactor
					);

		if (chosenMUIndex >= 0)
		{

			currentFibre->mf_motorUnit_ =
						MD->motorUnit_[chosenMUIndex]->mu_id_;
			MD->motorUnit_[chosenMUIndex]->addFibre(currentFibre);
			MD->motorUnit_[chosenMUIndex]->recalculateCentroid();
			nCellsAssigned++;
//			LogInfo("   Assignment %d,  fibre %d (%f, %f) assigned to MU %d\n",
//							i, fibreIndex,
//						currentFibre->getXCell() / CELLS_PER_MM,
//						currentFibre->getYCell() / CELLS_PER_MM,
//						currentFibre->mf_motorUnit_);
//			MD->validate();

		} else
		{
			/** no motor unit wanted this fibre, so delete it */
			LogInfo("   Assignment %d,  fibre %d (%f, %f) deleted 0x%08x\n",
							i, fibreIndex,
						currentFibre->getXCell() / CELLS_PER_MM,
						currentFibre->getYCell() / CELLS_PER_MM,
						currentFibre);
			MD->validate();
			delete MD->masterFibreList_[i];
			MD->masterFibreList_[i] = NULL;
			MD->validate();
		}
	}

	deleteReportTimer(reportTimer);

	FREE_BITSTRING(bitstring);

#ifdef DEBUG_LAYOUT
	fclose(tmpfp);
#endif

	if (nCellsAssigned > 0)
		return 1;

	LogCrit("No cells assigned to motor units!\n");
	return 0;
}

struct distpair
{
	float distance;
	int id;
};

static int
distComparator(const void *v1, const void *v2)
{
	struct distpair *d1, *d2;

	d1 = (struct distpair *) v1;
	d2 = (struct distpair *) v2;

	if (d1->distance < d2->distance)
		return (1);

	if (d1->distance > d2->distance)
		return (-1);

	return 0;
}


/**
 * Update the expected density of motor units which
 * extend outside of the muscle
 */
static int
setMURadiiByFibreLocations(MuscleData *MD)
{
	struct distpair *distData = NULL;
	MotorUnit *curMU;
	MuscleFibre *curFibre;
	float muXInMM, muYInMM;
	float fibreXInMM, fibreYInMM;
	float radius;
	float minMUDiameter = (-1), maxMUDiameter = (-1);
	double sumRelativeChange;
	double ssRelativeChange;
	double relativeChange;
	double stdDev;
	double mean;
	int nBlocks = 0;
	int maxControlFibres;
	int i, j;
	FILE *fp;

	sumRelativeChange = 0;
	ssRelativeChange = 0;

	fp = fopen(MUDIAMETERLOGFILE_NAME, "w");

	LogInfo("\n");
	LogInfo("Updating MU diameters\n");
	for (i = 0; i < MD->getNumMotorUnits(); i++)
	{

		curMU = MD->getMotorUnitFromMasterList(i);

		if (curMU == NULL)
		{
			continue;
		}

		if (curMU->getNumFibres() == 0)
		{
			continue;
		}

		listMkCheckSize(
		            curMU->getNumFibres(),
		            (void **) &distData,
		            &nBlocks,
		            BUFSIZ / sizeof(struct distpair),
		            sizeof(struct distpair), __FILE__, __LINE__);
		MSG_ASSERT(distData != NULL, "Out of memory");

		muXInMM = (float) curMU->getXLocationInMM();
		muYInMM = (float) curMU->getYLocationInMM();

		maxControlFibres = 0;
		for (j = 0; j < curMU->getNumFibres(); j++)
		{
			curFibre = curMU->getFibre(j);
			if (curFibre == NULL)
			{
				distData[j].id = (-1);
				distData[j].distance = (-1);
				continue;
			}

			fibreXInMM = (float) (curFibre->getXCell() / CELLS_PER_MM);
			fibreYInMM = (float) (curFibre->getYCell() / CELLS_PER_MM);

			distData[j].id = j;
			distData[j].distance = (float)
					CARTESIAN_DISTANCE(muXInMM, muYInMM,
						fibreXInMM, fibreYInMM);
			maxControlFibres++;
		}

		qsort(distData,
				curMU->getNumFibres(),
				sizeof(struct distpair),
				distComparator);

		if (maxControlFibres > 5)
		{
			maxControlFibres = (int) (maxControlFibres * 0.1);
			if (maxControlFibres < 5)
			{
				maxControlFibres = 5;
			}
		}

		if (maxControlFibres > 0)
		{
			radius = 0;
			for (j = 0; j < maxControlFibres; j++)
			{
				radius += distData[j].distance;
			}
			radius = radius / (float) maxControlFibres;

			relativeChange = radius / (curMU->mu_expected_diameter_mm_ / 2.0);
			sumRelativeChange += relativeChange;
			ssRelativeChange += SQR(relativeChange);


//			LogInfo("Changing MU %d Radius from %f to %f (%f factor)\n",
//							i,
//							curMU->mu_expected_diameter_mm_,
//						(float) (2.0 * radius),
//						(float) relativeChange
//					);
			curMU->mu_diameter_mm_ = (float) (2.0 * radius);

			fprintf(fp, "%f %f\n",
							curMU->mu_expected_diameter_mm_,
						curMU->mu_diameter_mm_);
		}

		if (maxMUDiameter < 0)
		{
			maxMUDiameter = minMUDiameter = curMU->mu_diameter_mm_;
		} else
		{
			if (maxMUDiameter < curMU->mu_diameter_mm_)
				maxMUDiameter = curMU->mu_diameter_mm_;
			if (minMUDiameter > curMU->mu_diameter_mm_)
				minMUDiameter = curMU->mu_diameter_mm_;
		}
	}

	LogInfo("\n");
	mean = sumRelativeChange / MD->getNumMotorUnits();
	stdDev = sqrt(
					(ssRelativeChange / MD->getNumMotorUnits())
					- SQR(mean)
				);
	LogInfo("Mean Relative Radius Change    %f\n", (float) mean);
	LogInfo("Std Dev Relative Radius Change %f\n", (float) stdDev);

	LogInfo("\n");

	fclose(fp);

//    MD->minMotorUnitDiameter_ = minMUDiameter;
//    MD->maxMotorUnitDiameter_ = maxMUDiameter;

	if (distData != NULL)
		ckfree(distData);
	return 1;
}


/**
 * Update the expected density of motor units which
 * extend outside of the muscle
 */
static float
calculateMotorUnitExpectedFibreCounts(
		MuscleData *MD,
		float fibreDensityOfMotorUnit
	)
{
	struct report_timer *reportTimer;
	/** muscle area = number of muscle fibres */
	//float muscleArea;
	float muRadius;
	float muscleRadius;
	double muArea, rawMuArea; //, muArea_mah;
	int numAdjusted = 0;
	int numUnadjusted = 0;
	int i;


	muscleRadius = (float) (MD->muscleDiameter_ / 2.0);
	//muscleArea = (float) (M_PI * muscleRadius * muscleRadius);

	reportTimer = startReportTimer(MD->nMotorUnitsInMuscle_);

	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{

		if ((i + 1) % 10 == 0)
			LogInfo("    Fibres for MU %s\n", reportTime(i, reportTimer));

		if (MD->motorUnit_[i] == NULL)
			continue;

		muRadius = (float) (MD->motorUnit_[i]->mu_diameter_mm_ / 2.0);

		/**
		 * calculate the expected number of fibres based on
		 * the area, taking into account that some MU's extend
		 * past the muscle.
		 */


		/**
		 * Calculate the intersection of the muscle and MU so that
		 * if the MU projects out of the muscle, we trim it.
		 */

//		muArea_mah = calculateAreaOfIntersectionOfTwoCircles(
//							0, 0, muscleRadius,
//							CARTESIAN_X_FROM_POLAR(
//								MD->motorUnit_[i]->mu_loc_r_mm_,
//								MD->motorUnit_[i]->mu_loc_theta_),
//							CARTESIAN_Y_FROM_POLAR(
//								MD->motorUnit_[i]->mu_loc_r_mm_,
//								MD->motorUnit_[i]->mu_loc_theta_),
//							muRadius);
		/**
		 * Calculate the intersection of the muscle and muscle and MU so that
		 * if the MU projects out of the muscle, we trim it.
		 * This function is for three Circles, thus here muscle is repeated
		 * to play the role of the other.
		 */

		muArea = calculateAreaOfIntersectionOfThreeCircles(
							0, 0, muscleRadius,
							0, 0, muscleRadius,
							CARTESIAN_X_FROM_POLAR(
								MD->motorUnit_[i]->mu_loc_r_mm_,
								MD->motorUnit_[i]->mu_loc_theta_),
							CARTESIAN_Y_FROM_POLAR(
								MD->motorUnit_[i]->mu_loc_r_mm_,
								MD->motorUnit_[i]->mu_loc_theta_),
							muRadius);


		rawMuArea = (M_PI * muRadius * muRadius);

		if (muArea < rawMuArea)
		{
			numAdjusted++;
		} else
		{
			numUnadjusted++;
		}
	}
	deleteReportTimer(reportTimer);

	LogInfo("Adjusted %d of %d (%f) motor units for chording\n",
				numAdjusted, MD->nMotorUnitsInMuscle_,
				numAdjusted/(double)MD->nMotorUnitsInMuscle_);

//    LogInfo("%d of %d (%f) did not intersect the side of the muscle\n",
//				numUnadjusted, MD->nMotorUnitsInMuscle_,
//				numUnadjusted/(double)MD->nMotorUnitsInMuscle_);

	MSG_ASSERT(numUnadjusted + numAdjusted == MD->nMotorUnitsInMuscle_,
				"adjustment count failure");

	return 1;
}

/**
 ** This is the power function that defines the diameter
 ** of the motor units.
 **
 ** Motor unit 1 has a diameter of small, while Motor unit
 ** MD->nMotorUnitsInMuscle_ has a diameter of large.
 ** This is based on Andy Fuglevand's work.
 **/
static float
calculateMotorUnitBaseDiameters(
		MuscleData *MD,
//		float range_of_twtch_tens,
		float min_mu_diam,
		float max_mu_diam,		/** not used except to clip muscle size! */
		float fibreDensityOfMotorUnit,
		float muscleArea_per_fib
	)
{
	/** muscle area = number of muscle fibres */
	float muscleArea;
	float muRadius;
	float muscleDiameter;
	double ratio, ratio2;
	int i;


	muscleArea = 0;
//    ratio = range_of_twtch_tens;
	ratio = SQR(max_mu_diam)/SQR(min_mu_diam);
	ratio2 = log(ratio) / (MD->nMotorUnitsInMuscle_ - 1);
	ratio = SQR(min_mu_diam / 2.0) * M_PI / exp(ratio2);


	/**
	 * set motor unit territory diameter in mm
	 */
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		MD->motorUnit_[i]->mu_expected_diameter_mm_ =
				MD->motorUnit_[i]->mu_diameter_mm_ =
		        (float) sqrt(ratio * exp(ratio2 * i) / M_PI) * 2.0f;

		muRadius = (float) (MD->motorUnit_[i]->mu_expected_diameter_mm_ / 2.0);

		/** calculate the expected number of fibres based on the area */
		MD->motorUnit_[i]->mu_expectedNumFibres_ = (int)
			(fibreDensityOfMotorUnit * (M_PI * muRadius * muRadius) + 0.5);

		/**
		 * muscleArea estimates the area of the muscle, in mm**2,
		 * assuming a density of fibreDensityOfMotorUnit fibres
		 * per motor unit per mm^2.
		 *
		 * This was based on Dr. Stashuk's estimate !!!!!!
		 */

		muscleArea = (float)
		        (muscleArea +
		            M_PI * pow(
		                    (MD->motorUnit_[i]->mu_expected_diameter_mm_ / 2.0),
		                    2.0)
		                 * fibreDensityOfMotorUnit
		                 * muscleArea_per_fib
		        );
	}


	/* the estimated muscle diameter is */
	muscleDiameter = (float) (sqrt(muscleArea / M_PI) * 2.0);
	if (muscleDiameter < max_mu_diam)
	{
		LogWarning("%s -- fixing muscle diameter from %f to %f\n",
		        "Muscle too small",
		        muscleDiameter,
		        max_mu_diam);
		muscleDiameter = max_mu_diam;
	}

	return muscleDiameter; // / 2.0;
}


///**
// * Figure out what the motor unit density is at the center
// *
// * We are checking the motor unit density -- we want to count
// * the number of motor units which are witin a 16 millimeter
// * square in the center.
// */
//static int
//checkMotorUnitDensity(MuscleData *MD)
//{
//    Rect searchRect;
//    struct rTreeResultList rtreeResults;
//    int nMotorUnitsIntersecting;
//    double motorUnitDensity;
//
//    /** search for the point which is the centre */
//    searchRect.boundary[0] = 0.0;
//    searchRect.boundary[1] = 0.0;
//    searchRect.boundary[2] = 0.0;
//    searchRect.boundary[3] = 0.0;
//
//
//    /** use the R-Tree to gather possible MU's */
//    memset(&rtreeResults, 0, sizeof(rtreeResults));
//    nMotorUnitsIntersecting = RTreeSearch(
//				MD->muRTreeRoot_,
//                &searchRect,
//                MUrTreeSearchCallback__,
//                (void *) &rtreeResults
//			);
//
//    /** we do not need the actual list, and must free it */
//    ckfree(rtreeResults.results_);
//
//    /** calculate the density based on the number found */
//    motorUnitDensity = nMotorUnitsIntersecting / 4.0;
//
//    LogInfo("\n");
//    LogInfo("Motor Unit Density/mm in center is : %f\n",
//				motorUnitDensity);
//    LogInfo("\n");
//
//    if ((motorUnitDensity >= MIN_ALLOWED_MU_DENSITY) &&
//					(motorUnitDensity <= MAX_ALLOWED_MU_DENSITY))
//    {
//		return 1;
//    }
//
//    LogCrit("Motor Unit density out of bounds (%f not between %f, %f)\n",
//				motorUnitDensity,
//				MIN_ALLOWED_MU_DENSITY,
//				MAX_ALLOWED_MU_DENSITY);
//    return 0;
//}

#ifdef DEBUG_LOG_MUSCLE
static void
logMuscle(MuscleData *MD)
{
	const char *heading = "mu #fibres";
	const char *title = "MOTOR UNITS WITH FIBRES IN THE TEST AREA";
	const int columnWidth = 13;
	const int numColumns = (int) (72 / columnWidth);
	const int idWidth = 4;
	int i, width, dataCount;

	LogInfo("%*s%s\n",
			((numColumns * columnWidth) / 2) - (strlen(title) / 2),
			"", title);

	LogInfo("|");
	for (i = 0; i < numColumns; i++)
	{
		LogInfo("%*s|", (columnWidth-1), heading);
	}
	LogInfo("\n");

	for (i = 0; i < (numColumns * columnWidth) + 1; i++)
	{
		LogInfo("-");
	}
	LogInfo("\n");


	dataCount = 0;
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		if (MD->motorUnit_[i] != NULL && MD->motorUnit_[i]->mu_nFibres_ > 0)
		{
		    LogInfo("| %*d %*d ",
							idWidth, MD->motorUnit_[i]->mu_id_,
						columnWidth - (4 + idWidth),
						MD->motorUnit_[i]->mu_nFibres_);
		    dataCount++;
			if ((dataCount % numColumns) == 0)
			{
				LogInfo("|\n");
			}
		}
	}

	if ((dataCount % numColumns) != 0)
	{
		LogInfo("|\n");
	}

	/** print out terminal row of -'s */
	width = ((dataCount % numColumns) * columnWidth);
	if (width == 0)
		width = numColumns * columnWidth;
	for (i = 0; i < width + 1; i++)
	{
		LogInfo("-");
	}
	LogInfo("\n");
}

static void
logMuscleFibreCounts(MuscleData *MD)
{
	const char *title = "Muscle Fibre Counts";
	const char *heading[] =
	{
						"MU", "want", "got", "frac"
					};
	const int subColumnWidth = 4;
	const int doubleColumnWidth = 5;
	const int columnWidth = (subColumnWidth * 3) + doubleColumnWidth + 5;
	const int numColumns = (int) (72 / columnWidth);
	double mean, sigmaSquared, sigma;
	double fraction;
	int i, width, dataCount;
	int numWithZero = 0;

	LogInfo("%*s%s\n",
			((numColumns * columnWidth) / 2) - (strlen(title) / 2),
			"", title);

	LogInfo("|");
	for (i = 0; i < numColumns; i++)
	{
		LogInfo(" %*s:%*s %*s %*s |",
						(subColumnWidth), heading[0],
						(subColumnWidth), heading[1],
						(subColumnWidth), heading[2],
						(doubleColumnWidth), heading[3]);
	}
	LogInfo("\n");

	for (i = 0; i < (numColumns * columnWidth) + 1; i++)
	{
		LogInfo("-");
	}
	LogInfo("\n");


	dataCount = 0;
	mean = 0;
	sigmaSquared = 0;
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		if (MD->motorUnit_[i] != NULL)
		{
			if (MD->motorUnit_[i]->mu_nFibres_ <= 0)
			{
				numWithZero++;
			} else
			{

				fraction = ((double) MD->motorUnit_[i]->mu_nFibres_
						 / (double) MD->motorUnit_[i]->mu_expectedNumFibres_);
				LogInfo("| %*d:%*d %*d %*.*f ",
							subColumnWidth, i + 1,
							subColumnWidth,
									MD->motorUnit_[i]->mu_expectedNumFibres_,
							subColumnWidth, MD->motorUnit_[i]->mu_nFibres_,
							doubleColumnWidth, (doubleColumnWidth - 2),
									fraction);
				dataCount++;
				if ((dataCount % numColumns) == 0)
				{
					LogInfo("|\n");
				}

				mean += fraction;
				sigmaSquared += (fraction * fraction);
			}
		}
	}

	if ((dataCount % numColumns) != 0)
	{
		LogInfo("|\n");
	}

	/** print out terminal row of -'s */
	width = ((dataCount % numColumns) * columnWidth);
	if (width == 0)
		width = numColumns * columnWidth;
	for (i = 0; i < width + 1; i++)
	{
		LogInfo("-");
	}
	LogInfo("\n");


	mean = mean / dataCount;
	sigmaSquared = (sigmaSquared / dataCount) - (mean * mean);
	sigma = sqrt(sigmaSquared);

	LogInfo("\n");
	LogInfo("Muscle Fibre Allocation Statistics:\n");
	LogInfo("                  Mean : %f\n", mean);
	LogInfo("              Variance : %f\n", sigmaSquared);
	LogInfo("    Standard Deviation : %f\n", sigma);
	LogInfo("\n");
	LogInfo("  # MU's with 0 fibres : %d\n", numWithZero);

	LogInfo("\n");
}
#endif // DEBUG_LOG_MUSCLE

int
setAndClipNeedleLocation(
		MuscleData *MD,
		float muscleRadius,
		float needleX,
		float needleY,
		float needleZ,
		float needleCannulaLength,
		float needleCannulaRadius
	)
{
	double distanceFromCentre;
	double theta, excess;

	distanceFromCentre = sqrt(SQR(needleX) + SQR(needleY));

	/** if we are within one mm of the edge of the muscle, clip */
	excess = distanceFromCentre -
				(muscleRadius - MIN_NEEDLE_INSERTION_DISTANCE_MM);
	if (excess > 0)
	{
		theta = atan2(needleY, needleX);
		needleX -= (float) (cos(theta) * excess);
		needleY -= (float) (sin(theta) * excess);
		LogAlert("Clipping needle position to (%f,%f)\n",
						needleX, needleY);
	}

	/**
	 * we are potentially placing the needle after a regeneration
	 * so if there is already needle data, we need to delete it.
	 */
	if (MD->needle_ != NULL)
	{
		delete MD->needle_;
	}
	MD->needle_ = new NeedleInfo(
		        needleX, needleY,
		        needleZ,
		        needleCannulaLength,
		        needleCannulaRadius,
		        DEFAULT_CANNULA_SLOPE
		    );

	return 1;
}

int storeNeedleInfo(
		MuscleData *MD,
		int emgFileId,
		const char *directory
	)
{
	char tmpFilename[FILENAME_MAX];

	slnprintf(tmpFilename, FILENAME_MAX,
			"%s\\Needle%d.dat", directory, emgFileId);
	return MD->needle_->store(tmpFilename);
}


/**
 * Create a muscle definition by randomly placing motor
 * unit centers to acheive a given density, and then
 * assigning fibres to the motor units.
 *
 * Needle location is stored, but not used to plow fibres
 * in the initial layout in order that this layout can be
 * used as a master layout for later needle placements
 */
MuscleData *
muscle(
		int emgFileId,
		const char *muscleDirectory,
		const char *outputDirectory,
		MuscleParameters *muscleParams,
		PathologyControl *pathologyParams,
		float needleCannulaLength,
		float needleCannulaRadius,
		float needleX,
		float needleY,
		float needleZ,
		int motorUnitLayoutType,
		int doJitterSuperFibres,
		int muscleLayoutFunctionType
	)
{
	char tmpFilename[FILENAME_MAX];
	double muDensityFound, muDensityRequired;
	MuscleData *MD = NULL;
	MotorUnit *mu;
	int muscleDensityMaxLayoutTries = 10;
	int newNumberMotorUnits;
	int muscleDensityOk = 0;
	int i;

	/**
	 * First verify that all the disease information is
	 * within sane boundaries
	 */
	if (pathologyParams->neuropathicMULossFraction > 0.0)
	{
		/** clip neuopathy to 1.0 */
		if (pathologyParams->neuropathicMULossFraction > 1.0)
		{
			if (pathologyParams->neuropathicMULossFraction < 100.0)
			{
				LogInfo("Interpreting neuropathic "
						"involvement as a percentage\n");
				pathologyParams->neuropathicMULossFraction /= 100.0;
			} else {
				LogCrit("Neuropathic involvement of %g uninterpretable\n",
						pathologyParams->neuropathicMULossFraction);
				goto FAIL;
			}
		}

		/**
		 * min useful "adoption distance" is 1; at 0 there is no
		 * adoption.  If this is set to zero it will effectively
		 * prevent the adoption of fibres.
		 */
		if (pathologyParams->neuropathicMaxAdoptionDistanceInUM < 0)
		{
			pathologyParams->neuropathicMaxAdoptionDistanceInUM = 0;
		}
	}

	if (pathologyParams->myopathicFractionOfFibresAffected > 0.0)
	{
		if (pathologyParams->myopathicFractionOfFibresAffected > 1.0)
		{
			if (pathologyParams->myopathicFractionOfFibresAffected < 100.0)
			{
				LogInfo("Interpreting myopathic "
						"involvement as a percentage\n");
				pathologyParams->myopathicFractionOfFibresAffected /= 100.0;
			} else {
				LogCrit("Myopathic involvement of %g uninterpretable\n",
						pathologyParams->myopathicFractionOfFibresAffected);
				goto FAIL;
			}
		}
	}


	LogInfo("\nCreating Muscle Data and Saving to File\n");

	MD = new MuscleData();

	while (muscleDensityMaxLayoutTries-- > 0)
	{

		MD->allocateNMotorUnits(muscleParams->numMotorUnits);
	//    MD->validate();

		MD->minMotorUnitDiameter_ = muscleParams->minMUDiam;
		MD->maxMotorUnitDiameter_ = muscleParams->maxMUDiam;


		/**
		 ** Construct the motor units by size
		 **/
		LogInfo("     Creating Muscle Data ...\n\n");
		MD->muscleDiameter_ = calculateMotorUnitBaseDiameters(
					MD,
//					muscleParams->rangeOfTwitchTensions,
					muscleParams->minMUDiam,
					muscleParams->maxMUDiam,
					muscleParams->fibreDensity,
					muscleParams->areaPerFibre
				);
		LogInfo("Muscle diameter_ = %f\n", MD->muscleDiameter_);


		if ( !  setAndClipNeedleLocation(
					MD,
					(float) (MD->muscleDiameter_ / 2.0),
					needleX, needleY,
					needleZ,
					needleCannulaLength,
					needleCannulaRadius))
		{
			LogCrit("Needle placement failed\n");
			goto FAIL;
		}

		/**
		 * calculate the model layout distance to always encompass
		 * the entire muscle
		 *
		 * N.B.:  diam * 500 = (diam/2) * 1000
		 */
		muscleParams->modelLayoutDistanceInUM =
					(int) (MD->muscleDiameter_ * 500.0);



	//    MD->validate();
		/**
		 ** Lay out the motor unit territories
		 **/
		if ( ! layoutMotorUnitCentroids(MD,
					muscleParams,
					&MD->muscleDiameter_,
					motorUnitLayoutType,
					&muDensityFound,
					&muDensityRequired) )
		{


			/**
			 * try and adjust the number of motor units to get
			 * density to fit
			 */
			if (muDensityFound < muDensityRequired)
			{
				LogInfo("Density too low . . . (%f < %f)\n",
						muDensityFound, muDensityRequired);

				newNumberMotorUnits = 1 + (int)
						((float) muscleParams->numMotorUnits * 1.5);
			} else
			{
				LogInfo("Density too high . . . (%f < %f)\n",
						muDensityFound, muDensityRequired);

				newNumberMotorUnits = (int)
						((float) muscleParams->numMotorUnits * 0.98);
				if (newNumberMotorUnits < 25)
				{
					newNumberMotorUnits = 25;
				}
			}

			LogInfo("Density adjust -- changing number of MU's from %d to %d\n",
					muscleParams->numMotorUnits, newNumberMotorUnits);

			muscleParams->numMotorUnits = newNumberMotorUnits;

			LogInfo("Trying muscle layout again . . .\n");
			continue;
		}

		muscleDensityOk = 1;
	//    MD->validate();

		if (isnan(MD->muscleDiameter_))
		{
			LogInfo("Muscle diameter is a NAN\n");
		}

	}

	if (muscleDensityOk)
	{
		LogInfo("Found reasonable muscle density of %f, want %f\n",
				muDensityFound, muDensityRequired);
	} else
	{
		LogInfo("Can't find good muscle density\n");
		goto FAIL;
	}

#ifdef DEBUG_LAYOUT
	if ( ! dumpMuLayout(MD, muscleDirectory) )
	{
		LogCrit("Critical error from debug dump routine\n");
		goto FAIL;
	}
#endif /* DEBUG_LAYOUT */

	/**
	 * set up the R-Tree
	 */
//    if ( ! addMotorUnitsToRTree(
//						MD,
//						MD->muscleDiameter_,
//						-1, //muscleParams->maxMUDiam
//						1.5f,
//						muscleParams->maxMUDiam
//					) )
//		{
//        goto FAIL;
//    }


//    MD->validate();
	/*
	 * This section of the program selects the motor unit
	 * to represent each of the fibres in the detection area
	 */
	LogInfo(" . Doing fibre assignments . . .\n");
//    if (! doJitterSuperFibres)
	{
		if ( ! allocateFibres(MD, muscleParams->modelLayoutDistanceInUM) )
		{
			goto FAIL;
		}
//		MD->validate();

		if (muscleLayoutFunctionType == LAYOUT_WEIGHTING){
			if ( ! assignFibresToMotorUnitsUsingWeightingMethod(
					muscleDirectory,
					MD,
					muscleParams->modelLayoutDistanceInUM,
					muscleParams->numFibreLayoutWeightingFunctions,
					muscleParams->fibreLayoutWeightingFunctionList,
					muscleParams->fibreDenominatorWeightingFunctionList,
					muscleParams->fibreLayoutWeightingFunctionWeights,
					muscleParams->fibreLayoutWeightingNoiseFactor) )
			{
				goto FAIL;
			}
		} else
		{
			if ( ! assignFibresToMotorUnitsUsingProbabilityMethod(
					muscleDirectory,
					MD,
					muscleParams->modelLayoutDistanceInUM,
					muscleParams->numFibreLayoutProbabilityFunctions,
					muscleParams->fibreLayoutProbabilityFunctionList,
					muscleParams->fibreLayoutProbabilityFunctionProbabilities) )
			{
				goto FAIL;
			}
		}

		/**
		 * fix the number of healthy fibres now that we have finished
		 * allocating fibres to motor units
		 */
		for (i = 0; i < MD->getNumMotorUnits(); i++)
		{
			mu = MD->getMotorUnitFromMasterList(i);
			mu->mu_nHealthyFibres_ = mu->mu_nFibres_;
		}


//		MD->validate();

//    } else {
//		if ( ! assignSuperFibresToMotorUnits(
//							muscleDirectory,
//						MD,
//						muscleParams->modelLayoutDistanceInUM) ) {
//			goto FAIL;
//		}
   }


	/**
	 * if density out of bounds, then abort this run
	 */
		/** density check uses the R-tree -- new max bound throws this off */
//    if ( ! checkMotorUnitDensity(MD) ) {
//		LogCrit("Aborting due to bad MU density\n");
//		goto FAIL;
//    }

	LogInfo("Number of units intersecting center : %d\n",
			countMotorUnitsIntersectingCentre(MD));


//    MD->validate();
	LogInfo("Calcuating MU expected fibre counts\n");
	if ( ! calculateMotorUnitExpectedFibreCounts(
						MD, muscleParams->fibreDensity) )
	{
		goto FAIL;
	}


//    MD->validate();
	LogInfo("Calcuating MU radii limits based on fibre locations\n");
	if ( ! setMURadiiByFibreLocations(MD) )
	{
		goto FAIL;
	}

	LogInfo("Calcuating fibre diameter distributions and z-shifts of NMJ's\n");
	if ( ! setFibreDiameterAndShift(MD, muscleDirectory) )
	{
		goto FAIL;
	}


	/**
	 * Handle Neuropathies -- this edits the muscle as created
	 * so far and updates the appropriate fields.
	 *
	 * We do this before calculating the number of MUs, so that
	 * the detection area count comes out all right.  This must
	 * be done _after_ the check for density, as it obviously
	 * affects the density in a possbily detrimental way
	 */
	if (pathologyParams->neuropathicMULossFraction > 0.0)
	{
		LogInfo("Simulating neuropathic involvement . . .\n");
		updateMuscleWithNeuropathy(MD,
				pathologyParams->neuropathicMULossFraction,
				(int) ((pathologyParams->neuropathicMaxAdoptionDistanceInUM
					   		/ UM_PER_CELL) + 0.5),
				pathologyParams->neuropathicEnlargementFraction
			);
	}


	if (pathologyParams->myopathicFractionOfFibresAffected > 0.0)
	{
		LogInfo("Simulating myopathic involvement . . .\n");
		updateMuscleWithMyopathy(MD,
				pathologyParams->myopathicFractionOfFibresAffected,
				pathologyParams->myopathicFibreDiameterMean,
				pathologyParams->myopathicFibreGraduallyDying,
				pathologyParams->myopathicDependentProcedure,
				pathologyParams->myopathicCycleNewInvolvementPercentage,
				pathologyParams->myopathicFibreDeathDiameter,
				pathologyParams->myopathicPercentageOfAffectedFibersDying,
				pathologyParams->myopathicHypertrophicFibreFraction,
				pathologyParams->myopathicHypertrophySplitThreshold,
				pathologyParams->myopathicPercentageOfHypertrophicFibersSplit,
				pathologyParams->myopathicAtrophyRatePerCycle,
				pathologyParams->myopathicHypertrophyRatePerCycle
			);
	}
	/** if either of these are true, the R-tree is now invalid */
	if ((pathologyParams->myopathicFractionOfFibresAffected > 0.0) ||
				(pathologyParams->neuropathicMULossFraction > 0.0))
	{
		if (MD->fibreRTreeRoot_ != NULL)
		{
			RTreeDeleteIndex(MD->fibreRTreeRoot_);
			MD->fibreRTreeRoot_ = NULL;
		}
	}


	/**
	 * calculate the MU's in the detection area
	 */
	LogInfo("Calculating which MU's are in the detection area\n");
	MD->nMotorUnitsInDetectionArea_ = 0;
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		if (MD->motorUnit_[i]->mu_nFibres_ > 0)
		{
		    listMkCheckSize(
		            MD->nMotorUnitsInDetectionArea_ + 1,
		            (void **) &MD->motorUnitInDetect_,
		            &MD->motorUnitInDetectBlocks_,
		            16,
		            sizeof(MotorUnit *), __FILE__, __LINE__);
			MSG_ASSERT(MD->motorUnit_[i]->mu_id_ == i + 1,
							"Motor Unit ID mismatch");
		    MD->motorUnitInDetect_[
		                MD->nMotorUnitsInDetectionArea_
		            ] = MD->motorUnit_[i];
		    MD->nMotorUnitsInDetectionArea_++;
		}
	}

	MD->needle_->cannulaLength_ = (float) (MD->muscleDiameter_ / 2.0);

//    MD->validate();
	LogInfo("Muscle generation complete\n");

//    if ( ! checkMotorUnitDensity(MD) )
//		goto FAIL;
//


	/*
	 * This section of program lets you plot the fibre locations
	 * for up to 3 motor units.
	 * A table is created containing all the motor units with at
	 * least one fibre in the detection area.
	 */
	LogInfo("\n");
#   ifdef DEBUG_LOG_MUSCLE
	LogInfo("\n");
	logMuscle(MD);
	LogInfo("\n");
	LogInfo("\n");
	logMuscleFibreCounts(MD);
	LogInfo("\n");
	LogInfo("\n");
#   endif



	slnprintf(tmpFilename, FILENAME_MAX,
			"%s\\MF-unplowed.dat", muscleDirectory);
	MD->writeMFInfo(tmpFilename);

	slnprintf(tmpFilename, FILENAME_MAX,
			"%s\\MUParam.dat", muscleDirectory);
	MD->writeMUParam(tmpFilename);

	slnprintf(tmpFilename, FILENAME_MAX,
			"%s\\MU.dat", muscleDirectory);
	MD->writeMUInfo(tmpFilename);

	storeNeedleInfo(MD, emgFileId, outputDirectory);
	return MD;


FAIL:
	LogCrit("Muscle routine has FAILED -- cleaning up\n");
	if (MD != NULL) delete MD;
	return NULL;
}


/**
 * Load the data generated in muscle() from a disk based file
 */
MuscleData *
loadMuscleData(
		int emgFileId,
		const char *muscleDirectory,
		const char *outputDirectory,
		int usePlowedFibres
	)
{
	char tmpFilename[FILENAME_MAX];
	MuscleData *MD;

	LogInfo("\nLoading Muscle Data from File\n");

	MD = new MuscleData();

	/*LogInfo(tmpFilename);*/


	/**
	 * load the data that was saved before
	 */
	if ( ! MD->loadData(muscleDirectory,
						outputDirectory,
						emgFileId,
						usePlowedFibres) )
	{
		LogCrit("Load of Muscle Data from %s failed\n",
				muscleDirectory);
		goto FAIL;
	}



	if (emgFileId > 0)
	{
		MD->needle_ = new NeedleInfo();
		slnprintf(tmpFilename, FILENAME_MAX, "%s\\Needle%d.dat",
					outputDirectory,
				emgFileId);
		if ( ! MD->needle_->load(tmpFilename) )
		{
			LogCrit("Load of NeedleData%d from %s failed\n",
				emgFileId, outputDirectory);
			goto FAIL;
		}
	} else
	{
		MD->needle_ = NULL;
	}

	LogInfo("Muscle load complete\n");

	return MD;

FAIL:
	delete MD;
	return NULL;
}


