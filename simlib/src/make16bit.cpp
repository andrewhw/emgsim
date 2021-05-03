/**
 ** Convert the output from the emg.dat file into the 16 bit
 ** format required by the DQEMG program.
 **
 ** $Id: make16bit.cpp 28 2020-09-25 20:00:58Z andrew $
 **/

#include "os_defs.h"

#ifndef    MAKEDEPEND
# include    <stdio.h>
# include    <string.h>
# include    <errno.h>
# include    <sys/types.h>
# include    <sys/stat.h>
# ifndef    OS_WINDOWS_NT
#   include    <unistd.h>
# else
#   include    <io.h>
# endif
# include    <fcntl.h>
#endif

#include "DQEmgData.h"

#include "stringtools.h"
#include "error.h"
#include "pathtools.h"
#include "tclCkalloc.h"
#include "reporttimer.h"
#include "io_utils.h"
#include "massert.h"
#include "julian.h"

#include "log.h"

#include "make16bit.h"
#include "SimulatorControl.h"



#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

/*
#define        WRITE_DEBUG_FILE
*/



static short *convertEmgTo16Bit(
		const char *inputFile,
		const char *outputFile,
		EmgHeader *header,
		EmgVoltageDesc *voltageDesc,
		short maxShortVoltage
	);

static int
writeDQEmgFormatFile(
		DQEmgData *dqemgData,
		EmgHeader *header,
		EmgVoltageDesc *voltageDesc,
		short maxShortVoltage,
		short *dataValues
	)
{
	DQEmgChannelData    *microChannel;
	DQEmgChannelData    *macroChannel;
	short *macroData;
	int status = 1;
	int i;
	int macroAllocSize;
	int addMacroChannel = 0;

	if (g->generate_second_channel && dqemgData->getNumChannels() == 0)
	{
		addMacroChannel = 1;
	}

	/* set up and add the micro channel */
	microChannel = new DQEmgChannelData(
		                0,
		                header->emg_hpCutoff * 1000,
		                header->emg_lpCutoff * 1000,
		                "Micro-Channel (indwelling electrode) data");
	microChannel->setElapsedTime(
		                (unsigned long) ((header->emg_elapsedTime
		                        / header->emg_samplingRate) * 1000.0));
	microChannel->setData(
		                header->emg_numberOfSamples,
		                dataValues,
		                (float) header->emg_samplingRate,
		                "µV",
		                (float) (header->emg_scale / 10000.0)
		            );
	dqemgData->addChannel(microChannel);


	/* create a "macro" channel with every 10th sample from micro */
	if (addMacroChannel == 1)
	{
		macroAllocSize = header->emg_numberOfSamples / 10;
		macroData = (short *) ckalloc(macroAllocSize * sizeof(short));
		memset(macroData, 0, macroAllocSize * sizeof(short));
		for (i = 0; (i * 10) < header->emg_numberOfSamples; i++)
		{
			if (i >= macroAllocSize)
				break;
			macroData[i] = dataValues[i*10];
		}

		macroChannel = new DQEmgChannelData(
							0,
							header->emg_hpCutoff * 1000,
							header->emg_lpCutoff * 1000,
							"Macro-Channel (surface electrode) data");
		macroChannel->setElapsedTime(
							(unsigned long) ((header->emg_elapsedTime
									/ header->emg_samplingRate) * 1000.0));
		macroChannel->setData(
							header->emg_numberOfSamples / 10,
							macroData,
							header->emg_samplingRate / 10.0f,
							"µV",
							(float) (header->emg_scale / 10000.0)
						);
		dqemgData->addChannel(macroChannel);
		ckfree(macroData);
	}

	return status;
}


