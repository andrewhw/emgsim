/**
 ** Simulator Interface File
 ** Contains the definition of the simulator object, and the
 ** results it deals in
 **
 ** $Id: Simulator.cpp 24 2019-06-12 12:04:31Z andrew $
 **/


#include "os_defs.h"

#include <string.h>
#include <time.h>
# include <errno.h>
#ifndef OS_WINDOWS_NT
# include <unistd.h>
#else
# include <io.h>
#endif

#define PRIVATE public
#include <Simulator.h>

#include "tclCkalloc.h"

#include "random.h"
#include "stringtools.h"
#include "pathtools.h"
#include "msgir.h"
#include "log.h"
#include "massert.h"

#include "SimulatorControl.h"
#include "NeedleInfo.h"
#include "MUP_utils.h"
#include "globalHandler.h"
#include "make16bit.h"
#include "statistics.h"
#include "userinput.h"
#include "MUP.h"
#include "DQEmgData.h"
#include "dco.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif


const int Simulator::FLAG_USE_LAST_MUSCLE		= 0x01;
const int Simulator::FLAG_USE_OLD_FIRING_TIMES		= 0x02;
const int Simulator::FLAG_RUN_SURFACE				= 0x04;

static const char *sVersionString = "2.1";

extern struct globals *g;

const char *Simulator::sGetVersion()
{
	return sVersionString;
}

Simulator::Simulator()
{
	memset(this, 0, sizeof(*this));
}


Simulator::~Simulator()
{
	if (configFile_ != NULL)
		ckfree(configFile_);
}

NeedleInfo *SimulationResult::getNeedleInfo() const
{
	if (muscleData_ == NULL)
		return NULL;
	return muscleData_->getNeedleInfo();
}

const char *Simulator::getConfigFileName()
{
	if (configFile_ == NULL)
	{
		setConfigFileName(NULL);
	}
	return configFile_;
}

/**
 ** Record what config file we are using
 **/
int Simulator::setConfigFileName(const char *configFile)
{
	if (configFile == NULL)
	{
		{
			const char *tmpConfigName;

#			ifdef OS_WINDOWS_NT
			tmpConfigName = "c:\\simulator\\simulator.cfg";
#			else
			tmpConfigName = "./data/simulator.cfg";
#			endif

			configFile_ = osIndependentPath(tmpConfigName);
		}

	} else
	{
		configFile_ = osIndependentPath(configFile);
	}

	return 1;
}

int Simulator::readConfigInfo()
{
	FILE		*fp;

	LogInfo("Using globals definition file '%s'\n", configFile_);

	fp = fopenpath(configFile_, "rb");
	if (fp == NULL)
	{
		LogError("Failed to open \"%s\" for reading.",
					configFile_);
		goto FAIL;
	}


	/* retrieve globals structure */
	g->list_ = parseAttValFile(fp);
	fclose(fp);

	clearGlobalPointers();
	return 1;

FAIL:
	fclose(fp);
	return 0;
}

struct globals *
Simulator::initializeGlobals(
		const char *configFile,
		const char *outputRoot
	)
{
	int status;

	MSG_ASSERT(configFile != NULL, "Simulator::initializeGlobals - config file not specified");
	MSG_ASSERT(outputRoot != NULL, "Simulator::initializeGlobals - output root not specified");

	/** allocate the globals structure */
	g = (struct globals *) ckalloc(sizeof(struct globals));
	g->muscle_ = new MuscleParameters();

	/* assign hard-coded defaults */
	if ( ! setGlobalDefaultValues(g) )
	{
		LogError("Global value initialization failed\n");
		return 0;
	}

	/**
	 * overwrite base path with config file directory, if not already
	 * pointing there.  Note that this is _meant_ to be a pointer
	 * comparison, as it is testing whether output stem is the buffer
	 * that outputRoot is actually pointing at, to avoid a bug in
	 * strncpy on some platforms.
	 */
	if (outputRoot != g->output_stem)
	{
		strncpy(g->output_stem, outputRoot, FILENAME_MAX);
	}

	setConfigFileName(configFile);

	/** load in the values from the config file, if present */
#   ifndef OS_WINDOWS_NT
	status = access(configFile_, R_OK);
#   else
	status = access(configFile_, 00);
#   endif
	if (status >= 0)
	{
		if ( ! readConfigInfo() )
		{
			return NULL;
		}
	}

	g->read_fib_dat = 0;
	g->write_fib_dat = 1;

	fixPathnames(g);
	loadValues(g);

	return g;
}

