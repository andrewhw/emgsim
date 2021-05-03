/**
 ** Mainline routine for updated simulator, begun Summer 2001.
 **
 ** $Id: main.cpp 11 2011-03-23 11:29:15Z andrew $
 **/
#include <os_defs.h>

#ifndef MAKEDEPEND
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>

#  ifdef                OS_WINDOWS_NT
#   include <io.h>
#	include <process.h>
# else
#	include <unistd.h>
#  endif

# include <ctype.h>
# include <sys/stat.h>
#endif

#include "Simulator.h"
#include "SimulatorControl.h"
#include "dco.h"

#include "log.h"
#include "globalHandler.h"
#include "pathtools.h"
#include "filetools.h"
#include "stringtools.h"
#include "random.h"
#include "userinput.h"
#include "error.h"
#include "tclCkalloc.h"
#include "MUP.h"

#include "os_types.h"
#include "os_names.h"

#include "options.h"

#include "attvalfile.h"



#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif
	int debug;
# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

/** forward declarations */
//static int doFullSimulator(struct optionflags *flags);
//static int doLastStudy(struct optionflags *flags);

static void outputFunction(
	void *userData,
	int level,
	char *buffer,
	int size
	)
{
	write(1, buffer, size);
}

static void
initializePathBuffers(
		char *configBasePath,
		char *configFile,
		char *outputRoot,
		int bufferLength,
		struct optionflags *flags
	)
{
	char *conversion;

#ifdef  OS_WINDOWS_NT
	/** figure out where the data should come from */
	if (flags->configFilePath == NULL)
	{
		if (flags->destinationRoot == NULL)
		{
			slnprintf(configBasePath, bufferLength,
					"%c:\\simulator\\data\\", flags->driveLetter[0]);
		} else
		{
			slnprintf(configBasePath, bufferLength,
				"%s\\", flags->destinationRoot);
		}
	} else
	{
		slnprintf(configBasePath, bufferLength,
			"%s\\", flags->configFilePath);
	}

#else
	if (flags->configFilePath == NULL)
	{
		if (flags->destinationRoot == NULL)
		{
			slnprintf(configBasePath, bufferLength, "./data/");
		} else
		{
			slnprintf(configBasePath, bufferLength,
				"%s/", flags->destinationRoot);
		}
	} else
	{
		slnprintf(configBasePath, bufferLength,
			"%s/", flags->configFilePath);
	}
#endif


	/** used the path to the config to find it */
	slnprintf(configFile, bufferLength,
			"%ssimulator.cfg",
			configBasePath);


	/** figure out where the data should go */
	if (flags->destinationRoot != NULL)
	{
		strncpy(outputRoot, flags->destinationRoot, bufferLength);
	} else
	{
		strncpy(outputRoot, configBasePath, bufferLength);
	}


	conversion = osIndependentPath(configBasePath);
	strncpy(configBasePath, conversion, bufferLength);
	ckfree(conversion);

	conversion = osIndependentPath(configFile);
	strncpy(configFile, conversion, bufferLength);
	ckfree(conversion);

	conversion = osIndependentPath(outputRoot);
	strncpy(outputRoot, conversion, bufferLength);
	ckfree(conversion);

}