static int
writeTextFormatEMGDataFile(
		FILE *tfp,
		EmgHeader *header,
		EmgVoltageDesc *voltageDesc,
		short maxShortVoltage,
		short *dataValues
	)
{
	int status = 1;
	int i;
	int addMacroChannel = 0;

	if (g->generate_second_channel)
	{
		addMacroChannel = 1;
	}

	fprintf(tfp, "HighPass = %g\n", (float) header->emg_hpCutoff);
	fprintf(tfp, "LowPass = %g\n", (float) header->emg_lpCutoff);
	fprintf(tfp, "ElapsedTime = %ld\n",
		                (unsigned long) ((header->emg_elapsedTime
		                        / header->emg_samplingRate) * 1000.0));
	fprintf(tfp, "SamplingRate = %f\n", (float) header->emg_samplingRate);
	fprintf(tfp, "Units = %s\n", "µV");
	fprintf(tfp, "\n");
	for (i = 0; i < header->emg_numberOfSamples; i++)
	{
		fprintf(tfp, "%g\n",
						((float) dataValues[i]) *
						((float) (header->emg_scale / 10000.0)));
	}

	/* create a "macro" channel with every 10th sample from micro */
	if (addMacroChannel == 1)
	{
		fprintf(tfp, "\n");
		for (i = 0; i < header->emg_numberOfSamples; i+=10)
		{
			fprintf(tfp, "%g\n",
						((float) dataValues[i]) *
						((float) (header->emg_scale / 10000.0)));
		}
	}

	return status;
}

/**
 ** wrapper to read from globals
 **/
int
make16bitFile(
		DQEmgData *dqemgData,
		const char *outputfile,
		const char *inputfile,
		short maxShortVoltage,
		const char *textfile,
		int doTextOutput
	)
{
	FILE *textfp;
	EmgHeader header;
	EmgVoltageDesc voltageDesc;
	short *dataValues = NULL;
	int status = 0;

	memset(&header, 0, sizeof(EmgHeader));

	header.emg_channel = 0;
	header.emg_hpCutoff = 5000;
	header.emg_lpCutoff = 500;
	header.emg_samplingRate = 500;

	/*
	 * compression threshold is actually used as a compression
	 * factor in DQEMG
	 */
	header.emg_compressionThreshold = 1; /* 0000; */
	header.emg_scale = 0;
	header.work_.effectiveScale_ = 1;

	dataValues = convertEmgTo16Bit(inputfile, outputfile,
		    &header, &voltageDesc, maxShortVoltage);

	if (dataValues != NULL)
	{
		status = writeDQEmgFormatFile(
				dqemgData,
		        &header,
		        &voltageDesc,
		        maxShortVoltage,
		        dataValues
		    );

		if (status && doTextOutput)
		{
			textfp = fopenpath(textfile, "w");
			if (textfp != NULL)
			{
				status = writeTextFormatEMGDataFile(
						textfp,
						&header,
						&voltageDesc,
						maxShortVoltage,
						dataValues
					);
				fclose(textfp);
			}
		}

		ckfree(dataValues);
	}


	return status;
}


