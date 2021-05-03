/**
 ** Routines used in the convolution tools file
 **
 ** $Id: MUP_utils.h 5 2008-04-24 22:05:30Z andrew $
 **/

#ifndef __MFP_GENERATION_TOOLS_HEADER__
#define __MFP_GENERATION_TOOLS_HEADER__

#include "MUP.h"

class NeedleInfo;

#define  BIPOLE_SEP             0.200           /* mm */

typedef struct MUPControlStruct {
		int              MUPsPerMu;
		const char      *firingsDirectory;
		const char      *MUPDirectory;
		const char      *muscleDirectory;
		int             MUPLength;
		int             electrodeType;
		int             tipUptakeDistanceInMicrons;
		int             canUptakeDistanceInMicrons;
		float   stdDev_X;
		float   stdDev_Y;
		float   stdDev_Z;
} MUPControl;


/* muscle fibre data */
struct mscl_fibr_record {
		int x;          /* distance relative to centre of */
		int y;          /* detection area (50 micron units) */
		float diam;     /* fibre diameter (in microns) */
		float jshift;   /* z - distance from endplate */
};


int makeMUP(
		        MuscleData *muscleDefinition,
		        int **loadMUPVector,
		        int *nMUPs
		);

int loadMUPs(
		        int **loadMUPIdVector,
		        int totalNumberOfMUPs
		);

int generateAllMUPs(
		        MuscleData *muscleDefinition,
		        MUPControl *control,
		        int **loadMUPIdVector,
		        int *nMUPs
		);

int statFilenameFromMask(
				const char *directory,
				const char *filenameMask,
				...
		);

#endif


