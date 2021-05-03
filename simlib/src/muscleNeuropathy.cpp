/**
 * Take a muscle which has already been laid out and edit its
 * setup for neuropathic involvement
 *
 * $Id: muscleNeuropathy.cpp 28 2020-09-25 20:00:58Z andrew $
 */

#include "os_defs.h"

#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
#endif

#define PRIVATE public
#include "MuscleData.h"
#include "muscle.h"

#include "listalloc.h"
#include "tclCkalloc.h"
#include "random.h"
#include "stringtools.h"
#include "mathtools.h"
#include "reporttimer.h"
#include "bitstring.h"
#include "massert.h"
#include "log.h"

#include "rTreeIndex.h"

//#define		EXPANDING_FIBRE_ADOPTION_SEARCH		1

int
FibreRTreeSearchCallback__(long id, void *voidResultList)
{
	struct rTreeResultList *result =
				(struct rTreeResultList *) voidResultList;

	listMkCheckSize(
			result->nEntries_ + 1,
			(void **) &result->results_,
			&result->nBlocks_,
			16,
			sizeof(int), __FILE__, __LINE__);

	/**
	 * ids were stored with values 1 greater than that
	 * desired to account for 0 being invalid, so we
	 * remove the extra 1 here
	 */
	result->results_[result->nEntries_++] = (id - 1);

	return 1; /** keep searching */
}

static int
chooseAdjacentMUForFibre(
		MuscleFibre *fibreData,
		MuscleData *MD,
		Node *fibreRTree,
		int maxDistance,
		int currentOwnerID,
		BITSTRING fibreOwnerFlags,
		float maxFibreCountFraction
	)
{
	struct rTreeResultList rtreeResults;
	MuscleFibre *adoptiveFibre;
	Rect searchRect;
	//float fibreXInMM, fibreYInMM;
	double changeFraction;
	int muID;
	int *possibleIdList;
	int nPossibleFibres;
	int listIndex;
	int adoptiveFibreId;
	int fibreChoiceOK;
	int i, j;


	//fibreXInMM = fibreData->getXLocationInMM();
	//fibreYInMM = fibreData->getYLocationInMM();

	// LogInfo("Max adoption distance is %d\n", maxDistance);


#ifdef		EXPANDING_FIBRE_ADOPTION_SEARCH
	i = 1;
#else
	i = maxDistance - 1;
#endif

	while (i <= maxDistance)
	{

		// LogInfo("   Trying adoption distance of %d\n", i);
		/**
		 * Distance is in cells, so we simply add in the
		 * distance indicated by i to find centers of other
		 * fibres
		 */
		searchRect.boundary[0] =
		   		 fibreData->mf_xCell_ - (float) (i + 0.25);
		searchRect.boundary[1] =
					fibreData->mf_yCell_ - (float) (i + 0.25);
		searchRect.boundary[2] =
					fibreData->mf_xCell_ + (float) (i + 0.25);
		searchRect.boundary[3] =
					fibreData->mf_yCell_ + (float) (i + 0.25);

		/** get a list of possible adjacent fibres */
		memset(&rtreeResults, 0, sizeof(rtreeResults));
		nPossibleFibres = RTreeSearch(
					fibreRTree,
					&searchRect,
					FibreRTreeSearchCallback__,
					(void *) &rtreeResults
				);
		possibleIdList = rtreeResults.results_;

		/**
		 * Now determine whether we see anything we like
		 * in the results list
		 */
		while (nPossibleFibres > 0)
		{
			adoptiveFibre = NULL;
			listIndex = intRangeRandom(nPossibleFibres);
			adoptiveFibreId = possibleIdList[ listIndex ];
			adoptiveFibre = MD->getFibre(adoptiveFibreId);

			/**
			 * if we have found a deleted fibre, or our own
			 * parent MU, then delete this candidate from the
			 * list and choose another
			 */
			MSG_ASSERT(adoptiveFibreId < MD->getTotalNumberOfFibres(),
									"bitstring");


			fibreChoiceOK = 1;

			/** any of the following disallow this fibre */
			if (adoptiveFibre == NULL)
			{
				fibreChoiceOK = 0;

			} else if (adoptiveFibre->getMotorUnit() == currentOwnerID)
			{
				fibreChoiceOK = 0;

			} else if (GET_BIT(fibreOwnerFlags, adoptiveFibreId))
			{
				fibreChoiceOK = 0;

			} else
			{
				muID = adoptiveFibre->getMotorUnit();
				MSG_ASSERT(MD->motorUnit_[muID-1]->mu_id_ == muID, "index bad");

				changeFraction = (MD->motorUnit_[muID-1]->mu_nFibres_ + 1)
						/ (MD->motorUnit_[muID-1]->mu_nHealthyFibres_);

				/**
				 * if this changes the MU by too much, then
				 * discard this option
				 */
				if (changeFraction > maxFibreCountFraction)
				{
					fibreChoiceOK = 0;
				}
			}


			if (! fibreChoiceOK)
			{

				for (j = listIndex + 1; j < nPossibleFibres; j++)
				{
					possibleIdList[ j-1 ] = possibleIdList[ j ];
				}
				nPossibleFibres--;
			} else
			{
				/**
				 * we found something, so we will break
				 * out of this loop processing
				 */
				ckfree(possibleIdList);
				return adoptiveFibre->getMotorUnit();
			}
		}

		/** clean up result list */
		ckfree(possibleIdList);

		i++;
	}

	return (-1);
}


