/**
 ** Define structures used throughout the simulator
 **
 ** $Id: SimulatorControl.h 27 2020-09-25 20:00:32Z andrew $
 **/

#ifndef __SIMULATOR_CONTROL_HEADER__
#define __SIMULATOR_CONTROL_HEADER__

#include "os_defs.h"

#include "mathtools.h"
#include "attvalfile.h"

#include "muscle.h"


#define __PROG_VERSION__			"Version 2.2 (emglab)"
#define __PROG_VERSION_LEN__		24


#define DEFAULT_DRIVE				"c:"

#define MIN_ALLOWED_MU_DENSITY		6.0f
#define MAX_ALLOWED_MU_DENSITY		16.0f

#define MIN_NEEDLE_INSERTION_DISTANCE_MM		1.5f

struct file_description_data
{
	char operator_name[FILENAME_MAX];
	char patient_name[FILENAME_MAX];
	char muscle_description[FILENAME_MAX];
	int patient_id;
	int muscle_side;
	int new_muscle;
	int new_operator;
	int new_patient;
};


class MuscleData;
class SimulationResult;

/********* GLOBAL STRUCTURE 'g' **************/
typedef struct globals
{
	/* version string */
	char version_string[__PROG_VERSION_LEN__];

	/* list of all global attributes */
	attValList*list_;


	/* this is where all the output stuff below will go */
	char output_stem[FILENAME_MAX];


	/* 2nd level data directory muscle directory */
	char muscle_dir_sub[FILENAME_MAX];

	/* 3rd level data directory firing times directory */
	char firings_dir_sub[FILENAME_MAX];

	/* 3rd level data  directory MUPs  directory */
	char MUPs_dir_sub[FILENAME_MAX];

	/* 3rd level data  directory emg directory */
	char output_dir_sub[FILENAME_MAX];


	/* 2nd level data directory muscle directory */
	char *muscle_dir;
	/* 3rd level data directory firing times directory */
	char *firings_dir;
	/* 3rd level data  directory MUPs  directory */
	char *MUPs_dir;
	/* 3rd level data  directory emg directory */
	char *output_dir;

	/* patient directory name */
	char patient_name[FILENAME_MAX];


	/* Global Sampling Parameters */

	/* first sample location to be loaded */
	int first_samp;     
	/* sampling frequency for MUP (kHz) */
	float smpling_freq;

	/** definitions for the muscle routines */
	MuscleParameters		*muscle_;

	int   muscleLayoutFunctionType;

	int   read_fib_dat;  /* Y or N */
	int   write_fib_dat; /* Y or N write output */

	/* Global Firing Parameters  */
	FiringParameters		firing_;

	/* Global MUP parameters  */
	int   electrode_type;  /** between 1 and 3 */
	int   tipUptakeDistance;
	int   canUptakeDistance;
	int   canPhysicalRadius;
	int   MUPs_per_mu;
	int   needleReferenceSetup;

	/* jitter variance in milliseconds */
	int   doJitter;
	int   super_jitter_seeds;
	float jitterAccelThreshold;
	int   jitter;

	int seekNeedle;
	float minimumMuscleMetricThreshold;

	float significantFibreAccelerationThreshold;

	/* standard deviation in 50 um */
	float stddev_x;
	float stddev_y;
	float stddev_z;

	/** needle position (x,y from center of muscle, z from NMJ) */
	float   needle_x_position;
	float   needle_y_position;
	float   needle_z_position;

	/**
	 * length of the cannula from the tip to the end of
	 * the modelled length
	 */
	float   cannula_length;

	/* (ms) needle move time constant */
	int   needle_TC;

	/* time index to stop EMG generation */
	int   emg_elapsed_time;

	/* amount of noise to place in the signal */
	int   use_noise;
	float signalToNoiseRatio;

	short maxShortVoltage;

	int   jitterInterpolationExpansion;

	struct PathologyControl     pathology;

	struct file_description_data fileDescription;

	int   use_last_muscle;
	int   use_old_firing_times;
	int   filter_raw_signal;

	int	  generateMFPsWithoutInitiation;

	int   myopathicFibreGraduallyDying;

	int   myopathicDependentProcedure;

	int   myopathicCycleNewInvolvementPercentage;

	int		  recordMFPPeakToPeak;

	int   mu_layout_type;

	/** generate 2nd channel */
	int   generate_second_channel;

	int   text_output;
} SimulationControl;

/* Global Parameters Structure */
extern struct globals *g;

int setGlobalDefaultValues(struct globals *globalValues);
void  clearGlobalPointers(void);
void deleteGlobals(void);



/***** END OF FUNCTION PROTOTYPES *********/
#endif /* __SIMULATOR_CONTROL_HEADER__ */