static int
prescanInputFile(
		EmgHeader *header,
		FP *ifp,
		EmgVoltageDesc *voltageDesc,
		short maxShortVoltage
	)
{
	float startTime, endTime, loadVoltage;
	struct stat sb;
	int size;
	int i;



	/** figure out the size we are going to be */
#if defined OS_WINDOWS
	if (fstat(_fileno(ifp->fp), &sb) < 0)
	{
		Error("Cannot stat file '%s' : %s\n",
		        ifp->name, strerror(errno));
	}
#else
	if (fstat(fileno(ifp->fp), &sb) < 0)
	{
		Error("Cannot stat file '%s' : %s\n",
		        ifp->name, strerror(errno));
	}
#endif
	size = sb.st_size;

	if ((size % sizeof(float)) != 0)
	{
		Error("File size %d is not multiple of sizeof float %d!\n",
		        size, sizeof(float));
		return 0;
	}
	voltageDesc->nFloats_ =
		    (size - (2 * sizeof(float))) / sizeof(float);

	LogInfo("    Scanning %d values\n", voltageDesc->nFloats_);



	/** now start the conversion */
	if ( ! rFloat(ifp, &startTime))                    return 0;
	if ( ! rFloat(ifp, &endTime))                    return 0;



	if ( ! rFloat(ifp, &loadVoltage))            return 0;
	voltageDesc->minVoltage_ = voltageDesc->maxVoltage_ = loadVoltage;

	for (i = 1; i < voltageDesc->nFloats_; i++)
	{

		if ( ! rFloat(ifp, &loadVoltage))            return 0;
		if (voltageDesc->maxVoltage_ < loadVoltage)
		{
		    voltageDesc->maxVoltage_ = loadVoltage;
		}
		if (voltageDesc->minVoltage_ > loadVoltage)
		{
		    voltageDesc->minVoltage_ = loadVoltage;
		}
	}


	/** find absolute max voltage */
	voltageDesc->maxAbsVoltage_ = voltageDesc->minVoltage_;
	if (voltageDesc->maxAbsVoltage_ < 0)
		voltageDesc->maxAbsVoltage_ = (-voltageDesc->maxAbsVoltage_);
	if (voltageDesc->maxAbsVoltage_ < voltageDesc->maxVoltage_)
		voltageDesc->maxAbsVoltage_ = voltageDesc->maxVoltage_;

	if (-voltageDesc->minVoltage_ > voltageDesc->maxVoltage_)
		voltageDesc->maxAbsVoltage_ = voltageDesc->maxVoltage_;


	LogInfo("    Voltage range is        : %f to %f µV\n",
		        (double) (voltageDesc->minVoltage_ * 1000.0),
		        (double) (voltageDesc->maxVoltage_ * 1000.0));
	LogInfo("    Max Absolute Voltage is : %f µV\n",
		        (double) (voltageDesc->maxAbsVoltage_ * 1000.0));



	/** calculate and save header values */
	if (header->emg_scale == 0)
	{


		/*
		 * Effective scale is used to multiply every value
		 * in the emg file in order to force them (maximally)
		 * into the range specified by maxShortVoltage.
		 */
		header->work_.effectiveScale_ = (float)
		        (
		                (double) maxShortVoltage
		                        / (double) voltageDesc->maxAbsVoltage_
		            );


		/*
		 * Convert data from mV to uV, as well as
		 */
		header->emg_scale = (int)
		        ((
		                10000.0 /
		                    (header->work_.effectiveScale_ / 1000.0)
		            ) + 0.5);

		MSG_ASSERT(header->emg_scale < SHRT_MAX, "Scale overflow!\n");


	}
	LogInfo("    Scale Factor            : %d mV\n",
		        (int) header->emg_scale);

	header->emg_elapsedTime =
		    header->emg_numberOfSamples = (long) voltageDesc->nFloats_;


	/* sampling rate is fixed at 31250 */
	header->emg_samplingRate = 31250;

	/*
	 * compression Threshold is fixed at 10000; we do not want
	 * to compress the data in any way using this factor
	 */
	header->emg_compressionThreshold = 10000;


	{
		double calcCheck =
		            (double) voltageDesc->maxAbsVoltage_ *
		                    (double) header->work_.effectiveScale_;
		LogInfo("    Scale Factor Check: Max * scale:\n");
		LogInfo("        %f * %f = %f\n",
		            (double) voltageDesc->maxAbsVoltage_,
		            (double) header->work_.effectiveScale_,
		            calcCheck);
		LogInfo("        4096 - scale Factor = %f\n",
		            (calcCheck - (double) maxShortVoltage));

	}

	return 1;
}