static int
removeNeuron(
		MuscleData *MD,
		Node *fibreRTree,
		int maxDistance,
		BITSTRING fibreOwnerFlags,
		float neuropathicEnlargementFraction
	)
{
	MuscleFibre *fibreData;
	int neuronIndex, neuronStartIndex;
	int nFibres;
	int chosenMU, chosenMUIndex;
	int i;

	neuronStartIndex = neuronIndex
				= intRangeRandom(MD->nMotorUnitsInMuscle_);

	/** find an MU which has not already been destroyed */
	nFibres = MD->motorUnit_[neuronIndex]->mu_nFibres_;
	while (nFibres == 0)
	{
		neuronIndex = (neuronIndex + 1) % MD->nMotorUnitsInMuscle_;
		nFibres = MD->motorUnit_[neuronIndex]->mu_nFibres_;
		if (neuronStartIndex == neuronIndex)
		{
			LogWarning("Only one neuron (%d, with %d fibres) remains\n",
						neuronIndex,
						MD->motorUnit_[neuronIndex]->mu_nFibres_);
			return 0;
		}
	}


	/**
	 * Clear all the fibre flags.  We will use these flags
	 * in this loop to ensure that we do not adopt to fibres
	 * which are being adopted this time around -- ie; that
	 * each fibre gets a fair chance of being adopted "at
	 * the same time"
	 */
	ZERO_BITSTRING(fibreOwnerFlags, MD->getTotalNumberOfFibres() + 1);

	/**
	 * locate all the fibres this MU controls, and add
	 * them to other MU's, based on coverage
	 */
	for (i = nFibres - 1; i >= 0; i--)
	{

		fibreData = MD->motorUnit_[neuronIndex]->mu_fibre_[i];
		MSG_ASSERT(fibreData != NULL, "Null Fibre Found!");



		/** choose a new MU for this fibre */
		if (maxDistance > 0)
		{
			chosenMU = chooseAdjacentMUForFibre(fibreData,
						MD, fibreRTree, maxDistance,
						MD->motorUnit_[neuronIndex]->mu_id_,
						fibreOwnerFlags,
						neuropathicEnlargementFraction
					);
		} else
		{
			chosenMU = (-1);
		}

		if (chosenMU > 0)
		{


			/**
			 * flag this fibre as "used" in the bitstring,
			 * so that we do not choose it as a later adoptive
			 * fibre
			 */
			MSG_ASSERT(i < MD->getTotalNumberOfFibres(), "bitstring");
			SET_BIT(fibreOwnerFlags, i, 1);

			/**
			 * If there was a new MU close enough, move
			 * the fibre to the new parent neuron
			 */
			chosenMUIndex = chosenMU - 1;

//			LogInfo("    Neuropathy:       Fibre %d adopted by MU %d\n",
//							i, chosenMU);

			MD->motorUnit_[neuronIndex]->removeFibre(i);
			MD->motorUnit_[chosenMUIndex]->addFibre(fibreData);
			fibreData->mf_motorUnit_ =
						MD->motorUnit_[chosenMUIndex]->mu_id_;
			/**
			 * FIX:
			 * We need to think through what happens to the
			 * jShift here
			 */
		} else
		{
			/**
			 * If there is no adopting neurong, then
			 * we assume the fibre dies, and becomes
			 * adipose tissue.  This implies we do not
			 * need to model it, so it is removed.
			 */
			MD->motorUnit_[neuronIndex]->removeFibre(fibreData);
			MD->removeFibre(fibreData);
			delete fibreData;
		}
	}

	MSG_ASSERT(MD->motorUnit_[neuronIndex]->getNumFibres() == 0,
			"MU not empty after delete");

	return 1;
}