static int
getNextRunID()
{
	int fileId;

	fileId = dirToolsGetNextId(g->output_dir, "contraction*.dat");
	if (fileId <= 0)
		fileId = 1;

	return fileId;
}

SimulationResult *Simulator::runSurface(int flags)
{
	return 0;
}
//{
//	SimulationResult *result;
//	FILE *fp;
//	int status;
//
//
///******* Temporary test objects constructed here ************/
//
//SMUPGeometryInfo *sg = new SMUPGeometryInfo();
//SMUPFiringInfo *sf = new SMUPFiringInfo();
//
///************************************************************/
//
//	if ( ! saveGlobalDirectoryInfo(g, NULL) )
//	{
//		LogError("Directory management failed\n");
//		return NULL;
//	}
//
//	result = new SimulationResult();
//
//	/**
//	 ** Save the config info . . .
//	 **/
//	fp = fopenpath(configFile_, "wb");
//	if (fp == NULL) {
//		LogError("Cannot open config file '%s' for writing\n",
//						configFile_);
//		result->setState(-1);
//		goto CLEANUP;
//	}
//
//	writeAttValList(fp, g->list_, "version 2.2 simulator config file");
//	fclose(fp);
//
//	seedLocalRandom(time(NULL));
//
//	/** set up jitter factor */
//	MUP::sSetExpansionFactor(g->jitterInterpolationExpansion);
//	result->MUPPath_ = ckstrdup(g->MUPs_dir);
//
//
//	emgFileId = getNextRunID();
//	result->setFileId( emgFileId );
//
//	/** create new data */
//	result->muscleData_ = muscle(
//				g->muscle_dir,
//				g->output_dir,
//				g->muscle_,
//				&g->pathology,
//				g->cannula_length,
//				g->canPhysicalRadius,
//				g->needle_x_position,
//				g->needle_y_position,
//				g->needle_z_position,
//				g->mu_layout_type,
//				g->super_jitter_seeds
//			);
//	if ( result->muscleData_ == NULL) {
//		result->setState(-1);
//		LogError("Failure in muscle routine\n");
//		goto CLEANUP;
//	}
//
//
//
//	status = firing(
//				g->firings_dir,
//				result->muscleData_,
//				g->firing_.contractionLevelAsPercentMVC,
//				g->firing_.coefficientOfVarianceInFiringTimes,
//				g->firing_.recruitmentSlope,
//				g->firing_.minimumFiringRate,
//				g->firing_.maximumFiringRate,
//				g->firing_.maximumFiringThreshold,
//				g->emg_elapsed_time,
//				0
//			);
//	if ( ! status ) {
//		result->setState(-1);
//		LogError("Failure in firing routine\n");
//		goto CLEANUP;
//	}
//
//	/**
//	 * Add calls to functions doing surface EMG here
//	 */
///**************************************************************************/
//
//if (!(getSMUPFiringInfo(sf, 60 ,result)))
//		LogError(" *error in getting MU firingInfo\n");
//
//printSMUPFiringInfo(sf);
//
//if (!(getSMUPGeometryInfo(sg, 60 ,result)))
//		LogError(" *error in getting MU GeometryInfo");
//
//printSMUPGeometryInfo(sg);
//
///**************************************************************************/
//
//	/**
//	 * At the end, clean up and return
//	 */
//CLEANUP:
//	return result;
//}

