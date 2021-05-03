/**
 ** This class holds the definition of a muscle data structure
 **
 ** $Id: MuscleData.cpp 13 2011-03-23 11:22:44Z andrew $
 **/

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "tclCkalloc.h"
#include "stringtools.h"
#include "mathtools.h"
#include "pathtools.h"
#include "listalloc.h"
#include "massert.h"
#include "log.h"
#include "msgir.h"

#include "rTreeIndex.h"

#define PRIVATE public

#include "MuscleData.h"
#include "NeedleInfo.h"
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


const int MuscleData::PRINT_MU_DATA				= 0x01;
const int MuscleData::PRINT_FIBRE_DATA				= 0x02;
const int MuscleData::PRINT_FULL_MU_LIST_DATA		= 0x04;

const int MuscleData::PRINT_ALL_DATA =
		MuscleData::PRINT_MU_DATA				|
		MuscleData::PRINT_FIBRE_DATA				|
		MuscleData::PRINT_FULL_MU_LIST_DATA;


MuscleData::MuscleData()
{
	xDetect_ = 0;
	yDetect_ = 0;

	nMotorUnitsInMuscle_ = 0;
	motorUnit_ = NULL;

	nMotorUnitsInDetectionArea_ = 0;
	muscleDiameter_ = 0;
	maxMotorUnitDiameter_ = 0;
	minMotorUnitDiameter_ = 0;

	motorUnitInDetect_ = NULL;
	motorUnitInDetectBlocks_ = 0;

	activeMotorUnit_ = NULL;
	nActiveMotorUnits_ = 0;
	nActiveMotorUnitBlocks_ = 0 ;

	activeInDetectMotorUnit_ = NULL;
	nActiveInDetectMotorUnits_ = 0;
	nActiveInDetectMotorUnitBlocks_ = 0;

	needle_ = NULL;
	fibreRTreeRoot_ = NULL;

	nTotalFibres_ = 0;
	nFibreBlocks_ = 0;
	masterFibreList_ = NULL;
	nMaxFibres_ = 0;
}

MuscleData::~MuscleData()
{
	int i;

	validate();

	if (nMotorUnitsInMuscle_ > 0)
	{
		if (motorUnit_ != NULL)
		{
			for (i = 0; i < nMotorUnitsInMuscle_; i++)
			{
				if (motorUnit_[i] != NULL)
				{
					delete motorUnit_[i];
					motorUnit_[i] = NULL;
				}
			}
			ckfree(motorUnit_);
			motorUnit_ = NULL;
		}
	}

	if (motorUnitInDetect_ != NULL)
		ckfree(motorUnitInDetect_);

	if (activeMotorUnit_ != NULL)
		ckfree(activeMotorUnit_);

	if (activeInDetectMotorUnit_ != NULL)
		ckfree(activeInDetectMotorUnit_);

	if (fibreRTreeRoot_ != NULL)
		RTreeDeleteIndex(fibreRTreeRoot_);

	if (masterFibreList_ != NULL)
		ckfree(masterFibreList_);

	if (needle_ != NULL)
		delete needle_;
}

int
MuscleData::validate() const
{
	int status = 1;
	int i;

//    LogInfo(" --------------------------------------------------------\n");
//    LogInfo(" Validating Muscle with %d Fibres, %d MUs\n",
//					nTotalFibres_, nMotorUnitsInMuscle_);
//    LogInfo(" --------------------------------------------------------\n");
//
	for (i = 0; i < nTotalFibres_; i++)
	{
		if (masterFibreList_[i] == NULL)
			continue;

		status = masterFibreList_[i]->validate() && status;
	}


	for (i = 0; i < nMotorUnitsInMuscle_; i++)
	{
		if (motorUnit_[i] == NULL)
		{
//			LogWarn("Motor Unit %d is NULL\n", i);
			continue;
		}

		status = motorUnit_[i]->validate() && status;
	}

	return status;
}

int
MuscleData::readMotorUnit(
		FILE *fp,
		MotorUnit **masterMotorUnitList
	)
{
	MotorUnit *target = NULL;
	MuscleFibre *currentFibre = NULL;
	char inputLine[4096];
	int motorUnitId = 0;
	int motorUnitIndex;
	int readState = 0;
	int done = 0;
	int i = 0;


	while (( ! done ) && fgets(inputLine, 4096, fp) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		switch (readState)
		{
		case 0:
			/** read in the MU header line */
			if (sscanf(inputLine, "id: %d", &motorUnitId) != 1)
			{
				LogDebug(__FILE__, __LINE__,
					"Failure reading MU ID from:\n  %s\n", inputLine);
				return (-1);
			}
			motorUnitIndex = motorUnitId - 1;
			target = masterMotorUnitList[motorUnitIndex];
			if (target == NULL)
			{
				LogDebug(__FILE__, __LINE__,
					"Internal error finding motor unit %d (id %d)\n",
					motorUnitIndex, motorUnitId);
				return (-1);
			}
			if (target->mu_id_ != motorUnitId)
			{
				LogDebug(__FILE__, __LINE__,
					"Internal error -- ID mismatch on MU %d (id %d)\n",
					motorUnitIndex, motorUnitId);
				return (-1);
			}
			readState = 1;
			break;

		case 1:
			/**
			 * read in the MU description defining the number of fibres
			 */
			if (sscanf(inputLine, "%d %f %f %f",
						&target->mu_nFibres_,
						&target->mu_loc_r_mm_,
						&target->mu_loc_theta_,
						&target->mu_diameter_mm_) != 4)
			{
				LogDebug(__FILE__, __LINE__,
					"Failure reading MU data from:\n  %s\n", inputLine);
				return (-1);
			}

			target->mu_nHealthyFibres_ = (-1);


			/** if we have fibres, then set state to read 'em */
			if (target->mu_nFibres_ > 0)
			{
				target->mu_fibre_ = (MuscleFibre **)
						ckalloc(sizeof(MuscleFibre *)
								* target->mu_nFibres_);
				memset(target->mu_fibre_, 0, sizeof(MuscleFibre *)
								* target->mu_nFibres_);
				readState = 2;
			} else
			{
				/** otherwise we are done this MU */
				done = 1;
			}
			break;

		case 2:
			/**
			 * create a new fibre and add it to the MU and master list
			 */
			if (i >= target->mu_nFibres_)
			{
				LogError("Too many ( > %d )fibres for MU %d\n",
								target->mu_nFibres_, target->mu_id_);
				return (-1);
			}
			currentFibre = new MuscleFibre();
			target->mu_fibre_[i++] = currentFibre;
			currentFibre->mf_motorUnit_ = target->mu_id_;
			if (sscanf(inputLine, "%f %f",
						&currentFibre->mf_xCell_,
						&currentFibre->mf_yCell_) != 2)
			{
				LogDebug(__FILE__, __LINE__,
					"Failure reading fibre cell from:\n  %s\n", inputLine);
				return (-1);
			}

			/** add the fibre to the master list too */
			addFibre(currentFibre);

			/** go to adding diameter and shift from next line */
			readState = 3;
			break;


		case 3:
			/** fill in fibre diameter and shift */
			if (sscanf(inputLine, "%f %f",
						&currentFibre->mf_diameter_,
						&currentFibre->mf_jShift_) != 2)
			{
				LogDebug(__FILE__, __LINE__,
					"Failure reading fibre size/shift from:\n  %s\n",
					inputLine);
				return (-1);
			}

			/** determine if we are done */
			if (i == target->mu_nFibres_)
				done = 1;

			/** loop back for next fibre if not done */
			readState = 2;
		}
	}

	if (target != NULL && i < target->mu_nFibres_)
	{
		LogError("Too few fibres for MU %d\n", target->mu_id_);
		return (-1);
	}

	return motorUnitId;
}