/**
 * Simulate Neuropathic involvement by removing a fraction of
 * the neurons
 */
int
updateMuscleWithNeuropathy(
		MuscleData *MD,
		float involvement,
		int maxAdoptionDistanceInCells,
		float neuropathicEnlargementFraction
	)
{
	int numNeuronsInvolved;
	int numNonZeroMUs = 0;
	int status = 1;
	BITSTRING fibreOwnerFlags = NULL;
	int i;

	LogInfo("    Neuropathy : involvement %f\n", involvement);
	LogInfo("               : enlargement %f\n",
			neuropathicEnlargementFraction);
	LogInfo("    Neuropathy : max adoption distance %d\n",
			maxAdoptionDistanceInCells);

	if ( ! buildFibreRTree(MD) )
		return 0;

	fibreOwnerFlags = ALLOC_BITSTRING(MD->getTotalNumberOfFibres() + 1);
	LogInfo("    %d bits allocated for fibre-flag bitstring\n",
							MD->getTotalNumberOfFibres());

	/**
	 * Figure out how many MUs have non-zero fibre count
	 */
	for (i = 0; i < MD->nMotorUnitsInMuscle_; i++)
	{
		if (MD->motorUnit_[i]->mu_nFibres_ > 0)
		{
			numNonZeroMUs++;
		}
	}

	numNeuronsInvolved = (int) ceil(numNonZeroMUs * involvement);

	/** trim in case we have over-involved */
	if (numNeuronsInvolved > (numNonZeroMUs - NEURO_MIN_NUM_MUS))
		numNeuronsInvolved = (numNonZeroMUs - NEURO_MIN_NUM_MUS);

	LogInfo("    Neuropathy :   of %d MUs, %d have > zero fibres\n",
				MD->nMotorUnitsInMuscle_, numNonZeroMUs);
	LogInfo("    Neuropathy :   based on "
			"ceil(MUs with fibres (%d) * involvement (%s))\n",
				numNonZeroMUs, niceDouble(involvement));
	LogInfo("    Neuropathy : Death of "
			"%d neurons (total in muscle: %d = %s%%)\n",
				numNeuronsInvolved, numNonZeroMUs,
				niceDouble(
						numNeuronsInvolved
								/ (double)numNonZeroMUs
					));

	{
		struct report_timer *rt;
		time_t startTime, curTime;

		rt = startReportTimer(numNeuronsInvolved);
		startTime = time(NULL);

		/** remove the appropriate number of neurons */
		for (i = 0; i < numNeuronsInvolved; i++)
		{

			curTime = time(NULL);
			if ((curTime - startTime > 7) && ((i % 10) == 0))
			{
				LogInfo("        Removing neruron %s of %d\n",
								reportTime(i, rt), numNeuronsInvolved);
			}
			if ( ! removeNeuron(
							MD,
							MD->fibreRTreeRoot_,
							maxAdoptionDistanceInCells,
							fibreOwnerFlags,
							neuropathicEnlargementFraction
						) )
			{
				LogWarning("    Neuropathy : "
							"Failure removing neuron %d of %d\n",
							i, numNeuronsInvolved);
				status = 0;
				goto CLEANUP;
			}
		}
		deleteReportTimer(rt);
	}

	LogInfo("    Neuropathy : Simulation Successful\n");

CLEANUP:
	if (fibreOwnerFlags != NULL)
		FREE_BITSTRING(fibreOwnerFlags);
	return status;
}


