
/**
 ** Handle the reading and writing of data structures to hold
 ** EMG data.
 **
 ** $Id: emg.cpp 39 2010-07-19 15:22:11Z andrew $
 **/

#include "os_defs.h"

#ifndef MAKEDEPEND
#include	   <stdio.h>
#include	   <string.h>
#include	   <errno.h>
#include	   <sys/types.h>
#include	   <sys/stat.h>
#ifndef		OS_WINDOWS_NT
#include	 <unistd.h>
#else
#include	 <io.h>
#endif
#include	   <fcntl.h>
#include	   <stdlib.h>
#endif

#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#include "emg.h"

#include "massert.h"
#include "msgisort.h"
#include "stringtools.h"
#include "error.h"
#include "pathtools.h"
#include "tclCkalloc.h"
#include "reporttimer.h"
#include "io_utils.h"

#define		 BLOCK_SIZE			  24

#define		 MAX_INDENT			  16
static char sIndentBuffer[MAX_INDENT];


/** create a blank EMG object */
EmgData *
createEmgData()
{
	EmgData *result;

	result = (EmgData *) ckalloc(sizeof(EmgData));

	memset(result, 0, sizeof(EmgData));
	return result;
}

/** destroy one of these objects */
void
deleteEmgData(EmgData * data)
{
	if (data == NULL)
		return;

	if (data->scaledData_ != NULL)
		ckfree(data->scaledData_);

	if (data->data_ != NULL)
		ckfree(data->data_);

	ckfree(data);
}

void
dumpEmg(FILE * fp, int indent, EmgData * data)
{
	int i;
	int min, max;

	slnprintf(sIndentBuffer, MAX_INDENT, "%*s", indent, "");

	dumpEmgDatHeader(fp, indent + 4, &data->definition_);

	min = max = data->data_[0];
	for (i = 1; i < data->definition_.emg_numberOfSamples; i++)
	{
		if (min > data->data_[i])
			min = data->data_[i];
		if (max < data->data_[i])
			max = data->data_[i];
	}

	fprintf(fp, "%sRange %d : %d A/D units\n", sIndentBuffer, min, max);
	fprintf(fp, "%sRange %f : %f uV\n", sIndentBuffer,
			(double) (min * data->definition_.work_.effectiveScale_),
			(double) (max * data->definition_.work_.effectiveScale_));
}

void
dumpEmgVerbose(FILE * fp, int indent, EmgData * data)
{
	int i;

	slnprintf(sIndentBuffer, MAX_INDENT, "%*s", indent, "");

	dumpEmg(fp, indent, data);

	for (i = 0; i < data->definition_.emg_numberOfSamples && i < 10000; i++)
	{
		fprintf(fp, "%s%d : %d\n",
				sIndentBuffer,
				i, (int) data->data_[i]);
	}
}

/**
 ** Write out a EMG file
 **/
int
writeEmgFile(const char *outputFile, EmgData * data)
{
	FP *ofp;
	int i;

	if ((ofp = openFP(outputFile, "wb")) == NULL)
		return 0;

	if (!writeEmgDatHeader(ofp, &data->definition_))
		return 0;
	
	for (i = 0; i < data->definition_.emg_numberOfSamples; i++)
		if (!wEmgDat(ofp, data->data_[i]))
			return 0;

	closeFP(ofp);
	return 1;
}


/**
 ** Read in a EMG file
 **/
EmgData *
readEmgFile(const char *inputFile)
{
	EmgData *result;
	FP *ifp;
	int i;

	if ((ifp = openFP(inputFile, "rb")) == NULL)
	{
		fprintf(stderr, "Cannot open file '%s' : %s\n",
						inputFile, strerror(errno));
		return NULL;
	}

	result = createEmgData();

	if (!readEmgDatHeader(ifp, &result->definition_))
	{
		fprintf(stderr, "Failed reading header info\n");
		return NULL;
	}
	result->data_ = (emgValue *)
		ckalloc(sizeof(emgValue)
				* result->definition_.emg_numberOfSamples);

	for (i = 0; i < result->definition_.emg_numberOfSamples; i++)
	{
		if (!rEmgDat(ifp, &result->data_[i]))
		{
			fprintf(stderr, "Failed reading EMG data\n");
			return NULL;
		}
	}

	closeFP(ifp);
	return result;
}

/**
 ** Convert the data to the scaled value
 **/
void
scaleEmgData(EmgData * data)
{
	int i;

	if (data->scaledData_ == NULL)
	{
		data->scaledData_ = (float *)
			ckalloc(sizeof(float)
					* data->definition_.emg_numberOfSamples);
	}
	for (i = 2; i < data->definition_.emg_numberOfSamples; i++)
	{
		data->scaledData_[i] =
			data->definition_.work_.effectiveScale_
			* (float) data->data_[i];
	}
}