/**
 ** Allocate a set of MotorUnits
 **/
void MuscleData::allocateNMotorUnits(int nMotorUnits)
{
	int i;

	if (motorUnit_ != NULL)
	{
		for (i = 0; i < nMotorUnitsInMuscle_; i++)
		{
			delete motorUnit_[i];
		}
		ckfree(motorUnit_);
	}

	nMotorUnitsInMuscle_ = nMotorUnits;

	motorUnit_ = (MotorUnit **) ckalloc(sizeof(MotorUnit *)
				* nMotorUnitsInMuscle_);
	memset(motorUnit_, 0, sizeof(MotorUnit *) * nMotorUnitsInMuscle_);


	for (i = 0; i < nMotorUnitsInMuscle_; i++)
	{
		motorUnit_[i] = new MotorUnit();
		motorUnit_[i]->mu_id_ = i + 1;
	}
}

int MuscleData::loadAMUdata(const char *amuFilename)
{
	char inputLine[4096];
	FILE *amuFP;
	int readHeader = 0;
	int tmpMotorUnitId;
	int muActiveIndex = 0;


	/*
	 * Load the list of active motor units
	 */
	amuFP = fopenpath(amuFilename, "rb");
	if (amuFP == NULL)
	{
		LogError("Unable to open %s\n", amuFilename);
		return 0;
	}


	while (fgets(inputLine, 4096, amuFP) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		if ( ! readHeader )
		{
			if (sscanf(inputLine, "%d", &nActiveMotorUnits_) != 1)
			{
				LogDebug(__FILE__, __LINE__,
					"Failure reading num active MU's from:\n  %s\n",
					inputLine);
				return 0;
			}

			/**
			 * allocate the list for Active MU's.  The
			 * actual MU's must already have been loaded
			 * by a call to loadMUdata
			 */
			activeMotorUnit_ = (MotorUnit **)
						ckalloc(sizeof(MotorUnit *)
								* nActiveMotorUnits_);
			memset(activeMotorUnit_, 0, sizeof(MotorUnit *)
								* nActiveMotorUnits_);
			readHeader = 1;
		} else
		{

			if (muActiveIndex < nActiveMotorUnits_)
			{
				/**
				 * create a (blank) motor unit and place it
				 * in both the master and detection area
				 * lists.  The master list will be used
				 * to find this MU when reading the MF.dat
				 * file in order to populate it with the
				 * appropriate values
				 */
				if (sscanf(inputLine, "%d", &tmpMotorUnitId) != 1)
				{
					LogDebug(__FILE__, __LINE__,
							"Failure reading MU id from:\n  %s\n",
							inputLine);
					return 0;
				}
				activeMotorUnit_[muActiveIndex]
								= motorUnit_[tmpMotorUnitId - 1];

				if (activeMotorUnit_[muActiveIndex] == NULL)
				{
					LogDebug(__FILE__, __LINE__,
						"Internal error -- no master MU found for id %d\n",
						tmpMotorUnitId);
					return 0;
				}

				muActiveIndex++;
			} else
			{
				LogError("Too many lines in '%s'\n", amuFilename);
				return 0;
			}
		}
	}

	if (muActiveIndex < nActiveMotorUnits_)
	{
		LogError("Too few lines in '%s'\n", amuFilename);
		return 0;
	}
	return 1;
}