static int
splitCurrentMuscleFibre(
		MuscleData *MD,
		MuscleFibre *currentFibre,
		int currentFibreIndex,
		double areaBeforeSplit,
		BITSTRING involvedFibres,
		BITSTRING hypertrophicFibres,
		BITSTRING splitFibres,
		int numBitsAllocated
	)
{
	MuscleFibre *newFibre;
	int newFibreMUIndex;
	int newFibreMuscleIndex;
	int splitFlag;
	double areaAfterSplit;

	/*
	LogInfo("        Myopathy Splitting fibre %d\n", currentFibreIndex);
	*/

	splitFlag = GET_BIT(splitFibres, currentFibreIndex);

	MSG_ASSERT(splitFlag == 0, "re-splitting fibre");

	/** move the old fibre slightly */
	currentFibre->mf_xCell_ = currentFibre->mf_xCell_ + 0.5f;

	/**
	 * add a new companion fibre, copying all
	 * relevant data from the old fibre
	 */
	newFibre = new MuscleFibre(
				currentFibre->getXCell() - 0.5f,
				currentFibre->getYCell()
			);
	newFibre->mf_motorUnit_				= currentFibre->mf_motorUnit_;
	newFibre->mf_jShift_				= currentFibre->mf_jShift_;
	newFibre->mf_healthyDiameter_		= currentFibre->mf_healthyDiameter_;


	/**
	 * add the new fibre to the muscle at the
	 * end of the list of fibres . . .
	 */


	/** for the motor unit */
	newFibreMUIndex = MD->motorUnit_[
					currentFibre->mf_motorUnit_-1
				]->getNumFibres();
	MD->motorUnit_[
					currentFibre->mf_motorUnit_-1
				]->addFibre(newFibre);
	MSG_ASSERT(MD->motorUnit_[
					currentFibre->mf_motorUnit_-1
				]->getFibre(newFibreMUIndex) == newFibre,
				"fibre index math incorrect in MU");

	/**
	 * for the muscle itself
	 */
	newFibreMuscleIndex = MD->getTotalNumberOfFibres();
	MD->addFibre(newFibre);
	MSG_ASSERT(MD->getFibre(newFibreMuscleIndex) == newFibre,
				"fibre index math incorrect in muscle");
	MSG_ASSERT(MD->getTotalNumberOfFibres() == newFibreMuscleIndex + 1,
				"fibre index math incorrect in muscle index");


	/**
	 * set the diameter of both halves based on each having
	 * half the current area
	 */
	//areaAfterSplit = areaBeforeSplit / 2.0;

	/**
	 * set the diameter of both halves based on each having
	 * 1/3 the current area
	 */
	areaAfterSplit = areaBeforeSplit / 3.0;
	currentFibre->mf_diameter_ =
		newFibre->mf_diameter_ = (float) (sqrt(areaAfterSplit / M_PI) * 2.0);

	MSG_ASSERT(newFibreMuscleIndex < numBitsAllocated, "bitstring too short");


	/** record both halves as having split */
	SET_BIT((splitFibres), currentFibreIndex, 1);
	SET_BIT((splitFibres), newFibreMuscleIndex, 1);

	/** record new half as being both involved and hypertrophic */
	SET_BIT((hypertrophicFibres), newFibreMuscleIndex, 1);
	SET_BIT((involvedFibres), newFibreMuscleIndex, 1);

	return 1;
}

/**
 * Simulate Myopathic involvement by attaching the fibers
 */
