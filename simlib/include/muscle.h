/**
 * Define values to be used in the generation of the muscle.
 * There will be a single control structure produced based
 * on the degree of involvement of disease
 *
 * $Id: muscle.h 11 2011-03-18 12:08:51Z andrew $
 */

#ifndef __MUSCLE_DEFINITION_HEADER__
#define __MUSCLE_DEFINITION_HEADER__

#include "os_defs.h"
#include "MuscleData.h"

struct Node;

/** a fibre layout function pointer */
typedef double (MFL_WeightingFunction)(
				float *totalDistanceToChosenCentroid,
				MuscleData *MD,
				float xLocationInMM,
				float yLocationInMM,
				int motorUnitID,
				int nInIntersectionList,
				int *muIntersectionList,
				double denominator
	);

/** a fibre layout function pointer */
typedef double (MFL_WeightingDenominatorFunction)(
				MuscleData *MD,
				float xLocationInMM,
				float yLocationInMM,
				int nInIntersectionList,
				int *muIntersectionList
	);

typedef int (MFL_ProbabilityFunction)(
				float *totalDistanceToChosenCentroid,
				MuscleData *MD,
				float xLocationInMM,
				float yLocationInMM,
				int *rTreeIdListIndexChoice,
				int nInIntersectionList,
				int *muIntersectionList
	);



#define		FIBRE_DIAMETER_MEAN				55.0
#define		FIBRE_DIAMETER_STD_DEV				9.0

/**
 * Muscle definition structure
 */
struct MuscleParameters
{
public:
	int numMotorUnits;
	float fibreDensity;
	float areaPerFibre;
	float minMUDiam;
	float maxMUDiam;
//    float rangeOfTwitchTensions;
	int modelLayoutDistanceInUM;
//    float fibreAssignmentFracByWeighting;
	int numFibreLayoutProbabilityFunctions;
	MFL_ProbabilityFunction **fibreLayoutProbabilityFunctionList;
	float *fibreLayoutProbabilityFunctionProbabilities;

	int numFibreLayoutWeightingFunctions;
	MFL_WeightingFunction **fibreLayoutWeightingFunctionList;
	MFL_WeightingDenominatorFunction
						**fibreDenominatorWeightingFunctionList;
	float *fibreLayoutWeightingFunctionWeights;

	float fibreLayoutWeightingNoiseFactor;

public:
	MuscleParameters();
	~MuscleParameters();
	void setMuscleParamDefaultValues(int nMotorUnits);
};

struct FiringParameters {
	float coefficientOfVarianceInFiringTimes;
	float contractionLevelAsPercentMVC;
	float recruitmentSlope;
	float minimumFiringRate;
	float maximumFiringRate;
	float maximumFiringThreshold;
};


typedef struct PathologyControl
{
		float neuropathicMULossFraction;
		int   neuropathicMaxAdoptionDistanceInUM;
		float neuropathicEnlargementFraction;

		int   myopathicCycleNewInvolvementPercentage;
		float myopathicFractionOfFibresAffected;
		float myopathicFibreDiameterMean;
		float myopathicFibreDeathDiameter;
		float myopathicPercentageOfAffectedFibersDying;
		int   myopathicFibreGraduallyDying;
		int   myopathicDependentProcedure;
		float myopathicHypertrophicFibreFraction;
		float myopathicHypertrophySplitThreshold;
		float myopathicPercentageOfHypertrophicFibersSplit;
		float myopathicAtrophyRatePerCycle;
		float myopathicHypertrophyRatePerCycle;
} PathologyControl;

#define		MYOPATHY_ATROPHY_FRACTION_PER_CYCLE		0.96
#define		MYOPATHY_HYPERTROPHY_FRACTION_PER_CYCLE		1.04

MuscleParameters *getMuscleParameters(
				int baseNumberOfMotorUnits,
				float neuropathicInvolvement = 0,
				float myopathicInvolvement = 0
			);


struct rTreeResultList {
	int *results_;
	int nEntries_;
	int nBlocks_;
};

/**
 * The minimum number of MU's which remain in a neuropathy
 */
#define NEURO_MIN_NUM_MUS		1

/**
 * Layout controls
 */
#define RANDOM_MU_LAYOUT        0x00
#define GRID_MU_LAYOUT          0x01


/** muscle layout related functions */
MuscleData *muscle(
		int fileId,
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
	);
MuscleData *loadMuscleData(
		int emgFileId,
		const char *muscleDirectory,
		const char *outputDirectory,
		int usePlowedFibres = 0
	);

int FibreRTreeSearchCallback__(long id, void *resultList);
int buildFibreRTree(MuscleData *MD);

int setAndClipNeedleLocation(
		MuscleData *MD,
		float muscleRadius,
		float needleX, float needley, float needleZ,
		float cannulaLength, float cannulaRadius
	);

int storeNeedleInfo(
		MuscleData *MD,
		int emgFileId,
		const char *directory
	);

int seekNeedleToNearbyFibres(
		MuscleData *MD,
		float minFibreMovementMetricThreshold
	);

int plowMuscleFibres(
		int emgFileId,
		const char *outputDir,
		MuscleData *MD,
		float canPhysicalRadius
	);

int updateMuscleWithNeuropathy(
		MuscleData *MD,
		float involvement,
		int maxAdoptionDistanceInCells,
		float neuropathicEnlargementFraction
	);

int updateMuscleWithMyopathy(
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
	);

/** firing related functions */
int firing(
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
	);
int loadFiring(
		MuscleData *muscleDataDefinition,
		const char *firingsDirectory
	);

/** EMG generation functions */
int makeEmg(MuscleData *muscleDataDefinition, int fileId);
int create_next_dir(
				char *path,
				char *dirmask,
				char *newpath,
				char *newemg_fn
			);

/**
 * Query and use the fibre layout function set
 */

#define				LAYOUT_PROBABILITY				0x01
#define				LAYOUT_WEIGHTING				0x02

/** find out what functions are defined */
OS_EXPORT int getNumFibreLayoutWeightingFunctions();
OS_EXPORT MFL_WeightingFunction
				*getFibreLayoutWeightingFunction(int index);
OS_EXPORT MFL_WeightingDenominatorFunction
				*getFibreLayoutWeightingDenominatorFunction(int index);
OS_EXPORT float getFibreLayoutWeightingFunctionWeighting(int index);
OS_EXPORT float getFibreLayoutWeightingFunctionProbability(int index);
OS_EXPORT const char * getFibreLayoutWeightingFunctionDescription(int index);
OS_EXPORT const char * getFibreLayoutWeightingFunctionName(int index);

/** find out what functions are defined */
OS_EXPORT int getNumFibreLayoutProbabilityFunctions();
OS_EXPORT MFL_ProbabilityFunction
				*getFibreLayoutProbabilityFunction(int index);
OS_EXPORT float getFibreLayoutProbabilityFunctionWeighting(int index);
OS_EXPORT float getFibreLayoutProbabilityFunctionProbability(int index);
OS_EXPORT const char * getFibreLayoutProbabilityFunctionDescription(int index);
OS_EXPORT const char * getFibreLayoutProbabilityFunctionName(int index);

#endif /* __MUSCLE_DEFINITION_HEADER__ */