int MuscleData::loadAMUInDetectData(const char *amuFilename)
{
	char inputLine[4096];
	FILE *amuFP;
	int readHeader = 0;
	int tmpMotorUnitId;
	int muActiveIndex = 0;


	/*
	 * Load the list of active motor units
	 */
	amuFP = fopenpath(amuFilename, "rb");
	if (amuFP == NULL)
	{
		LogError("Unable to open %s\n", amuFilename);
		return 0;
	}


	while (fgets(inputLine, 4096, amuFP) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		if ( ! readHeader )
		{
			if (sscanf(inputLine, "%d", &nActiveInDetectMotorUnits_) != 1)
			{
				LogDebug(__FILE__, __LINE__,
						"Failure reading num MU's in detect from:\n  %s\n",
						inputLine);
				return 0;
			}

			/**
			 * allocate the list for Active MU's.  The
			 * actual MU's must already have been loaded
			 * by a call to loadMUdata
			 */
			activeInDetectMotorUnit_ = (MotorUnit **)
						ckalloc(sizeof(MotorUnit *)
								* nActiveInDetectMotorUnits_);
			memset(activeInDetectMotorUnit_, 0, sizeof(MotorUnit *)
								* nActiveInDetectMotorUnits_);
			readHeader = 1;
		} else
		{

			if (muActiveIndex < nActiveInDetectMotorUnits_)
			{
				/**
				 * create a (blank) motor unit and place it
				 * in both the master and detection area
				 * lists.  The master list will be used
				 * to find this MU when reading the MF.dat
				 * file in order to populate it with the
				 * appropriate values
				 */
				if (sscanf(inputLine, "%d", &tmpMotorUnitId) != 1)
				{
					LogDebug(__FILE__, __LINE__,
						"Failure reading MU id from:\n  %s\n",
						inputLine);
					return 0;
				}
				activeInDetectMotorUnit_[muActiveIndex]
								= motorUnit_[tmpMotorUnitId - 1];

				if (activeInDetectMotorUnit_[muActiveIndex] == NULL)
				{
					LogDebug(__FILE__, __LINE__,
						"Internal error -- no master MU found for %d\n",
						tmpMotorUnitId);
					return 0;
				}

				muActiveIndex++;
			} else
			{
				LogError("Too many lines in '%s'\n", amuFilename);
				return 0;
			}
		}
	}

	if (muActiveIndex < nActiveInDetectMotorUnits_)
	{
		LogError("Too few lines in '%s'\n", amuFilename);
		return 0;
	}
	return 1;
}


int MuscleData::loadMFdata(const char *filename)
{
	char inputLine[4096];
	FILE *fp;
	int newMUid = 0;
	int i;

	if (motorUnit_ == NULL)
	{
		LogDebug(__FILE__, __LINE__,
				"loadMUdata must be called before loadMFdata\n");
		return 0;
	}

	fp = fopenpath(filename, "r");
	if (fp == NULL)
	{
		return 0;
	}

	while (fgets(inputLine, 4096, fp) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		if (sscanf(inputLine, "%d %d", &xDetect_, &yDetect_) != 2)
		{
			LogDebug(__FILE__, __LINE__,
						"Failure reading X/Y detect from:\n  %s\n",
						inputLine);
			return 0;
		}
		break;
	}

	while (fgets(inputLine, 4096, fp) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		if (sscanf(inputLine, "%d", &nMaxFibres_) != 1)
		{
			LogDebug(__FILE__, __LINE__,
						"Failure reading num max fibres detect from:\n  %s\n",
						inputLine);
			return 0;
		}
		break;
	}

	for (i = 0; i < nMotorUnitsInMuscle_; i++)
	{
		newMUid = readMotorUnit(fp, motorUnit_);
		if (newMUid < 0)
		{
			LogCrit("Fatal error\n");
			return 0;
		}

		if (newMUid == 0)
		{
			break;
		}

		if (motorUnit_[newMUid - 1] == NULL ||
				motorUnit_[newMUid - 1] != motorUnitInDetect_[i])
		{
			LogDebug(__FILE__, __LINE__,
				"MU in detect not equal to master MU\n");
			return 0;
		}
	}
	LogInfo("Read %d motor Units from file '%s'\n", i, filename);

	return 1;
}

int MuscleData::loadMUdata(const char *filename)
{
	char inputLine[4096];
	FILE *fp;
	int readHeader = 0;
	int tmpMotorUnitId;
	int muInDetectIndex = 0;


	fp = fopenpath(filename, "r");
	if (fp == NULL)
	{
		return 0;
	}

	while (fgets(inputLine, 4096, fp) != NULL)
	{
		if (inputLine[0] == '#')
			continue;

		if ( ! readHeader )
		{
			if (sscanf(inputLine, "%d %f %f %f %d",
						&nMotorUnitsInMuscle_,
						&muscleDiameter_,
						&minMotorUnitDiameter_,
						&maxMotorUnitDiameter_,
						&nMotorUnitsInDetectionArea_) != 5)
			{
				LogDebug(__FILE__, __LINE__,
						"Failure reading muscle data from:\n  %s\n",
						inputLine);
				return 0;
			}


			/**
			 * motorUnit_ is the master list, which we can
			 * index through id.  motorUnitInDetect_ is
			 * an ordered put partial list of those units
			 * in the master list
			 */
			motorUnit_ = (MotorUnit **)
						ckalloc(sizeof(MotorUnit *)
								* nMotorUnitsInMuscle_);
			memset(motorUnit_, 0, sizeof(MotorUnit *)
								* nMotorUnitsInMuscle_);

			motorUnitInDetect_ = (MotorUnit **)
						ckalloc(sizeof(MotorUnit *)
								* nMotorUnitsInDetectionArea_);
			readHeader = 1;
		} else
		{

			if (muInDetectIndex < nMotorUnitsInDetectionArea_)
			{
				/**
				 * create a (blank) motor unit and place it
				 * in both the master and detection area
				 * lists.  The master list will be used
				 * to find this MU when reading the MF.dat
				 * file in order to populate it with the
				 * appropriate values
				 */
				if (sscanf(inputLine, "%d", &tmpMotorUnitId) != 1)
				{
					LogDebug(__FILE__, __LINE__,
						"Failure reading MU id from:\n  %s\n",
						inputLine);
					return 0;
				}
				motorUnitInDetect_[muInDetectIndex] = new MotorUnit();
				motorUnit_[tmpMotorUnitId - 1]
								= motorUnitInDetect_[muInDetectIndex];
				motorUnitInDetect_[muInDetectIndex]->mu_id_
								= tmpMotorUnitId;
				muInDetectIndex++;
			} else
			{
				LogError("Too many lines in '%s'\n", filename);
				return 0;
			}
		}
	}
	if (muInDetectIndex < nMotorUnitsInDetectionArea_)
	{
		LogError("Too few lines in '%s'\n", filename);
		return 0;
	}
	return 1;
}