static int
runOneSimulation(
		struct optionflags *flags,
		int *isQuitting,
		char *configBasePath,
		char *configFile,
		char *outputRoot,
		char *logFile
	)
{
	SimulationResult *result = NULL;
	class Simulator *sim = NULL;
	struct globals *initializedGlobals;
	int status = 0;
	int simFlags;
	time_t curtime;


	*isQuitting = 0;

	sim = new Simulator();


	/** create the globals structure with all the user-supplied info */
	initializedGlobals = sim->initializeGlobals(configFile, outputRoot);

	if ( initializedGlobals == NULL) {
		fprintf(stderr,
				"Failure in internal initialization -- aborting\n");
		goto END;
	}



	LogInfo("\n");
	LogInfo("Muscle Simulator Running on :%s\n", osGetVersionId());
	LogInfo("\n");

	if (flags->textOutput == 1) {
		g->text_output = 1;
	} else {
		g->text_output = 0;
	}

	if (flags->useLastMuscle == 1) {
		g->use_last_muscle = 1;
	} else {
		g->use_last_muscle = 0;
	}

	if (flags->useOldFiringTimes == 1) {
		g->use_old_firing_times = 1;
	} else {
		g->use_old_firing_times = 0;
	}


	/** >>>> HANDLE UI **/
	/*
	 * until we fix the globals, this has to be called
	 * to ensure that the values get linked up correctly
	 *
	 *  ** MENU **
	 */
	if ( ! flags->skipValidateGlobals) {
		if ( ! validateGlobals(g, isQuitting) ) {
			Error("User initialization failed!\n");
			goto END;
		}

		if (*isQuitting) {
			LogWarn("User abort . . .\n");
			goto END;
		}
	}

	dumpGlobalSettings(g);

	if (g->use_last_muscle) {
		flags->useLastMuscle = 1;
	} else {
		flags->useLastMuscle = 0;
	}
	
	if (g->use_old_firing_times) {
		flags->useOldFiringTimes = 1;
	} else {
		flags->useOldFiringTimes = 0;
	}
	/** <<<< HANDLE UI **/

	simFlags = 0;
	if (flags->useLastMuscle)
		simFlags |= Simulator::FLAG_USE_LAST_MUSCLE;

	if (flags->useOldFiringTimes)
		simFlags |= Simulator::FLAG_USE_OLD_FIRING_TIMES;

	if (flags->runSurface)
		simFlags |= Simulator::FLAG_RUN_SURFACE;

	/** GENERATE MUSCLE DATA **/
	result = sim->run(simFlags);

	/** let's see what we got! */
	//result->dump(stdout);

	if (result != NULL) {
		if (result->getErrorState() == 0)
			status = 1;

		if (result->getMuscleData() != NULL)
			result->getMuscleData()->validate();
	}
	curtime = time(NULL);
	LogInfo("Simulator Run Complete: %s\n", ctime(&curtime));


	LogFlush();
	if ((result != NULL) && (! (*isQuitting))) {
		char logFileName[4096];

		slnprintf(logFileName, 4096, "%s%csimulation%d.log",
			result->getOutputDirectory(),
			OS_PATH_DELIM,
			result->getFileId());

		/** if this fails, there is not much we can do */
		(void) copyFile(logFileName, logFile);

	}

END:
	if (result != NULL) {
		if (result->getMuscleData() != NULL)
			result->getMuscleData()->validate();
		delete result;
	}

	if (sim != NULL)
		delete sim;

	deleteGlobals();

	return status;
}

/**
 ** ----------------------------------------------------------------
 ** mainline routine -- call for option parsing, setup and
 ** processing
 **
 **/
int
main(int argc, char **argv)
{
	struct optionflags flags;
	char configBasePath[FILENAME_MAX];
	char configFile[FILENAME_MAX];
	char logFile[FILENAME_MAX];
	char outputRoot[FILENAME_MAX];
	int exitStatus = 0;
	int isQuitting = 0;
	int runCount = 0;
	time_t curtime;
	int i;

	// ensure that all the flags are cleared
	memset(&flags, 0, sizeof(flags));

	// initialize our randomizer
	seedLocalRandom(((int) time(NULL)) * getpid());


	curtime = time(NULL);

	if ( parseOptions(&flags, argc, argv) ) {

		initializePathBuffers(
				configBasePath, configFile, outputRoot,
				FILENAME_MAX, &flags);
#ifdef	OS_WINDOWS
		slnprintf(logFile, FILENAME_MAX, "%s\\simulator.log",
				outputRoot);
#else
		slnprintf(logFile, FILENAME_MAX, "%s/simulator.log",
				outputRoot);
#endif

		LogOpen(argv[0],
		        LOGDEST_FUNCTION|LOGDEST_NO_ID|LOGDEST_LOCALFILE,
		        logFile);
		LogFunction(outputFunction, NULL);
		LogInfo("Simulator Run Begins: %s\n", ctime(&curtime));
		LogInfo("\nThis version compiled on %s at %s\n",
				__DATE__, __TIME__);
		LogInfo("Arguments:\n");
		for (i = 0; i < argc; i++)
		{
				LogInfo("    %s\n", argv[i]);
		}

		do {
			runCount++;

			if ( ! runOneSimulation(&flags, &isQuitting,
					configBasePath, configFile, outputRoot, logFile) ) {
				fprintf(stderr, "Simulation run %d failed\n", runCount);
				exitStatus = 1;
			}
		} while (flags.runLoop && (!isQuitting));

		if (flags.runLoop)
		{
			LogInfo("Simulator run %d times\n", runCount);
		}
	}

	if (flags.waitForKeyToExit) {
		getUserInput("Press [ENTER] to exit : ");
	}

	cleanOptions(&flags);

	LogClose();

	DUMP_MEMORY;

#ifndef OS_WINDOWS_NT
	copyFileIfPresent(1, "ckalloc.log");
#endif

	return (exitStatus);
}