SimulationResult *Simulator::run(int flags)
{
	char patientIdString[256];
	char contractionDescription[1024 * 12];
	SimulationResult *result;
	FILE *fp;
	int emgFileId, status;
	int isNewMuscle;
	int needMfapsRebuilt;
	DQEmgData *outputContractionFile;


	needMfapsRebuilt = 0;
	if ((flags & Simulator::FLAG_USE_LAST_MUSCLE) != 0)
	{
		if ( ! reuseGlobalDirectoryInfo(g))
		{
			LogError("Directory management failed\n");
			return NULL;
		}
	} else
	{
		needMfapsRebuilt = 1;
		if ( ! saveGlobalDirectoryInfo(g, NULL) )
		{
			LogError("Directory management failed\n");
			return NULL;
		}
	}



	result = new SimulationResult();

	slnprintf(patientIdString, 256, "%d",
			g->fileDescription.patient_id);
	slnprintf(contractionDescription, 1024 * 12,
			"Simulated Contraction at %s %% MVC",
			niceDouble(g->firing_.contractionLevelAsPercentMVC));
	slnprintf(contractionDescription, 1024 * 12,
			"%s; Needle Tip Location (%s",
			contractionDescription,
			niceDouble(g->needle_x_position));
	slnprintf(contractionDescription, 1024 * 12,
			"%s, %s",
			contractionDescription,
			niceDouble(g->needle_y_position));
	slnprintf(contractionDescription, 1024 * 12,
			"%s, %s)",
			contractionDescription,
			niceDouble(g->needle_z_position));

	result->emgData_ =
		outputContractionFile = new DQEmgData(
							"UW:Sim:2",
						time(NULL),
						g->fileDescription.operator_name,
						g->fileDescription.patient_name,
						patientIdString,
						1924, 2, 29,
						80, /* 2004-1924*/
						DQEmgData::FEMALE,
						g->fileDescription.muscle_description,
						(DQEmgData::Side) g->fileDescription.muscle_side,
						contractionDescription
					);

	outputContractionFile->setNewOperator(
					g->fileDescription.new_operator
			);
	outputContractionFile->setNewPatient(
					g->fileDescription.new_patient
			);
	outputContractionFile->setNewMuscle(
					g->fileDescription.new_muscle
			);


	/**
	 ** Save the config info . . .
	 **/
	fp = fopenpath(configFile_, "wb");
	if (fp == NULL)
	{
		LogError("Cannot open config file '%s' for writing\n",
						configFile_);
		result->setState(-1);
		goto CLEANUP;
	}

	writeAttValList(fp, g->list_, "version 2.2 simulator config file");
	fclose(fp);

	seedLocalRandom((int) time(NULL));

	/** set up jitter factor */
	MUP::sSetExpansionFactor(g->jitterInterpolationExpansion);
	result->MUPPath_ = ckstrdup(g->MUPs_dir);


	emgFileId = getNextRunID();
	{
		char tmpFileName[4096];
		slnprintf(tmpFileName, 4096, "%s/contraction%d.dat",
						g->output_dir, emgFileId);
		LogInfo("Opening ID file '%s'\n", tmpFileName);
		fp = fopenpath(tmpFileName, "wb");
		if (fp == NULL)
		{
			LogError("Cannot open id file '%s/contraction%d.dat'\n",
						g->output_dir, emgFileId);
			result->setState(-1);
			goto CLEANUP;
		}
		fclose(fp);
	}
	result->setFileId( emgFileId );

	if ((flags & Simulator::FLAG_USE_LAST_MUSCLE) != 0)
	{
		/** load data from last time */
		result->muscleData_ = loadMuscleData(
				emgFileId - 1,
				g->muscle_dir,
				g->output_dir,
				0 /** load unplowed fibres - we plow later */
			);
		if ( result->muscleData_ == NULL)
		{
			result->setState(-1);
			LogError("Failure in muscle load routine\n");
			goto CLEANUP;
		}

		if (result->muscleData_->getNeedleInfo() == NULL)
		{
			setAndClipNeedleLocation(
						result->muscleData_,
						result->muscleData_->muscleDiameter_ / 2.0f,
						g->needle_x_position,
						g->needle_y_position,
						g->needle_z_position,
						g->cannula_length,
						(float) g->canPhysicalRadius
					);
		}

	} else
	{

		/** if we are making a new muscle, we cannot use old times */
		flags &= (~Simulator::FLAG_USE_OLD_FIRING_TIMES);

		/** create new data */
		result->muscleData_ = muscle(
				emgFileId,
				g->muscle_dir,
				g->output_dir,
				g->muscle_,
				&g->pathology,
				g->cannula_length,
				(float) g->canPhysicalRadius,
				g->needle_x_position,
				g->needle_y_position,
				g->needle_z_position,
				g->mu_layout_type,
				g->super_jitter_seeds,
				g->muscleLayoutFunctionType
			);
		if ( result->muscleData_ == NULL)
		{
			result->setState(-1);
			LogError("Failure in muscle routine\n");
			goto CLEANUP;
		}
	}

	if (result->muscleData_->needle_->isDifferent(
				g->needle_x_position,
				g->needle_y_position,
				g->needle_z_position,
				g->cannula_length,
				(float) g->canPhysicalRadius,
				DEFAULT_CANNULA_SLOPE
			))
	{
		result->muscleData_->needle_->set(
				g->needle_x_position,
				g->needle_y_position,
				g->needle_z_position,
				g->cannula_length,
				(float) g->canPhysicalRadius,
				DEFAULT_CANNULA_SLOPE
			);
		needMfapsRebuilt = 1;
	}

	result->muscleData_->validate();
	if ((flags & Simulator::FLAG_USE_OLD_FIRING_TIMES) != 0)
	{
		status = loadFiring(
				result->muscleData_,
				g->firings_dir
			);
		if ( ! status )
		{
			result->setState(-1);
			LogError("Failure in firing load routine\n");
			goto CLEANUP;
		}
	} else
	{
		status = firing(
				g->firings_dir,
				result->muscleData_,
				g->firing_.contractionLevelAsPercentMVC,
				g->firing_.coefficientOfVarianceInFiringTimes,
				g->firing_.recruitmentSlope,
				g->firing_.minimumFiringRate,
				g->firing_.maximumFiringRate,
				g->firing_.maximumFiringThreshold,
				g->emg_elapsed_time,
				0
			);

		if ( ! status )
		{
			result->setState(-1);
			LogError("Failure in firing routine\n");
			goto CLEANUP;
		}

	}


	result->muscleData_->validate();
	if (g->seekNeedle)
	{
		if ( ! seekNeedleToNearbyFibres(
					result->muscleData_,
					g->minimumMuscleMetricThreshold
				) )
		{
			result->setState(-1);
			LogError("Failure seeking needle\n");
			goto CLEANUP;
		}
	}


	result->muscleData_->validate();
	if ( ! storeNeedleInfo(
				result->muscleData_,
				emgFileId,
				g->output_dir
		))
	{
		result->setState(-1);
		LogError("Failure storing needle info\n");
		goto CLEANUP;
	}

	result->muscleData_->validate();
	if ( ! plowMuscleFibres(
				emgFileId,
				g->output_dir,
				result->muscleData_,
				(float) g->canPhysicalRadius
			) )
	{
		result->setState(-1);
		LogError("Failure in plowing function\n");
		goto CLEANUP;
	}

	/**
	 * Run the surface generation routines if
	 * called for
	 */
//	if ((flags & Simulator::FLAG_RUN_SURFACE) != 0)
//	{
//		SMUPGeometryInfo *sg = new SMUPGeometryInfo();
//		SMUPFiringInfo *sf = new SMUPFiringInfo();
//
//		if (!(getSMUPFiringInfo(sf, 60 ,result)))
//			LogError(" *error in getting MU firingInfo\n");
//
//		printSMUPFiringInfo(sf);
//
//		if (!(getSMUPGeometryInfo(sg, 60 ,result)))
//			LogError(" *error in getting MU GeometryInfo");
//
//		printSMUPGeometryInfo(sg);
//	}

	result->muscleData_->validate();


	/* if the MFP directory doesn't exist, we have to build it */
	{
		struct stat sb;

		if ((irStat(g->MUPs_dir, &sb) < 0) && (errno == ENOENT))
		{
			needMfapsRebuilt = 1;
		}
	}


	/**
	 * Rebuild MUP's if we have to.  If not, they are already
	 * on disk, and will be found by makeEmg (thus we don't
	 * need to load them here.
	 */
	if ( needMfapsRebuilt )
	{

		status = makeMUP(
				result->muscleData_,
				&result->MUPIdList_,
				&result->nMUPs_
			);
		if ( ! status )
		{
			result->setState(-1);
			LogError("Failure in make-MUP routine\n");
			goto CLEANUP;
		}
	}



	result->muscleData_->validate();
	status = makeEmg(result->muscleData_, emgFileId);
	if (! status)
	{
		result->setState(-1);
		LogError("Failure in make-emg routine\n");
		goto CLEANUP;
	}



	if ((flags & Simulator::FLAG_USE_LAST_MUSCLE) == 0)
		isNewMuscle = 0;
	else
		isNewMuscle = 1;

	result->muscleData_->validate();
	status = make16bit(
				g,
				outputContractionFile,
				emgFileId,
				isNewMuscle
			);

	if (status != 1)
	{
		result->setState(-1);
		LogError("Failure in make16bit routine\n");
		goto CLEANUP;
	}


	result->muscleData_->validate();
//	status = calculateStatistics(g, result, outputContractionFile);
//	if (status != 1)
//	{
//		result->setState(-1);
//		LogError("Failure calculating statistics\n");
//		goto CLEANUP;
//	}

	result->muscleData_->validate();
	saveOutputDirConfigFile(g, emgFileId);

	{
		char tmpBuffer[BUFSIZ];
		char *filename;
		/**
		 * Store the contraction data
		 */
		slnprintf(tmpBuffer, BUFSIZ, "%s/%s/contraction%d.dat",
					g->muscle_dir,
					g->output_dir_sub,
					emgFileId);
		filename = osIndependentPath(tmpBuffer);
		outputContractionFile->store(filename);
		ckfree(filename);

		/**
		 * load the GST file for the EMG data
		 */
		slnprintf(tmpBuffer, BUFSIZ, "%s/%s/micro%d.gst",
					g->muscle_dir,
					g->output_dir_sub,
					emgFileId);
		filename = osIndependentPath(tmpBuffer);
		result->dcoData_ = readDcoFile(filename);
		ckfree(filename);
	}

	result->muscleData_->validate();
	if (result->emgData_ != NULL)
	{
		LogNotice("\n");
		LogNotice("Output in : '%s'\n", result->getOutputDirectory());
		LogNotice("\n");
	}

CLEANUP:
	return result;
}