int MuscleData::loadData(
		const char *muscleDir,
		const char *outputDir,
		int contractionId,
		int usePlowedFibres
	)
{
	char *MFfilename, *MUfilename, *AMUfilename;
	struct stat sb;
	int status;

	/**
	 * Get the fibre and muscle layout data
	 */
	if (usePlowedFibres && outputDir != NULL)
	{
		char idBuffer[128];
		slnprintf(idBuffer, 128, "%d", contractionId);

		/** plowed data is in the output dir */
		MFfilename = strconcat(outputDir,
				OS_PATH_DELIM_STRING,
				"MF-plowed",
				idBuffer, ".dat", NULL);
	} else
	{
		/** unplowed data is in the muscle dir */
		MFfilename = strconcat(muscleDir,
				OS_PATH_DELIM_STRING,
				"MF-unplowed.dat", NULL);
	}
	/** if not found, old data was in the muscle dir too */
	if ( irStat(MFfilename, &sb) < 0 && errno == ENOENT)
	{
		ckfree(MFfilename);
		MFfilename = strconcat(muscleDir,
					OS_PATH_DELIM_STRING,
					"MF.dat", NULL);
	}


	MUfilename = strconcat(muscleDir,
				OS_PATH_DELIM_STRING,
				"MU.dat", NULL);

	status = loadMUdata(MUfilename);
	if (status)
		status = loadMFdata(MFfilename);

	ckfree(MFfilename);
	ckfree(MUfilename);


	/*
	 * Load the list of active motor units
	 */
	AMUfilename = strconcat(muscleDir,
				OS_PATH_DELIM_STRING,
				"Firing-Data",
				OS_PATH_DELIM_STRING,
				"AMU.dat", NULL);
	status = loadAMUdata(AMUfilename) && status;
	ckfree(AMUfilename);

	/*
	 * Load the list of active motor units in the detection area
	 */
	AMUfilename = strconcat(muscleDir,
				OS_PATH_DELIM_STRING,
				"AMU-inDetect.dat", NULL);
	status = loadAMUInDetectData(AMUfilename) && status;
	ckfree(AMUfilename);

	return status;
}

int
MuscleData::writeMFInfo(const char *filename) const
{
	FILE *ofp;
	int i, j;

	ofp = fopenpath(filename, "w");
	if (ofp == NULL)
	{
		LogError("Error in opening file %s", filename);
		return 0;
	}

	LogInfo("Writing muscle fibre data to file:\n");
	LogInfo("    '%s'\n", filename);


	fprintf(ofp, "# v2.0 Muscle Data File\n");
	fprintf(ofp, "# Header:\n");
	fprintf(ofp, "#   <x-detect> <y-detect>\n");
	fprintf(ofp, "#   <original number of healthy fibres>\n");
	fprintf(ofp, "# Motor Unit:\n");
	fprintf(ofp, "#  id: <mu-id>\n");
	fprintf(ofp, "#      %s %s %s %s\n",
						"<fibres/mu>", "<co-ord rad>",
						"<co-ord theta>", "<diameter>");
	fprintf(ofp, "#        Fibre:\n");
	fprintf(ofp, "#          %s %s\n",
						"<X (+/- x-detect)>",
						"<Y (+/- y-detect)>");
	fprintf(ofp, "#          <diameter> <jshift>\n");

	fprintf(ofp, "%d %d\n", xDetect_, yDetect_);
	fprintf(ofp, "%d\n", nMaxFibres_);


	/** print out all motor units that have fibres */
	for (i = 0; i < nMotorUnitsInMuscle_; i++)
	{

		// if this MU has been deleted, skip it
		if (motorUnit_[i] == NULL)
				continue;

		if (motorUnit_[i]->mu_nFibres_ > 0)
		{

			/**
			 ** Write out ID line
			 **/
			fprintf(ofp, "id: %04d\n", motorUnit_[i]->mu_id_);



			/**
			 ** Write out location line
			 **/
			fprintf(ofp, "    %04d %12.8f %12.8f %12.8f\n",
							motorUnit_[i]->mu_nFibres_,
							motorUnit_[i]->mu_loc_r_mm_,
							motorUnit_[i]->mu_loc_theta_,
							motorUnit_[i]->mu_diameter_mm_);




			/**
			 ** Write out individual fibre locations
			 **/
			for (j = 0; j < motorUnit_[i]->mu_nFibres_; j++)
			{

				fprintf(ofp, "        %s",
						fullyTrimmedDouble(
							motorUnit_[i]->mu_fibre_[j]->mf_xCell_
						));
				fprintf(ofp, " %s\n",
						fullyTrimmedDouble(
							motorUnit_[i]->mu_fibre_[j]->mf_yCell_
						));

				fprintf(ofp, "        %12.8f %12.8f\n",
						motorUnit_[i]->mu_fibre_[j]->mf_diameter_,
						motorUnit_[i]->mu_fibre_[j]->mf_jShift_);
			}
		}
	}

	LogInfo("\n\n");

	fclose(ofp);

	return 1;
}