int
updateMuscleWithMyopathy(
		MuscleData *MD,
		float involvement,
		float myopathicFibreDiameterMean,
		int   myopathicFibreGraduallyDying,
		int   myopathicDependentProcedure,
		int   myopathicCycleNewInvolvementPercentage,
		float myopathicFibreDeathDiameter,
		float myopathicPercentageOfAffectedFibersDying,
		float myopathicHypertrophicFibreFraction,
		float myopathicHypertrophySplitThreshold,
		float myopathicPercentageOfHypertrophicFibersSplit,
		float myopathicAtrophyRatePerCycle,
		float myopathicHypertrophyRatePerCycle
	)
{
	MuscleFibre *currentFibre;
	BITSTRING involvedFibres;
	BITSTRING hypertrophicFibres;
	BITSTRING splitFibres;
	BITSTRING stopFurtherInvolved;
	double randomChoice;
	double areaFraction, hypertrophicArea;
	double minRequiredArea; //, minRequiredDiameter;
	float Fibre25PercentDeathDiameter;
	float Fibre75PercentDeathDiameter;
	float DeathPercentageOfAffectedfibers;
	float SplitPercentageOfHypertrophicFibers;
	float desiredDeathPercentage;
	float desiredSplitPercentage;

	int check;
	int flagM;
	int numFibresKilled;
	int CountFibres75 = 0;
	int CountFibres25 = 0;
	int numFibresKilledThisCycle;
	int numFibresSplit;
	int numFibresSplitThisCycle;
	int desiredNumberOfInvolvedFibres;
	int cycleNewInvolvementCount;
	int currentNumberOfInvolvedFibres;
	int initialRandomInvolvementIndex;
	int bitstringLength;
	int fibreIndex;
	int cycleIndex;
	int status = 0;
	int i;
	int originalNumberOfFibres;

	Fibre75PercentDeathDiameter = myopathicFibreDeathDiameter + 5.0f;
	Fibre25PercentDeathDiameter = myopathicFibreDeathDiameter + 10.0f;

	flagM = 1 - myopathicDependentProcedure;

	desiredDeathPercentage = myopathicPercentageOfAffectedFibersDying;
	desiredSplitPercentage = myopathicPercentageOfHypertrophicFibersSplit;

	DeathPercentageOfAffectedfibers = 0.0;
	SplitPercentageOfHypertrophicFibers = 0.0;

	desiredNumberOfInvolvedFibres = (int)
					(MD->getTotalNumberOfFibres() * involvement) + 1;
	originalNumberOfFibres = MD->getTotalNumberOfFibres();

	/** clip down to one fibre left, so that loop will terminate */
	if (desiredNumberOfInvolvedFibres >= MD->getTotalNumberOfFibres())
	{
		desiredNumberOfInvolvedFibres = MD->getTotalNumberOfFibres() - 1;
	}

	LogInfo("    Myopathy : %s%% Muscle Involvement (%d fibres)\n",
					niceDouble(involvement * 100.0),
				desiredNumberOfInvolvedFibres);


	/** we allocate double the number required in case of splitting */
	bitstringLength = MD->getTotalNumberOfFibres() * 2;
	involvedFibres = ALLOC_BITSTRING(bitstringLength);
	ZERO_BITSTRING(involvedFibres, bitstringLength);

	hypertrophicFibres = ALLOC_BITSTRING(bitstringLength);
	ZERO_BITSTRING(hypertrophicFibres, bitstringLength);

	splitFibres = ALLOC_BITSTRING(bitstringLength);
	ZERO_BITSTRING(splitFibres, bitstringLength);

	stopFurtherInvolved = ALLOC_BITSTRING(bitstringLength);
	ZERO_BITSTRING(stopFurtherInvolved, bitstringLength);

	/** set up our counters and steps */
	numFibresKilled = 0;
	numFibresSplit = 0;
	currentNumberOfInvolvedFibres = 0;
	//cycleNewInvolvementCount = (int) (0.02 * MD->getTotalNumberOfFibres());
	//cycleNewInvolvementCount = (int) (0.01 * MD->getTotalNumberOfFibres());
	cycleNewInvolvementCount = (int) ( myopathicCycleNewInvolvementPercentage
										* MD->getTotalNumberOfFibres() )/100 + 1;


	/**
	 * a1 = pi (r1)^2
	 * r2 = sqrt(a2 / pi)
	 */
	minRequiredArea = 2.0 * M_PI * SQR(myopathicFibreDeathDiameter / 2.0);
	//minRequiredDiameter = sqrt(minRequiredArea / M_PI) * 2.0;


	/** loop until enough fibres have died */
	cycleIndex = 0;
	while (currentNumberOfInvolvedFibres < desiredNumberOfInvolvedFibres ||
			DeathPercentageOfAffectedfibers  < desiredDeathPercentage * flagM ||
			SplitPercentageOfHypertrophicFibers  < desiredSplitPercentage * flagM)
	{

		cycleIndex++;
		/**
		 * randomly choose 2% of the fibres, adding these to the
		 * bitstring describing the total set of fibres involved.
		 */
		check = 0;
		if (currentNumberOfInvolvedFibres < desiredNumberOfInvolvedFibres){
			check = 1;
			for (i = 0; i < cycleNewInvolvementCount; i++)
			{
				if (currentNumberOfInvolvedFibres >= MD->getTotalNumberOfFibres())
					break;
				initialRandomInvolvementIndex =
						fibreIndex = intRangeRandom(MD->getTotalNumberOfFibres());
				while (GET_BIT(involvedFibres, fibreIndex) != 0)
				{
					fibreIndex = (fibreIndex + 1) % MD->getTotalNumberOfFibres();
					if (fibreIndex == initialRandomInvolvementIndex)
					{
						status = 1;
						goto CLEANUP;
					}
				}

				SET_BIT(involvedFibres, fibreIndex, 1);
				currentNumberOfInvolvedFibres++;

				/** determine whether this fibre is hypertrophic or atropic */
				if (myopathicHypertrophicFibreFraction > 0.0)
				{
					randomChoice = localRandomDouble();
					if (randomChoice < myopathicHypertrophicFibreFraction)
					{
						SET_BIT(hypertrophicFibres, fibreIndex, 1);
					}
				}
			}
		}

		/**
		 * For all fibres in the involved bitstring, decrease the radius
		 * by 4%, removing those fibres which are below the
		 * death threshold
		 */
		numFibresKilledThisCycle = 0;
		numFibresSplitThisCycle = 0;
		for (i = 0; i < MD->getTotalNumberOfFibres(); i++)
		{

			/** if the fibre is involved, adjust it */
			if (GET_BIT(involvedFibres, i) != 0
					&& GET_BIT(stopFurtherInvolved,i) == 0)
			{

				currentFibre = MD->getFibre(i);
				if (currentFibre == NULL)
					continue;

				/** is this fibre hyper- or a- trophic? */
				if (GET_BIT(hypertrophicFibres, i) != 0)
				{
					/** fibres which are split do not change any more */
					int splitFlag;
					splitFlag = GET_BIT(splitFibres, i);

					if (splitFlag == 0)
					{
						currentFibre->mf_diameter_ = (float)
							(currentFibre->mf_diameter_
							* sqrt(myopathicHypertrophyRatePerCycle));

						hypertrophicArea = M_PI
							* SQR(currentFibre->mf_diameter_ / 2.0);

						areaFraction = hypertrophicArea /
							(M_PI * SQR(currentFibre->mf_healthyDiameter_ / 2.0));

						if ((areaFraction > myopathicHypertrophySplitThreshold)
									&& (hypertrophicArea > minRequiredArea)){
							if (SplitPercentageOfHypertrophicFibers
										* flagM < desiredSplitPercentage)
							{
								if ( ! splitCurrentMuscleFibre(
									MD,
									currentFibre,
									i,
									hypertrophicArea,
									involvedFibres,
									hypertrophicFibres,
									splitFibres,
									bitstringLength

									) )
								{
									goto FAIL;
								}
								numFibresSplitThisCycle++;
							}else{
								SET_BIT((stopFurtherInvolved), i,1);
							}
						}
					}
				} else
				{

					currentFibre->mf_diameter_ = (float)
						(currentFibre->mf_diameter_
						* sqrt(myopathicAtrophyRatePerCycle));

					if (DeathPercentageOfAffectedfibers
								* flagM < desiredDeathPercentage){
						if (myopathicFibreGraduallyDying == 0){
							if (currentFibre->mf_diameter_
									< myopathicFibreDeathDiameter)
							{
								MD->motorUnit_[
										currentFibre->mf_motorUnit_-1
									]->removeFibre( currentFibre);

								MD->masterFibreList_[i] = NULL;
								delete currentFibre;
								numFibresKilledThisCycle++;

							}
						} else
						{
							/** fibres gradually dying--probability-based */
							if (currentFibre->mf_diameter_
									< myopathicFibreDeathDiameter)
							{

								MD->motorUnit_[
									currentFibre->mf_motorUnit_-1
									]->removeFibre( currentFibre);

								MD->masterFibreList_[i] = NULL;
								delete currentFibre;
								numFibresKilledThisCycle++;
							} else
							{
								if (currentFibre->mf_diameter_
										< Fibre75PercentDeathDiameter)
								{
									CountFibres75++;
									if (CountFibres75==4)
									{
										CountFibres75 = 0;
									} else
									{
										MD->motorUnit_[
												currentFibre->mf_motorUnit_-1
											]->removeFibre( currentFibre);
										MD->masterFibreList_[i] = NULL;
										delete currentFibre;
										numFibresKilledThisCycle++;
									}
								} else
								{
									if (currentFibre->mf_diameter_
										< Fibre25PercentDeathDiameter){
										CountFibres25++;
										if (CountFibres25==4){
											MD->motorUnit_[
												currentFibre->mf_motorUnit_-1
												]->removeFibre( currentFibre);
											MD->masterFibreList_[i] = NULL;
											delete currentFibre;
											numFibresKilledThisCycle++;
											CountFibres25 = 0;
										}
									}
								}
							}
						}

					} else
					{
						if (currentFibre->mf_diameter_
								< myopathicFibreDeathDiameter)
						{
							SET_BIT((stopFurtherInvolved), i,1);
						}
					}
				}
			}
		}


		numFibresKilled += numFibresKilledThisCycle;
		numFibresSplit += numFibresSplitThisCycle;

		DeathPercentageOfAffectedfibers =
			(((float) numFibresKilled * 100.0f)
							/ (float) MD->getTotalNumberOfFibres()
								/ (float) involvement);

		SplitPercentageOfHypertrophicFibers = (((float) numFibresSplit * 100.0f)
						/ (float) originalNumberOfFibres
								/ (float) myopathicHypertrophicFibreFraction
								/ (float) involvement);

		LogInfo("    Myopathy progression cycle %d\n", cycleIndex);
		LogInfo("             %d fibres added to involvment this cycle\n",
						check*cycleNewInvolvementCount);

		LogInfo("             %d fibres involved of %d (%g of total)\n",
						currentNumberOfInvolvedFibres,
						MD->getTotalNumberOfFibres(),
						currentNumberOfInvolvedFibres
								/ (double) MD->getTotalNumberOfFibres());

		LogInfo("             %d fibres died this cycle\n",
						numFibresKilledThisCycle);
		LogInfo("             %d fibres died so far (%g of total involved fibers)\n",
						numFibresKilled,
						numFibresKilled / (float) currentNumberOfInvolvedFibres);

		LogInfo("             %d fibres split this cycle\n",
						numFibresSplitThisCycle);
		LogInfo("             %d fibres split so far (%g of total involved fibres)\n",
						numFibresSplit,
						numFibresSplit / (float) currentNumberOfInvolvedFibres);

	}

	/** Using the original number of fibres */
	LogInfo("\t Regarding the original number of fibres in the muscle\n");

	LogInfo(
		"    Myopathy : %d of %d (%5.1f%%) fibres died with diameter < %f\n",
			numFibresKilled, originalNumberOfFibres,
			(((float) numFibresKilled * 100.0f)
							/ (float) originalNumberOfFibres),
			myopathicFibreDeathDiameter);

	LogInfo(
		"    Myopathy : %5.1f%% of affected fibres died\n",
			(((float) numFibresKilled * 100.0f)
							/ (float) originalNumberOfFibres
								/ (float) involvement));

	LogInfo(
		"    Myopathy : %d of %d (%5.1f%%) fibres split\n",
			numFibresSplit, originalNumberOfFibres,
			(((float) numFibresSplit * 100.0f)
							/ (float) originalNumberOfFibres));

	LogInfo(
		"    Myopathy : %5.1f%% of hypertrophic fibres split\n",
			(((float) numFibresSplit * 100.0f)
						/ (float) originalNumberOfFibres
								/ (float) myopathicHypertrophicFibreFraction
								/ (float) involvement));

	/** using the updated number of fibres */
   /* LogInfo("\t Regarding the updated number of fibres in the muscle\n");

	LogInfo(
		"    Myopathy : %d of %d (%5.1f%%) fibres died with diameter < %f\n",
			numFibresKilled, MD->getTotalNumberOfFibres(),
			(((float) numFibresKilled * 100.0f)
							/ (float) MD->getTotalNumberOfFibres()),
			myopathicFibreDeathDiameter);

	LogInfo(
		"    Myopathy : %d of %d (%5.1f%%) fibres split\n",
			numFibresSplit, MD->getTotalNumberOfFibres(),
			(((float) numFibresSplit * 100.0f)
							/ (float) MD->getTotalNumberOfFibres()));
	LogInfo(
		"    Myopathy : %5.1f%% of hypertrophic fibres split\n",
			(((float) numFibresSplit * 100.0f)
						/ (float) MD->getTotalNumberOfFibres()
								/ (float) myopathicHypertrophicFibreFraction
								/ (float) involvement));
	LogInfo(
		"    Myopathy : %5.1f%% of affected fibres died\n",
			(((float) numFibresKilled * 100.0f)
							/ (float) MD->getTotalNumberOfFibres()
								/ (float) involvement));
	*/

	status = 1;

CLEANUP:
FAIL:
	FREE_BITSTRING(involvedFibres);
	FREE_BITSTRING(hypertrophicFibres);
	FREE_BITSTRING(splitFibres);
	FREE_BITSTRING(stopFurtherInvolved);
	return status;
}

