/**
 ** Define structures used throughout the simulator
 **
 ** $Id: SimulatorConstants.h 4 2008-04-24 21:27:41Z andrew $
 **/

#ifndef __SIMULATOR_CONSTANTS_HEADER__
#define __SIMULATOR_CONSTANTS_HEADER__

/** time in ms between MUP samples */
#define DELTA_T_MUP								0.032

/** time in ms between EMG  samples */
#define DELTA_T_EMG								0.032   

/** Length of a MUP buffer */
#define MUP_LENGTH								2048

/**
 * acceleration threshold for MUP's placed in GST file
 * in V/ss
 */
#define GST_ACCEL_THRESHOLD						12.50

/** time in ms between firing time samples */
#define DELTA_T_FIRING_TIMES						.1


#define MAX_GENERATION_TIME_IN_SECONDS				60


#define MAX_MUP_LEN								4096


/** Datalog file names */
#define MFPP2PLOGFILE_NAME						"MFP-p2p.txt"
#define MUDIAMETERLOGFILE_NAME						"MU-diameter-changes.txt"

/***** END OF FUNCTION PROTOTYPES *********/
#endif /* __SIMULATOR_CONSTANTS_HEADER__ */