/** Added on Januarry 2005 */
int
MuscleData::writeNeedleRelatedMUInfo(const char *filename) const
{
	FILE *ofp;
	int i,j;
	double CentroidDistanceFromNeedleTip = 0.0;
	double MUXLocation, MUYLocation, MFXLocation;
	double MFYLocation, MFNeedleDistance;//, MUIntersectedArea;
	int NeedleInsideMUTerritory = 0;

	int *LocalFibreCount = NULL;
	int *BelowNeedleLocalFibreCount = NULL;
	float *LocalFibreDensity = NULL;
	float *BelowNeedleLocalFibreDensity = NULL;

	LocalFibreDensity = (float *)
				ckalloc(sizeof(float) * nMotorUnitsInMuscle_);
	BelowNeedleLocalFibreDensity = (float *)
				ckalloc(sizeof(float) * nMotorUnitsInMuscle_);
	LocalFibreCount = (int *)
				ckalloc(sizeof(int) * nMotorUnitsInMuscle_);
	BelowNeedleLocalFibreCount = (int *)
				ckalloc(sizeof(int) * nMotorUnitsInMuscle_);

	/** clear LocalFibreCounts and LocalFibreDensity Buffers */
	memset(LocalFibreDensity, 0,
				sizeof(float) * nMotorUnitsInMuscle_);
	memset(BelowNeedleLocalFibreDensity, 0,
				sizeof(float) * nMotorUnitsInMuscle_);
	memset(LocalFibreCount, 0,
				sizeof(int) * nMotorUnitsInMuscle_);
	memset(BelowNeedleLocalFibreCount, 0,
				sizeof(int) * nMotorUnitsInMuscle_);

	/* radius of needle_uptake_area */
	const double NEEDLE_UPTAKE_IN_MM = 0.500;

	ofp = fopenpath(filename, "w");
	if (ofp == NULL)
	{
		LogError("Error in opening file %s", filename);
		return 0;
	}

	LogInfo("Writing Needle Related Info per MU :\n");
	LogInfo("		LocalFiberCounts,\n");
	LogInfo("   MUCentroidDistanceFromNeedle, NeedleInsideMuTeritorry \n");
	LogInfo("    '%s'\n", filename);

	fprintf(ofp,"# Needle Related MU Info\n");
	fprintf(ofp,"# <id> <local-fibre-count>\n");
	fprintf(ofp,"#          <below-needle-local-fibre-count>\n");
	fprintf(ofp,"#              <local-fibre-density>\n");
	fprintf(ofp,"#                  <below-needle-local-fibre-density>\n");
	fprintf(ofp,"#                      <centroid-distance-from-needle-mm> <needle-inside-mu-territory>\n");

	for (i = 0; i < nMotorUnitsInMuscle_; i++)
	{

		// if this MU has been deleted, skip it
		if (motorUnit_[i] == NULL)
		   continue;
		if (motorUnit_[i]->mu_nFibres_ > 0)
		{

			for (j = 0; j < motorUnit_[i]->mu_nFibres_; j++)
			{

				MFXLocation = (double) (motorUnit_[i]->mu_fibre_[j]->mf_xCell_)/CELLS_PER_MM;
				MFYLocation = (double) (motorUnit_[i]->mu_fibre_[j]->mf_yCell_)/CELLS_PER_MM;

				MFNeedleDistance = sqrt(SQR(needle_->xTip_-MFXLocation)+SQR(needle_->yTip_-MFYLocation));

				if (MFNeedleDistance < NEEDLE_UPTAKE_IN_MM){
					LocalFibreCount[i]++;
					if (MFYLocation < needle_->yTip_)
						BelowNeedleLocalFibreCount[i]++;
				}
			}

			MUXLocation = CARTESIAN_X_FROM_POLAR(motorUnit_[i]->mu_loc_r_mm_,motorUnit_[i]->mu_loc_theta_);
			MUYLocation = CARTESIAN_Y_FROM_POLAR(motorUnit_[i]->mu_loc_r_mm_,motorUnit_[i]->mu_loc_theta_);

			CentroidDistanceFromNeedleTip =  sqrt(SQR(needle_->xTip_-MUXLocation)+SQR(needle_->yTip_-MUYLocation));

			if (CentroidDistanceFromNeedleTip<=motorUnit_[i]->mu_diameter_mm_/2.0)
				NeedleInsideMUTerritory = 1;
			else
				NeedleInsideMUTerritory = 0;

			/**MUIntersectedArea =
				(float) calculateAreaOfIntersectionOfThreeCircles(
					0, 0,
				muscleDiameter_/2.0,
				needle_->xTip_,
				needle_->yTip_,
				NEEDLE_UPTAKE_IN_MM,
				MUXLocation,
				MUYLocation,
				motorUnit_[i]->mu_diameter_mm_/2.0);      */

			if (BelowNeedleLocalFibreCount[i]!=0){
				BelowNeedleLocalFibreDensity[i] = (float) BelowNeedleLocalFibreCount[i] / motorUnit_[i]->mu_nFibres_;
				LocalFibreDensity[i] = (float) LocalFibreCount[i] / motorUnit_[i]->mu_nFibres_;
			}

			/**
			 ** Write out data
			 **/
			fprintf(ofp, "  %04d  %04d  %04d  %2.5f  %2.5f  %15.8f  %02d\n",
							motorUnit_[i]->mu_id_,
							LocalFibreCount[i],
							BelowNeedleLocalFibreCount[i],
							LocalFibreDensity[i],
							BelowNeedleLocalFibreDensity[i],
							CentroidDistanceFromNeedleTip,
							NeedleInsideMUTerritory
							);
		}
	}

	LogInfo("\n\n");
	fclose(ofp);

	if (LocalFibreDensity != NULL)
		ckfree(LocalFibreDensity);
	if (LocalFibreCount != NULL)
		ckfree(LocalFibreCount);
	if (BelowNeedleLocalFibreDensity != NULL)
		ckfree(BelowNeedleLocalFibreDensity);
	if (BelowNeedleLocalFibreCount != NULL)
		ckfree(BelowNeedleLocalFibreCount);

	return 1;
}
/** */