///**
// * Simulate Neuropathic involvement by removing a fraction of
// * the neurons
// */
//int
//updateMuscleWithMyopathy(
//		MuscleData *MD,
//		float involvement,
//		float myopathicFibreDiameterMean,
//		float myopathicFibreDeathDiameter
//    )
//{
//    MuscleFibre *currentFibre;
//    float adjustAmount;
//    int numFibresDied = 0;
//    int i;
//
//    adjustAmount = (float)
//				((FIBRE_DIAMETER_MEAN - myopathicFibreDiameterMean) *
//						involvement);
//
//
//
//    LogInfo("    Myopathy : %s involvement\n", niceDouble(involvement));
//    LogInfo("    Myopathy :    Normal Mean : %.3f\n", FIBRE_DIAMETER_MEAN);
//    LogInfo("    Myopathy : Myopathic Mean : %.3f\n",
//					myopathicFibreDiameterMean);
//    LogInfo(
//    "    Myopathy : Decreasing fibre diameter by %.3f for mean of %.3fum\n",
//			adjustAmount,
//			myopathicFibreDiameterMean);
//
//
//    /**
//     * decrease the diameter of the fibres, deleting those below
//     * a threshold
//     */
//    for (i = 0; i < MD->getTotalNumberOfFibres(); i++)
//    {
//		currentFibre = MD->getFibre(i);
//		if (currentFibre == NULL)
//			continue;
//		currentFibre->mf_diameter_ -= adjustAmount;
//		if (currentFibre->mf_diameter_ < myopathicFibreDeathDiameter)
//		{
//			MD->motorUnit_[
//						currentFibre->mf_motorUnit_-1
//					]->removeFibre( currentFibre);
//			MD->masterFibreList_[i] = NULL;
//			delete currentFibre;
//			numFibresDied++;
//		}
//    }
//
//    LogInfo(
//		"    Myopathy : %d of %d (%5.1f%%) fibres died with diameter < %f\n",
//			numFibresDied, MD->getTotalNumberOfFibres(),
//			(((float) numFibresDied * 100.0f) / (float) MD->getTotalNumberOfFibres()),
//			myopathicFibreDeathDiameter);
//
//    return 1;
//}