SimulationResult *Simulator::open(
		const char *path,
		int contractionId,
		int usePlowedFibres
	)
{
	char tmpBuffer[BUFSIZ];
	char *filename;
	SimulationResult *result;
	int status;
	/*FILE *zohre;*/

	LogInfo("Opening simulator with path '%s'\n", path);
	if ( ! setupGlobalDirectoryInfoForOpen(g, path) )
	{
		LogError("Directory management failed\n");
		return NULL;
	}

	result = new SimulationResult();

	result->muscleData_ = loadMuscleData(
				contractionId,
				g->muscle_dir,
				g->output_dir,
				usePlowedFibres
			);
	if ( result->muscleData_ == NULL)
	{
		result->setState(-1);
		LogError("Failure in muscle load routine\n");
		goto CLEANUP;
	}

	status = loadFiring(
				result->muscleData_,
				g->firings_dir
			);
	if ( ! status )
	{
		result->setState(-1);
		LogError("Failure in firing load routine\n");
		goto CLEANUP;
	}

	result->MUPPath_ = ckstrdup(g->MUPs_dir);
	status = loadMUPs(&result->MUPIdList_,
					result->muscleData_->getNumMotorUnits());
	if ( ! status )
	{
		result->setState(-1);
		LogError("Failure in make-emg routine\n");
		goto CLEANUP;
	}
	result->nMUPs_ = result->muscleData_->getNumMotorUnits();

	/** load the EMG data */
	slnprintf(tmpBuffer, BUFSIZ, "%s\\%s\\contraction%d.dat",
				g->muscle_dir,
				g->output_dir_sub,
				contractionId);
	/* Matlab needed */
	/*zohre = fopen ("free11.dat","wb");
	fwrite(tmpBuffer,sizeof(double),BUFSIZ,zohre);
	fclose(zohre);*/

	filename = osIndependentPath(tmpBuffer);
	result->emgData_ = new DQEmgData();
	result->emgData_->load(filename);
	ckfree(filename);


	/** load the GST file for the EMG data */
	slnprintf(tmpBuffer, BUFSIZ, "%s\\%s\\micro%d.gst",
				g->muscle_dir,
				g->output_dir_sub,
				contractionId);
	filename = osIndependentPath(tmpBuffer);
	result->dcoData_ = readDcoFile(filename);
	ckfree(filename);

	return result;

CLEANUP:
	delete result;
	return NULL;
}