int
MuscleData::writeMUParam(const char *filename) const
{
	FILE *ofp;
	int i,j;
	float FiberDiameterSum;
	float MeanFiberDiameter = 0.0;
	float MinFiberDiameter = 1000.0;
	float MaxFiberDiameter = 0.0;
	float RangeFiberDiameter = 0.0;
	float StandardDevFiberDiameter = 0.0;
	float summation, difference;
	double AverageFiberDensity = 0.0;

	ofp = fopenpath(filename, "w");
	if (ofp == NULL)
	{
		LogError("Error in opening file %s", filename);
		return 0;
	}

	LogInfo("Writing number of fibres per mu to file:\n");
	LogInfo("    '%s'\n", filename);


	fprintf(ofp, "# MU Parameters file\n");
	fprintf(ofp, "# <id> <diameter-mm> <#fibres> <mean-fiber-dia-um>\n");
	fprintf(ofp, "#           <range-fiber-dia> <stdv-fiber-dia> <aver-fiber-dens-(Nfiber/MUArea-mm2)>\n");


	/** print out the info related to all motor units that have fibres */
	for (i = 0; i < nMotorUnitsInMuscle_; i++)
	{
		FiberDiameterSum = 0.0;
		MinFiberDiameter = 1000.0;
		MaxFiberDiameter = 0.0;


		// if this MU has been deleted, skip it
		if (motorUnit_[i] == NULL)
				continue;

		if (motorUnit_[i]->mu_nFibres_ > 0)
		{

			for (j = 0; j < motorUnit_[i]->mu_nFibres_ ; j++)
			{
				FiberDiameterSum = FiberDiameterSum +
									motorUnit_[i]->mu_fibre_[j]->mf_diameter_;

				if (motorUnit_[i]->mu_fibre_[j]->mf_diameter_
										< MinFiberDiameter)
					MinFiberDiameter = motorUnit_[i]->mu_fibre_[j]->mf_diameter_;


				if (motorUnit_[i]->mu_fibre_[j]->mf_diameter_
										> MaxFiberDiameter)
					MaxFiberDiameter = motorUnit_[i]->mu_fibre_[j]->mf_diameter_;
			}

			MeanFiberDiameter = FiberDiameterSum / motorUnit_[i]->mu_nFibres_;

			summation = 0.0;

			for (j = 0; j < motorUnit_[i]->mu_nFibres_ ; j++)
			{
				difference = motorUnit_[i]->mu_fibre_[j]->mf_diameter_ -
										MeanFiberDiameter;
				summation = SQR(difference) + summation;
			}

			StandardDevFiberDiameter = (float) sqrt(summation / (motorUnit_[i]->mu_nFibres_ - 1));

			RangeFiberDiameter = MaxFiberDiameter - MinFiberDiameter;

			AverageFiberDensity = 4 * motorUnit_[i]->mu_nFibres_ /
								(motorUnit_[i]->mu_diameter_mm_ *
											motorUnit_[i]->mu_diameter_mm_ * M_PI);

			/**
			 ** Write out data
			 **/
			fprintf(ofp, "  %04d  %15.8f  %04d  %15.8f  %15.8f  %15.8f %15.8f \n",
							motorUnit_[i]->mu_id_,
							motorUnit_[i]->mu_diameter_mm_,
							motorUnit_[i]->mu_nFibres_,
							MeanFiberDiameter,
							RangeFiberDiameter,
							StandardDevFiberDiameter,
							AverageFiberDensity
							);
		}
	}

	LogInfo("\n\n");

	fclose(ofp);

	return 1;
}

int
MuscleData::writeMUInfo(const char *filename) const
{
	FILE *ofp;
	int i;

	ofp = fopenpath(filename, "wb");
	if (ofp == NULL)
	{
		LogError("Error in opening file %s", filename);
		return 0;
	}


	LogInfo("Writing Motor Unit data to file:\n");
	LogInfo("    '%s'\n", filename);


	fprintf(ofp, "# Motor Unit Data File\n");
	fprintf(ofp, "# %s %s %s %s %s\n",
				"<n-MU's in muscle>",
				"<muscle diam>",
				"<min MU diam>",
				"<max MU diam>",
				"<# MU's in detect>");

	fprintf(ofp, "%d %s",
					nMotorUnitsInMuscle_,
					niceDouble(muscleDiameter_));
	fprintf(ofp, " %s", niceDouble(minMotorUnitDiameter_));
	fprintf(ofp, " %s", niceDouble(maxMotorUnitDiameter_));
	fprintf(ofp, " %d\n", nMotorUnitsInDetectionArea_);

	for (i = 0; i < nMotorUnitsInDetectionArea_; i++)
	{
		if (motorUnitInDetect_[i]->mu_nFibres_ > 0)
		{
			fprintf(ofp, "    %04d\n",
						motorUnitInDetect_[i]->mu_id_);
		} else
		{
			LogDebug(__FILE__, __LINE__,
				"Internal error -- MU %d (%d) has no fibres\n",
				motorUnitInDetect_[i]->mu_id_, i);
			goto FAIL;
		}
	}

	fclose(ofp);
	return 1;


FAIL:
	fclose(ofp);
	return 0;
}

int
MuscleData::writeAMUInDetectInfo(const char *filename) const
{
	FILE *ofp;
	int i;

	ofp = fopenpath(filename, "wb");
	if (ofp == NULL)
	{
		LogError("Error in opening file %s", filename);
		return 0;
	}


	LogInfo("Writing Active, Detected Motor Unit data to file:\n");
	LogInfo("    '%s'\n", filename);


	fprintf(ofp, "# IDs of active & detected units\n");
	fprintf(ofp, "#\n");

	fprintf(ofp, "%d\n", nActiveInDetectMotorUnits_);

	for (i = 0; i < nActiveInDetectMotorUnits_; i++)
	{
		fprintf(ofp, "    %d\n",
				activeInDetectMotorUnit_[i]->mu_id_);
	}

	fclose(ofp);
	return 1;
}


