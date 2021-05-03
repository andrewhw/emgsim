/**
 ** Convert the output from the emg.dat file into the 16 bit
 ** format required by the DQEMG program.
 **
 ** $Id: emgdat.cpp 8 2008-04-24 22:22:33Z andrew $
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
#endif

#include "emg.h"

#include "stringtools.h"
#include "error.h"
#include "pathtools.h"
#include "tclCkalloc.h"
#include "reporttimer.h"
#include "io_utils.h"



/**
 ** Write out the header used in the EMG data files
 **/
int
writeEmgDatHeader(FP * fp, EmgHeader * header)
{
	size_t startPos, endPos;

	startPos = ftell(fp->fp);

	if (!w2byteInt(fp, header->emg_channel))
		return 0;
	if (!w2byteInt(fp, header->emg_hpCutoff))
		return 0;
	if (!w2byteInt(fp, header->emg_lpCutoff))
		return 0;
	if (!w2byteInt(fp, header->emg_scale))
		return 0;

	if (!w4byteInt(fp, header->emg_samplingRate))
		return 0;
	if (!w4byteInt(fp, header->emg_numberOfSamples))
		return 0;
	if (!w4byteInt(fp, header->emg_elapsedTime))
		return 0;

	if (!w2byteInt(fp, header->emg_compressionThreshold))
		return 0;

	endPos = ftell(fp->fp);

	if ((endPos - startPos) != 22)
	{
		Error("Header size mismatch! Expected %d, got %d\n",
			  22, endPos - startPos);
		return 0;
	}
	return 1;
}


/**
 ** Read in the header used in the EMG data files
 **/
int
readEmgDatHeader(FP * fp, EmgHeader * header)
{
	size_t startPos, endPos;

	startPos = ftell(fp->fp);

	if (!r2byteInt(fp, &header->emg_channel))
		return 0;
	if (!r2byteInt(fp, &header->emg_hpCutoff))
		return 0;
	if (!r2byteInt(fp, &header->emg_lpCutoff))
		return 0;
	if (!r2byteInt(fp, &header->emg_scale))
		return 0;

	if (!r4byteInt(fp, &header->emg_samplingRate))
		return 0;
	if (!r4byteInt(fp, &header->emg_numberOfSamples))
		return 0;
	if (!r4byteInt(fp, &header->emg_elapsedTime))
		return 0;

	if (!r2byteInt(fp, &header->emg_compressionThreshold))
		return 0;

	endPos = ftell(fp->fp);

	if ((endPos - startPos) != 22)
	{
		Error("Header size mismatch! Expected %d, got %d\n",
			  22, endPos - startPos);
		return 0;
	}

	/*
	 * now set up working values
	 */
	header->work_.effectiveScale_ = ((float) header->emg_scale
		 		/ (float) header->emg_compressionThreshold);

		//((header->emg_scale / 10000.0)
		// /(header->emg_compressionThreshold / 10000.0));
		//
		//((header->emg_scale / 10000.0)
		// /header->emg_compressionThreshold);

	return 1;
}


/**
 ** Dump the values from the header to the given FILE pointer
 **/
int
dumpEmgDatHeader(FILE * fp, int indent, EmgHeader * header)
{
	char indentBuffer[128];
	int status = 1;

	slnprintf(indentBuffer, 128, "%*s", indent, "");

	status = status && fprintf(fp,
				"%schannel              : %d\n",
				indentBuffer,
				(int) header->emg_channel);
	status = status && fprintf(fp,
				"%shpCutoff             : %d\n",
				indentBuffer,
				(int) header->emg_hpCutoff);
	status = status && fprintf(fp,
				"%slpCutoff             : %d\n",
				indentBuffer,
				(int) header->emg_lpCutoff);
	status = status && fprintf(fp,
				"%sscale                : %d\n",
				indentBuffer,
				(int) header->emg_scale);

	status = status && fprintf(fp,
				"%ssamplingRate         : %ld\n",
				indentBuffer,
				(long) header->emg_samplingRate);
	status = status && fprintf(fp,
				"%snumberOfSamples      : %ld\n",
				indentBuffer,
				(long) header->emg_numberOfSamples);
	status = status && fprintf(fp,
				"%selapsedTime          : %ld\n",
				indentBuffer,
				(long) header->emg_elapsedTime);

	status = status && fprintf(fp,
				"%scompressionThreshold : %d\n",
				indentBuffer,
				(int) header->emg_compressionThreshold);


	status = status && fprintf(fp,
					"%sCalculated Values:\n", indentBuffer);

	status = status && fprintf(fp,
				"%s    Effective Scale  : %f\n",
				indentBuffer,
				(float) header->work_.effectiveScale_);

	return status;
}