static short *
convertFileData(
		EmgHeader *header,
		FP *ifp,
		EmgVoltageDesc *voltageDesc
	)
{
	float loadVoltage, loadDummy;
	double convertedVoltage;
	short *convertedData = NULL;
	int minValue, maxValue;
	int i;
#    ifdef    WRITE_DEBUG_FILE
	FP    *debugfp = NULL;
#    endif

	/** read in and ignore times -- we have handled them already */
	if ( ! rFloat(ifp, &loadDummy))                    return 0;
	if ( ! rFloat(ifp, &loadDummy))                    return 0;


#    ifdef    WRITE_DEBUG_FILE
	debugfp = openFP("debug-emgraw.txt", "wb");
	if (debugfp == NULL)
	{
		return NULL;
	}
#    endif

	convertedData = (short *)
		        ckalloc(voltageDesc->nFloats_ * sizeof(short));

	LogInfo("    Converting %d values . . .\n", voltageDesc->nFloats_);
	for (i = 0; i < voltageDesc->nFloats_; i++)
	{

		if ( ! rFloat(ifp, &loadVoltage))
		    goto FAIL;


		/**
		 ** Converting millivolts to microvolts (thus the 1000),
		 ** and then scaling into the output file, where the
		 ** scale recovery factor is a fraction with "scale" as
		 ** the numerator, and "compression threshold" as the
		 ** denominator.
		 **
		 ** To Accomodate 16 bit output, we cannot go more than
		 ** one microvolt resolution, which is why the scale factor
		 ** is hard-coded to be one.  The compression threshold
		 ** is therefore also always one.
		 **/


		/** convert to specified range using effective scale */
		convertedVoltage =
		        (loadVoltage * header->work_.effectiveScale_);


		if (convertedVoltage > SHRT_MAX)
		{
		    Error("Overflow in converted voltage!");
		    Error("  (%f * %f) = %f > %d\n",
		            (double) loadVoltage,
		            (double) header->work_.effectiveScale_,
		                (double) convertedVoltage,
		                SHRT_MAX);
		    convertedVoltage = SHRT_MAX;
		}
		if (convertedVoltage < SHRT_MIN)
		{
		    Error("Underflow in converted voltage!\n");
		    Error("  (%f * %f) = %f < %d\n",
		            (double) loadVoltage,
		            (double) header->work_.effectiveScale_,
		                (double) convertedVoltage,
		                SHRT_MIN);
		    convertedVoltage = SHRT_MIN;
		}
		convertedData[i] = (short) convertedVoltage;


#        ifdef    WRITE_DEBUG_FILE
		fprintf(debugfp->fp, "%d %f\n", i, (double) loadVoltage);
#        endif

		if (i == 0)
		{
		    minValue = maxValue = convertedData[i];
		} else
		{
		    if (minValue > convertedData[i])
		        minValue = convertedData[i];
		    if (maxValue < convertedData[i])
		        maxValue = convertedData[i];
		}
	}

#    ifdef    WRITE_DEBUG_FILE
	closeFP(debugfp);
#    endif

	LogInfo("    Range of values: (%d) - (%d)\n", minValue, maxValue);

	return convertedData;


FAIL:
#    ifdef    WRITE_DEBUG_FILE
	if (debugfp != NULL)    closeFP(debugfp);
#    endif

	return NULL;
}


static int
writeConvertedData(
		FP *ofp,
		EmgHeader *header,
		EmgVoltageDesc *voltageDesc,
		short *data
	)
{
	int i;


	for (i = 0; i < voltageDesc->nFloats_; i++)
	{
		if ( ! wEmgDat(ofp, data[i]))
		    return 0;
	}

	return 1;
}

/**
 ** Read an EMG file in floating point
 ** values and convert to a 22-byte
 ** header + 16bit short list.
 **/
short *
convertEmgTo16Bit(
		const char *inputFile,
		const char *outputFile,
		EmgHeader *header,
		EmgVoltageDesc *voltageDesc,
		short maxShortVoltage
	)
{
	FP    *ifp = NULL, *ofp = NULL;
	short *dataAsShorts = NULL;

	ifp = openFP(inputFile, "rb");
	if (ifp == NULL)
	{
		return NULL;
	}


	/** figure out what we are going to do */
	if ( ! prescanInputFile(header, ifp, voltageDesc, maxShortVoltage) )
	{
		goto FAIL;
	}

	if (fseek(ifp->fp, 0, SEEK_SET) < 0)
	{
		Error("Failure rewinding to beginning of file : %s\n",
		        strerror(errno));
		goto FAIL;
	}

	dataAsShorts = convertFileData(header, ifp, voltageDesc);
	if (dataAsShorts == NULL)
	{
		goto FAIL;
	}

	ofp = openFP(outputFile, "wb");
	if (ofp == NULL)
	{
		goto FAIL;
	}

	LogInfo("\n");
	LogInfo("    Writing %d values to '%s'\n",
		        voltageDesc->nFloats_,
		        outputFile);

	if ( ! writeEmgDatHeader(ofp, header) )             goto FAIL;

	if ( ! writeConvertedData(ofp, header, voltageDesc, dataAsShorts) )
		goto FAIL;

	closeFP(ofp);



	/* SUCCESS! */
	closeFP(ifp);
	return dataAsShorts;


FAIL:
	if (ifp != NULL)
		closeFP(ifp);
	if (ofp != NULL)
		closeFP(ofp);
	if (dataAsShorts != NULL)
		ckfree(dataAsShorts);
	return NULL;
}