int MuscleData::dump(FILE *fp, int flags) const
{
	int i, j;

	fprintf(fp, "Muscle Data:\n");
	fprintf(fp, "Detection Area (%d x %d (x4))\n",
				xDetect_, yDetect_);
	fprintf(fp, "  Num Motor Units in Muscle         : %d\n",
						nMotorUnitsInMuscle_);
	fprintf(fp, "  Num Motor Units in Detection Area : %d\n",
						nMotorUnitsInDetectionArea_);
	fprintf(fp, "  Muscle Diameter                   : %f\n",
						muscleDiameter_);
	fprintf(fp, "  Max Motor Unit Diameter           : %f\n",
						maxMotorUnitDiameter_);
	fprintf(fp, "  Min Motor Unit Diameter           : %f\n",
						maxMotorUnitDiameter_);

	if ((flags & PRINT_FULL_MU_LIST_DATA) != 0)
	{
		fprintf(fp, "\n");
		fprintf(fp, "  Full list of Motor Units in muscle\n");
		for (i = 0; i < nMotorUnitsInMuscle_; i++)
		{

			if (motorUnit_[i] != NULL)
			{
				fprintf(fp, " -- Motor Unit %3d : ID %d\n",
								i, motorUnit_[i]->mu_id_);
				if ((flags & PRINT_MU_DATA) != 0)
				{
					fprintf(fp, "      R        : %f\n",
								motorUnit_[i]->mu_loc_r_mm_);
					fprintf(fp, "      Theta    : %f\n",
								motorUnit_[i]->mu_loc_theta_);
					fprintf(fp, "      Diameter : %f\n",
								motorUnit_[i]->mu_diameter_mm_);
					fprintf(fp, "      N Fibres : %d\n",
								motorUnit_[i]->mu_nFibres_);

					if ((flags & PRINT_FIBRE_DATA) != 0)
					{
						for (j = 0; j < motorUnit_[i]->mu_nFibres_; j++)
						{
							fprintf(fp, "  -  -- Fibre %d\n", j);
							fprintf(fp, "          MU Id : %d\n",
									motorUnit_[i
									]->mu_fibre_[j]->mf_motorUnit_);
							fprintf(fp, "          MU X Cell : %s\n",
								fullyTrimmedDouble(
									motorUnit_[i
										]->mu_fibre_[j]->mf_xCell_));
							fprintf(fp, "          MU Y Cell : %s\n",
								fullyTrimmedDouble(
									motorUnit_[i
										]->mu_fibre_[j]->mf_yCell_));
							fprintf(fp, "          J-Shift   : %f\n",
									motorUnit_[i
									]->mu_fibre_[j]->mf_jShift_);
							fprintf(fp, "          Diameter  : %f\n",
									motorUnit_[i
									]->mu_fibre_[j]->mf_diameter_);
						}
					}
				}
			}
		}
	}

	fprintf(fp, "\n");
	fprintf(fp, "  Active Motor Units in Detection Area\n");
	for (i = 0; i < nActiveMotorUnits_; i++)
	{

		if (activeMotorUnit_[i] != NULL)
		{
			fprintf(fp, " -- Active Motor Unit %3d : ID %d\n",
							i, activeMotorUnit_[i]->mu_id_);
			if ((flags & PRINT_MU_DATA) != 0)
			{
				fprintf(fp, "      R        : %f\n",
							activeMotorUnit_[i]->mu_loc_r_mm_);
				fprintf(fp, "      Theta    : %f\n",
							activeMotorUnit_[i]->mu_loc_theta_);
				fprintf(fp, "      Diameter : %f\n",
							activeMotorUnit_[i]->mu_diameter_mm_);
				fprintf(fp, "      N Fibres : %d\n",
							activeMotorUnit_[i]->mu_nFibres_);

				if ((flags & PRINT_FIBRE_DATA) != 0)
				{
					for (j = 0; j < activeMotorUnit_[i]->mu_nFibres_; j++)
					{
						fprintf(fp, "  -  -- Fibre %d\n", j);
						fprintf(fp, "          MU Id : %d\n",
							activeMotorUnit_[i
								]->mu_fibre_[j]->mf_motorUnit_);
						fprintf(fp, "          MU X Cell : %s\n",
							fullyTrimmedDouble(
								activeMotorUnit_[i
									]->mu_fibre_[j]->mf_xCell_));
						fprintf(fp, "          MU Y Cell : %s\n",
							fullyTrimmedDouble(
								activeMotorUnit_[i
									]->mu_fibre_[j]->mf_yCell_));
						fprintf(fp, "          J-Shift   : %f\n",
							activeMotorUnit_[i
								]->mu_fibre_[j]->mf_jShift_);
						fprintf(fp, "          Diameter  : %f\n",
							activeMotorUnit_[i
								]->mu_fibre_[j]->mf_diameter_);
					}
				}
			}
		}
	}


	fprintf(fp, "\n");
	fprintf(fp, "Needle Data:\n");

	if (needle_ != NULL)
	{
		fprintf(fp, "  Tip : (%f, %f, %f)\n",
				needle_->getXTipInMM(),
				needle_->getYTipInMM(),
				needle_->getZInMM());

		fprintf(fp, "  End : (%f, %f, %f)\n",
				needle_->getXCannulaTerminusInMM(),
				needle_->getYCannulaTerminusInMM(),
				needle_->getZInMM());

		fprintf(fp, "Slope : (%f)\n", needle_->getSlope());
		fprintf(fp, "Length : (%f)\n", needle_->getCannulaLengthInMM());
	} else
	{
		fprintf(fp, "    NULL -- no needle data\n");
	}

	fprintf(fp, "\n");

	return 1;
}


// This function determines if pMotorUnit is part of activeMotorUnit_
// Return value:  0  if not part of activeMotorUnit_
//                1  if part of activeMotorUnit_
// Added Aug 15, 2002
int MuscleData::isMotorUnitActive(MotorUnit *pMotorUnit)
{
		int i;
		for (i=0; i<nActiveMotorUnits_; i++)
		{
				if (pMotorUnit == activeMotorUnit_[i])
						return 1;
		}
		return 0;
}


