/**
 ** Convert the output from the emg.dat file into the 16 bit
 ** format required by the DQEMG program.
 **
 ** $Id: globalHandler.cpp 25 2019-06-12 12:16:58Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKEDEPEND
# include       <stdio.h>
# include       <string.h>
# include       <errno.h>
# include       <sys/types.h>
# include       <sys/stat.h>
# ifndef        OS_WINDOWS_NT
#   include     <unistd.h>
# else
#   include     <io.h>
# endif
# include       <fcntl.h>
#endif

#include "stringtools.h"
#include "pathtools.h"
#include "tclCkalloc.h"
#include "reporttimer.h"
#include "SimulatorControl.h"
#include "io_utils.h"
#include "log.h"

#include "make16bit.h"
#include "dco.h"
#include "emg.h"


#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

static int makeMacroFile(char *outputFile, char *inputFile);

/**
 ** wrapper to read from globals
 **
 **
 ** outpt of make-emg is in uV,
 **
 ** convert to have 22byte header, and convert to shorts
 **
 ** scale factor -> .1 or .01
 **
 ** (take values, multiply by 10 and write as shorts)
 **
 **
 ** compress/expand so that largest value is 32000
 ** -- max resolution
 **
 **
 ** 100's of
 **
 **
 **
 ** gst file
 **
 **/
int
make16bit(
		struct globals *g,
		DQEmgData *dqemgData,
		int fileId,
		int newMuscle
	)
{
	int status;
	char *inputFile;
	char *emgTextFile;
	char *emgMicroFile;
	char *emgMacroFile;
	char idNumberBuffer[16];

	slnprintf(idNumberBuffer, 16, "%d", fileId);

	inputFile = strconcat(
				g->output_dir,
				OS_PATH_DELIM_STRING,
				"emg", idNumberBuffer, ".dat",
				NULL
			);

	emgMicroFile = strconcat(
				g->output_dir,
				OS_PATH_DELIM_STRING,
				"micro", idNumberBuffer, ".dat",
				NULL
			);

	emgMacroFile = strconcat(
				g->output_dir,
				OS_PATH_DELIM_STRING,
				"macro", idNumberBuffer, ".dat",
				NULL
			);

	emgTextFile = strconcat(
				g->output_dir,
				OS_PATH_DELIM_STRING,
				"emg", idNumberBuffer, ".txt",
				NULL
			);

	status = make16bitFile(
				dqemgData,
				emgMicroFile,
				inputFile,
				g->maxShortVoltage,
				emgTextFile,
				g->text_output
			);

	if (status)
	{
		LogInfo("\n\n");
		LogInfo("Output gathered into file:\n");
		LogInfo("    %s\n", emgMicroFile);

		status = makeMacroFile(emgMacroFile, emgMicroFile);
	}

	ckfree(emgTextFile);
	ckfree(emgMacroFile);
	ckfree(emgMicroFile);
	ckfree(inputFile);

	return status;
}

static int
makeMacroFile(char *outputFile, char *inputFile)
{
	EmgData *emg;

	emg = readEmgFile(inputFile);
	if (emg == NULL)
	{
		fprintf(stderr, "Cannot read EMG data from '%s' : %s\n",
				inputFile, strerror(errno));
		return 0;
	}

	if ( ! writeEmgFile(outputFile, emg))
	{
		fprintf(stderr, "Failure writing macro file : %s\n",
				strerror(errno));
		return 0;
	}

	deleteEmgData(emg);

	return 1;
}


int
makeDco(struct globals *g, int fileId)
{
	dcoData *dcoData;
	char *outputFile;
	char idNumberBuffer[16];
	int status;

	slnprintf(idNumberBuffer, 16, "%d", fileId);

	outputFile = strconcat(
				g->output_dir,
				OS_PATH_DELIM_STRING,
				"simulator", idNumberBuffer, ".gst", NULL
			);


	dcoData = createDcoData("testing");
	addMUP(dcoData, createMUP(0, 0, 0, 0, 1.0));
	status = writeDcoFile(outputFile, dcoData);
	deleteDcoData(dcoData);

	ckfree(outputFile);

	return status;
}