int
MuscleData::addFibre(MuscleFibre *newFibre)
{
	int status;

	status = listMkCheckSize(nTotalFibres_ + 1,
				(void **) &masterFibreList_,
				&nFibreBlocks_,
				8,
				sizeof(MuscleFibre *), __FILE__, __LINE__);
	MSG_ASSERT(status, "Allocation failed");
	masterFibreList_[nTotalFibres_++] = newFibre;

	if (nMaxFibres_ < nTotalFibres_)
		nMaxFibres_ = nTotalFibres_;

	return nTotalFibres_ - 1;
}


int
MuscleData::removeFibre(MuscleFibre *fibre)
{
	int i;

	for (i = 0; i < nTotalFibres_; i++)
	{
		if (masterFibreList_[i] == fibre)
		{
			masterFibreList_[i] = NULL;
			return 1;
		}
	}
	return nTotalFibres_ - 1;
}


MotorUnit::MotorUnit()
{
	mu_id_ = 0;
	mu_loc_r_mm_ = 0;
	mu_loc_theta_ = 0;
	mu_diameter_mm_ = 0;
	mu_nFibres_ = 0;
	mu_nHealthyFibres_ = (-1);
	mu_nFibreAllocationBlocks_ = 0;
	mu_fibre_ = NULL;
	mu_firingTime_ = NULL;
	mu_nFirings_ = 0;
	mu_nFiringBlocks_ = 0;
	mu_expectedNumFibres_ = 0;
}

MotorUnit::~MotorUnit()
{
	int i;

	validate();

	if (mu_fibre_ != NULL)
	{
		for (i = 0; i < mu_nFibres_; i++)
		{
			if (mu_fibre_[i] != NULL)
			{
				delete mu_fibre_[i];
				mu_fibre_[i] = NULL;
//				validate();
			}
		}
		ckfree(mu_fibre_);
		mu_fibre_ = NULL;
		mu_nFibres_ = 0;
	}

	if (mu_firingTime_ != NULL)
		ckfree(mu_firingTime_);
}

int
MotorUnit::validate() const
{
	int status = 1;
	int i;

	for (i = 0; i < mu_nFibres_; i++)
	{
		if (mu_fibre_[i] == NULL)
			continue;

		if (mu_fibre_[i]->mf_motorUnit_ != mu_id_)
		{
			LogError(
				"Motor Unit %d contains fibre %d (of %d) 0x%08x marked for MU %d\n",
				mu_id_, i, mu_nFibres_,
				mu_fibre_[i],
				mu_fibre_[i]->mf_motorUnit_);
			MSG_FAIL("BAD MU/FIBRE PAIRING");
		}

		status = mu_fibre_[i]->validate() && status;
	}

	return status;
}

int
MotorUnit::recalculateCentroid()
{
	int i;
	double xInMM, yInMM;

	xInMM = yInMM = 0;

	/** sum all the X's, Y's in mm */
	for (i = 0; i < mu_nFibres_; i++)
	{
		xInMM += mu_fibre_[i]->mf_xCell_ / CELLS_PER_MM;
		yInMM += mu_fibre_[i]->mf_yCell_ / CELLS_PER_MM;
	}

	/** calculate the average of all these values */
	xInMM = xInMM / (double) mu_nFibres_;
	yInMM = yInMM / (double) mu_nFibres_;

	mu_loc_theta_ = (float) POLAR_THETA_FROM_CARTESIAN(xInMM, yInMM);
	mu_loc_r_mm_ = (float) POLAR_R_FROM_CARTESIAN(xInMM, yInMM);

	return 1;
}


int
MotorUnit::addFibre(MuscleFibre *newFibre)
{
	int status;

	status = listMkCheckSize(mu_nFibres_ + 1,
				(void **) &mu_fibre_,
				&mu_nFibreAllocationBlocks_,
				8,
				sizeof(MuscleFibre *), __FILE__, __LINE__);
	MSG_ASSERT(status, "Allocation failed");
	mu_fibre_[mu_nFibres_] = newFibre;
	mu_fibre_[mu_nFibres_]->mf_motorUnit_ = mu_id_;
	mu_nFibres_++;

	return 1;
}

int
MotorUnit::removeFibre(MuscleFibre *oldFibre)
{
	int i, j;

	for (i = 0; i < mu_nFibres_; i++)
	{
		if (mu_fibre_[i] == oldFibre)
		{
			for (j = i + 1; j < mu_nFibres_; j++)
			{
				mu_fibre_[j - 1] = mu_fibre_[j];
			}
			mu_nFibres_--;
			return 1;
		}
	}
	return 0;
}

int
MotorUnit::removeFibre(int index)
{
	int j;

	for (j = index + 1; j < mu_nFibres_; j++)
	{
		mu_fibre_[j - 1] = mu_fibre_[j];
	}
	mu_nFibres_--;
	return 1;
}

float
MotorUnit::getXLocationInMM() const
{
	return (float) CARTESIAN_X_FROM_POLAR(
					getLocationRadius(),
				getLocationTheta()
			);
}

float
MotorUnit::getYLocationInMM() const
{
	return (float) CARTESIAN_Y_FROM_POLAR(
					getLocationRadius(),
				getLocationTheta()
			);
}

MuscleFibre::MuscleFibre()
{
	mf_motorUnit_ = 0;
	mf_xCell_ = 0;
	mf_yCell_ = 0;
	mf_jShift_ = 0;
	mf_diameter_ = 0;
	mf_healthyDiameter_ = 0;
}

MuscleFibre::MuscleFibre(float xCell, float yCell)
{
	memset(this, 0, sizeof(*this));
	mf_xCell_ = xCell;
	mf_yCell_ = yCell;
}

MuscleFibre::~MuscleFibre()
{
	mf_motorUnit_ = (-1);
	mf_xCell_ = (-1);
	mf_yCell_ = (-1);
	mf_jShift_ = (-1);
	mf_diameter_ = (-1);
	mf_healthyDiameter_ = (-1);
}

int
MuscleFibre::validate() const
{
	return 1;
}

