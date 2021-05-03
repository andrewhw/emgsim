/*++******************************************************************
* Module:               EMGUTIL  source module
*
* Author:               David Dubeau
*
* Date:         May. , 1990
*
*      Description:
*
* Modifications :  Aug '90 David Dubeau
*             Changed makeEmg and extract_MUPs to work without
*             Memory restrictions (well, fewer)
*
*
* $Id: emgutil.cpp 28 2020-09-25 20:00:58Z andrew $
*
*****************************************************************--*/

#ifndef    MAKEDEPEND
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <ctype.h>
# include <string.h>
# include <errno.h>
#endif

#include "tclCkalloc.h"
#include "stringtools.h"
#include "error.h"
#include "SimulatorControl.h"
#include "SimulatorConstants.h"
#include "pathtools.h"
#include "reporttimer.h"
#include "filtertools.h"

#include "log.h"

#include "dco.h"
#include "noise.h"
#include "MUP.h"
#include "MUP_utils.h"
#include "JitterDB.h"
#include "NoiseGenerator.h"

#include "log.h"
#include "massert.h"



#ifdef OS_WINDOWS
		/*
		 * disable _CRT_SECURE_NO_WARNINGS related flags for now,
		 * as they completely break the POSIX interface, as we
		 * will have to re-write wrappers for things like fopen
		 * to make this work more gracefully
		 */
# pragma warning(disable : 4996)
#endif

#define INPUT_BUFFER_SIZE       2048

#define EMG_PEAK_ALIGN          0
#define EMG_AREA_ALIGN          1
#define EMG_ACCEL_ALIGN         2
#define EMG_SLOPE_ALIGN         3
#define EMG_JITTER_ALIGN        4


// #define DUMP_CANNULA

// #define      SAVE_ASCII_EMG_DATA


static int
checkAndAlignMUP(
		MUP *currentMUP,
		int *alignmentPosition
	)
{
	int checkAlignmentPosition;

	checkAlignmentPosition = currentMUP->getCurrentMUPAlignmentPoint(0);

	/** if no alignment posn, there are no high freq MFPs */
	if (checkAlignmentPosition < 0)
		return 0;

	/*
	 * set the alignment position to be the recorded best
	 * alignment -- it is tested above for being +ve
	 */
	*alignmentPosition = checkAlignmentPosition;
	return 1;
}

int
addMUPToDCOonThreshold(
		dcoData *dco,
		MUP *currentMUP,
		int motorUnit,
		int MUPBufferPositionInEmgBuffer
	)
{
	int alignmentPointInMUPBuffer;
	int fileOffsetOfMUPAlignmentPoint;
	float timeOfFileOffsetOfMUPAlignmentPoint;
	dcoMUP *dcoMUP;
	//MUPDataElement *(MUP::*bufferFunction)(int id);

	//bufferFunction = &MUP::getCurrentMUPSlope;

	if ( !  checkAndAlignMUP(currentMUP, &alignmentPointInMUPBuffer))
	{
		return 0;
	}

	fileOffsetOfMUPAlignmentPoint = MUPBufferPositionInEmgBuffer
		        + alignmentPointInMUPBuffer;

	/*
	 *    time (s) = (samples) / (<samples/ms> * 1000)
	 */
	timeOfFileOffsetOfMUPAlignmentPoint =
		(float) (fileOffsetOfMUPAlignmentPoint
		        /(g->smpling_freq * 1000.0));


	/**
	 ** Ok -- the motor unit number being logged
	 ** here must start at one in order to get
	 ** EditDco to work.
	 **
	 ** The zero train is the "unassigned"
	 ** train, we have put one zero train element in, as if there
	 ** are no zero-train elements, this screws
	 ** up EditDco, and it cannot deal with it.
	 **
	 ** We should probably fix this in EditDco,
	 ** or perhaps we should add in a single
	 ** dummy zero-train element at time zero
	 ** or something.
	 **/
	dcoMUP = createMUP(
		    timeOfFileOffsetOfMUPAlignmentPoint,
		        /* offset into file */
		    fileOffsetOfMUPAlignmentPoint,
		    motorUnit + 1,      /* motor unit */
		    (-1),               /* MUP # */
		    1,                  /* certainty */
		    currentMUP->getId());
	if ( ! addMUP(dco, dcoMUP) )
	{
		deleteMUP(dcoMUP);
		return 0;
	}

	currentMUP->setDCOID(dcoMUP->mupMotorUnitNumber_);

	return 1;
}

/**
 ** Filter the EMG buffer between 10 and 10000 Hz
 **/
int filterEmgBuffer(
		float *buffer,
		int bufferLength,
		double samplingRate
	)
{
	double *rawEMG;
	double *filteredEMG;
	double *a, *b;
	int nA, nB, status;
	int i;

	LogInfo("Filtering EMG signal 10Hz -> 10kHz\n");

	rawEMG = (double *) ckalloc((bufferLength + 1) * sizeof(double));
	memset(rawEMG, 0, (bufferLength + 1) * sizeof(double));
	filteredEMG = (double *) ckalloc((bufferLength + 1) * sizeof(double));
	memset(filteredEMG, 0, (bufferLength + 1) * sizeof(double));

	for (i = 1; i <= bufferLength; i++)
	{
		rawEMG[i] = (double) buffer[i];
	}

	/* now get the appropriate filter values */
	status = getABParams(&b, &nB, &a, &nA,
		                // FILTAB_O8_31250_500_10000HZ,
		                FILTAB_O8_31250_10_10000HZ,
		                samplingRate);
	if (status <= 0)
		return 0;

	/* filter the data */
	filtfilt(filteredEMG, rawEMG,
		            bufferLength, b, nB, a, nA);

	/* put the noise in the buffer */
	for (i = 1; i <= bufferLength; i++)
	{
		buffer[i] = (float) filteredEMG[i];
	}

	/* clean up */
	ckfree(rawEMG);
	ckfree(filteredEMG);

	return 1;
}

static int
mapGoldStandardMupTemplates(const char *outputDirectory)
{
	char filename[FILENAME_MAX];
	FILE *mappingFP = NULL, *sourceTemplateFP = NULL, *destTemplateFP = NULL;
	char buffer[BUFSIZ];
	int from, to, len;

	slnprintf(filename, FILENAME_MAX, "%s\\trainMappingTable.dat", g->output_dir);
	mappingFP = fopenpath(filename, "r");
	if (mappingFP == NULL)
	{
		fprintf(stderr, "Error: cannot open mapping file '%s': %s\n",
				filename, strerror(errno));
		return -1;
	}

	while (fscanf(mappingFP, "%d -> %d\n", &from, &to) == 2) {

		if (from == 0)	continue;

		slnprintf(filename, FILENAME_MAX, "%s\\mfap-unmapped-template-%d.txt",
					g->output_dir, from);
		if ((sourceTemplateFP = fopenpath(filename, "r")) == NULL)
		{
			fprintf(stderr, "Error: cannot open 'from' template file '%s' : %s\n",
						filename, strerror(errno));
			return -1;
		}

		slnprintf(filename, FILENAME_MAX, "%s\\mfap-template-%d.txt",
					g->output_dir, to);
		if ((destTemplateFP = fopenpath(filename, "w")) == NULL)
		{
			fprintf(stderr, "Error: cannot open 'to' template file '%s' : %s\n",
						filename, strerror(errno));
			return -1;
		}

		while ((len = fread(buffer, 1, BUFSIZ, sourceTemplateFP)) > 0)
			(void) fwrite(buffer, 1, len, destTemplateFP);

		fclose(destTemplateFP);
		fclose(sourceTemplateFP);
	}
	fclose(mappingFP);

    return 0;
}

/**
 ** ----------------------------------------------------------------
 ** Function:     MAKE_EMG
 **    Description:  Makes an emg using the firing times and MUP files.
 **
 **
 **/
int makeEmg(
		MuscleData *MD,
		int fileId
	)
{
	char input_buffer[INPUT_BUFFER_SIZE];
	int currentFiringTimeIndex, activeMotorUnitIndex;
	int m, i;
	char filename[FILENAME_MAX];
	int nFiringTimes;

	/* Flag to increment or decrement through MUP list */
	//int increment = 0;
	int status;

	float *EMG = NULL;
	float *MUP_buffer = NULL;
	//float StartTime;
	float FinishTime;

	long *firingTimeList;
	long abs_stop_time_smpls;
	long emgBufferLengthInSamples;
	long emgBufferIndex;
#ifdef  DUMP_CANNULA
	float *EMG_cannula = NULL;
	float *EMG_nocannula = NULL;
	long emgCannulaBufferIndex;
	long emgNoCannulaBufferIndex;
#endif /* DUMP_CANNULA */

		/*
		 * offset to the peak of the motor unit in firing
		 * time samples
		 */

	FILE *firingTimeFP = NULL;
	FP *emgFP = NULL;

#ifdef  SAVE_ASCII_EMG_DATA
	FILE *emgAsciiFP = NULL;
#endif

	double jitterVarianceInSamples;

	dcoData *dco = NULL;
	int MUPsInGst = 0, totalMUPs = 0;
	int nMUPsOverflow = 0;
	struct report_timer *reportTimer;

	MotorUnit *currentMotorUnit;
	MUP    *currentMUP = NULL;
	int MUPsRecordedForCurrentMUP = 0;
	//double MUPMaxAcceleration = 0;



	jitterVarianceInSamples = (g->jitter / 1000.0 ) / DELTA_T_EMG;

	/** create the dco data to write the log to */
	dco = createDcoData("simulation-DCO");

	/** add a MUP 0 at location zero to make EditDCO happy */
	{
		addMUP(dco, createMUP(
		            0,          /* time */
		            0,          /* offset into file */
		            0,          /* motor unit */
		            (-1),       /* MUP # */
		            0           /* certainty */
		        ));
	}


	/*
	 * emgBufferLengthInSamples is in
	 * #'s of samplts = (s * 1000) / (samples/ms)
	 */
	{
		float nSeconds;

		// StartTime = 0;
		FinishTime = (float) g->emg_elapsed_time;
		nSeconds = FinishTime;

		emgBufferLengthInSamples = (long)
		        (((nSeconds + 1) * 1000.0) / DELTA_T_EMG);
	}

#    ifdef    USE_JITTER_DB
	{
		char buf[2048];

		JitterDB::sSetSamplingRate((float) DELTA_T_EMG);

		slnprintf(buf, 2048,
				"%s/jitter-raw%d", g->output_dir, fileId);
		JitterDB::sGet(JitterDB::RAW)->setOutputDir(buf);

		slnprintf(buf, 2048,
				"%s/jitter-gauss%d", g->output_dir, fileId);
		JitterDB::sGet(JitterDB::GAUSS)->setOutputDir(buf);

		slnprintf(buf, 2048,
				"%s/jitter-buffer%d", g->output_dir, fileId);
		JitterDB::sGet(JitterDB::BUFFER)->setOutputDir(buf);
	}
#    endif

	slnprintf(filename, FILENAME_MAX, "%s%cemg%d.dat",
			g->output_dir,
			OS_PATH_DELIM,
			fileId);
	emgFP = openFP(filename, "wb");
	if (emgFP == NULL)
	{
		Error("Failed to create emg file : %s\n", strerror(errno));
		goto FAIL;
	}
	{
		float dummyStartTime = 0;
		if ( ! wFloat(emgFP, dummyStartTime))
		{
			Error("Cannot write to emg file : %s\n", strerror(errno));
			goto FAIL;
		}
	}
	wFloat(emgFP, FinishTime);


#ifdef  SAVE_ASCII_EMG_DATA
	slnprintf(filename, FILENAME_MAX,
			"%s\\ascii-emg%d.dat", g->output_dir, fileId);
	emgAsciiFP = fopenpath(filename, "wb");
	if (emgAsciiFP == NULL)
	{
		Error("Failed to create ascii emg file : %s\n",
		        strerror(errno));
		goto FAIL;
	}
#endif


	/* calculate the start and finish times in numbers of samples */
	/* still in time format in 0.1 milliseconds */
	abs_stop_time_smpls = (long)
		((emgBufferLengthInSamples * DELTA_T_EMG)
		                / DELTA_T_FIRING_TIMES);

	EMG = (float *) ckalloc(emgBufferLengthInSamples * sizeof(float));
	memset(EMG, 0, emgBufferLengthInSamples * sizeof(float));

#ifdef  DUMP_CANNULA
	EMG_cannula = (float *) ckalloc(emgBufferLengthInSamples
		                * sizeof(float));
	memset(EMG_cannula, 0, emgBufferLengthInSamples * sizeof(float));

	EMG_nocannula = (float *) ckalloc(emgBufferLengthInSamples
		                * sizeof(float));
	memset(EMG_nocannula, 0, emgBufferLengthInSamples * sizeof(float));
#endif /* DUMP_CANNULA */


	reportTimer =
				startReportTimer(MD->getNumActiveInDetectMotorUnits());

	/***
	 ***    summate active MUPTs
	 ***/
	for (activeMotorUnitIndex = 0;
		        activeMotorUnitIndex <
								MD->getNumActiveInDetectMotorUnits();
		                activeMotorUnitIndex++)
	{


		LogInfo("    Recording firings for MUP %s\n",
		                reportTime(activeMotorUnitIndex, reportTimer));


		/* load the MUP buffer */
		if (currentMUP != NULL)
		{
		    delete currentMUP;
		    currentMUP = NULL;
		}
		currentMotorUnit =
		        MD->getActiveInDetectMotorUnit(activeMotorUnitIndex);
		currentMUP = new MUP(g->MUPs_dir, currentMotorUnit->getID());
		currentMUP->load();
		MUPsRecordedForCurrentMUP = 0;
		//MUPMaxAcceleration = 0;

		//increment = TRUE;



		/* open and read the  motor unit firing times file */
		slnprintf(filename, FILENAME_MAX, "%s\\FTMU%d.dat",
		        g->firings_dir, currentMotorUnit->getID());


		if ((firingTimeFP = fopenpath(filename, "rb")) == NULL)
		{
		    Error("\nError : Failed to open firing times file %s ",
		            filename);
		    goto FAIL;
		}

		do
		{
		    if (fgets(input_buffer, INPUT_BUFFER_SIZE, firingTimeFP) == NULL)
			{
		        Error("Bad return from fgets %s(%d)\n", __FILE__, __LINE__);
		        return 0;
		    }
		} while (input_buffer[0] == '#');
		if (sscanf(input_buffer, "%d", &nFiringTimes) != 1)
		{
		        Error("Cannot parse firing times from:\n%s\n", input_buffer);
		        return 0;
		}

		/* allocate a vector to contain the firing time samples */
		firingTimeList =
		    (long *) ckalloc((nFiringTimes) * (sizeof(long)));
		if (firingTimeList == NULL)
		{
		    Error("\n\nError in allocating firing time vector.");
		    goto FAIL;
		}


		for (currentFiringTimeIndex = 0;
						currentFiringTimeIndex < nFiringTimes;
										currentFiringTimeIndex++)
		{
		    long loadValue;
			char *inputLine;
		    do
			{
		        inputLine = fgets(input_buffer,
								INPUT_BUFFER_SIZE, firingTimeFP);
		        if (inputLine == NULL)
				{
		            Error("Bad return from fgets %s(%d)\n", __FILE__, __LINE__);
		            return 0;
		        }
		    } while (input_buffer[0] == '#');

		    if (sscanf(input_buffer, "%ld", &loadValue) != 1)
			{
		            LogError("Cannot parse firing time %d from:\n%s\n",
		                        currentFiringTimeIndex, input_buffer);
		            return 0;
		    }
		    firingTimeList[currentFiringTimeIndex] = loadValue;
		}
		fclose(firingTimeFP);


		/** ensure that we have no leftover jitters for this MUP */
		currentMUP->resetJitterAccounting();

		/** add in every firing of this MUP */
		currentFiringTimeIndex = 0;
		while (currentFiringTimeIndex < nFiringTimes)
		{

		    /*
		     *    If beginning of MUP is past end of EMG finish
		     *    time then break
		     */
		    if (abs_stop_time_smpls <= firingTimeList[currentFiringTimeIndex])
			{
		        break;
		    }

		    /*
		     * the memory the buffer points to is managed by the
		     * currentMUP structure
		     */
		    if ( currentMUP->calcJitteredMUP(0,
		                g->doJitter,
		                (float) jitterVarianceInSamples,
		                (MUP::ReferenceSetup) g->needleReferenceSetup
		            ) <= 0)
			{
		        break;
		    }

		    MUP_buffer = currentMUP->getCurrentMUP(0);
		    if (MUP_buffer == NULL)
			{
		        break;
		    }

		    emgBufferIndex = (long)
		            ((firingTimeList[currentFiringTimeIndex]
		                        * DELTA_T_FIRING_TIMES)
		                                / DELTA_T_EMG
		            );
#ifdef  DUMP_CANNULA
		    emgCannulaBufferIndex = emgBufferIndex;
		    emgNoCannulaBufferIndex = emgBufferIndex;
#endif  /* DUMP_CANNULA */

		    totalMUPs++;

		    /** add in the MUP to the dco file */
		    if (0 <= firingTimeList[currentFiringTimeIndex])
			{
		        status = addMUPToDCOonThreshold(
		                    dco,
		                    currentMUP,
		                    activeMotorUnitIndex,
		                    emgBufferIndex
		                );
				if (status < 0)
				{
					LogError("Adding MUP from MU %d to DCO failed\n",
							currentMUP->getId());
					goto FAIL;
				} else if (status > 0)
				{
		            MUPsInGst++;
		            MUPsRecordedForCurrentMUP++;
		        } else
				{
					static int warnPrinted = 0;
					if (warnPrinted == 0)
					{
						warnPrinted = 1;
						LogWarn("Too many MUPs in record -- suppressing add to DCO\n");
					}
		            nMUPsOverflow++;
				}
		    }


		    /*
		     * emgBufferIndex = offset in EMG buffer
		     *            in sample units to the start
		     *            of the current MUP
		     *          = (absolute MUP start time in
		     *              EMG samples)
		     *            - (start time in EMG samples)
		     *          = (number of EMG samples to the
		     *              start of the current MUP)
		     */
		    for (m = 0; m < MUP_LENGTH; m++)
			{
		        if ((emgBufferIndex >= 0) &&
						(emgBufferIndex < emgBufferLengthInSamples))
				{
		            EMG[emgBufferIndex] += MUP_buffer[m];
		        }
		        emgBufferIndex++;
		    }


#ifdef  DUMP_CANNULA
		    /*
		     * record the cannula-specific values
		     */
			{
		        float *cannulaBuffer;
		        float *noCannulaBuffer;

		        (void) currentMUP->calcJitteredMUP(0,
		                    g->doJitter,
		                    (float) jitterVarianceInSamples,
		                    MUP::CANNULA_ONLY
		                );

		        cannulaBuffer = currentMUP->getCurrentMUP(0);

		        for (m = 0; m < MUP_LENGTH; m++)
				{
		            if ((emgCannulaBufferIndex >= 0) &&
		                    (emgCannulaBufferIndex
		                                < emgBufferLengthInSamples))
					{
		                EMG_cannula[emgCannulaBufferIndex] +=
		                                    cannulaBuffer[m];
		            }
		            emgCannulaBufferIndex++;
		        }

		        (void) currentMUP->calcJitteredMUP(0,
		                    g->doJitter,
		                    (float) jitterVarianceInSamples,
		                    MUP::TIP_ONLY
		                );

		        noCannulaBuffer = currentMUP->getCurrentMUP(0);

		        for (m = 0; m < MUP_LENGTH; m++)
				{
		            if ((emgNoCannulaBufferIndex >= 0) &&
		                    (emgNoCannulaBufferIndex
		                                < emgBufferLengthInSamples))
					{
		                EMG_nocannula[emgNoCannulaBufferIndex] +=
		                                    noCannulaBuffer[m];
		            }
		            emgNoCannulaBufferIndex++;
		        }
		    }
#endif  /* DUMP_CANNULA */


		    currentFiringTimeIndex++;
		}



		/*
		 * Now generate and save the "gold standard template jittered MUP"
		 */
		{
			FILE *mfpTemplateFP = NULL;
			float *mfpData = NULL;
			int m, alignmentPoint;

			(void) currentMUP->calcJitteredMUP(0,
		                g->doJitter,
		                (float) jitterVarianceInSamples,
		                (MUP::ReferenceSetup) g->needleReferenceSetup,
						MUP::JITTER_TEMPLATE
		            );

			/**
			 * if alignment point is negative, there are no fibres
			 * to align with, and we should ignore this train
			 */
			alignmentPoint = currentMUP->getCurrentMUPAlignmentPoint(0);

			if (alignmentPoint > 0 && currentMUP->getDCOID() > 0) {

				slnprintf(filename, FILENAME_MAX, "%s\\mfap-unmapped-template-%d.txt",
		                g->output_dir, currentMUP->getDCOID());
				mfpTemplateFP = fopenpath(filename, "wb");
				LogDebug(__FILE__, __LINE__, "Dumping MFP jitter template\n");

				mfpData = currentMUP->getCurrentMUP(0);
				for (m = 0; m < MUP_LENGTH; m++)
				{
					fprintf(mfpTemplateFP, "%d %f\n",
						m - alignmentPoint, mfpData[m]);
				}
				fclose(mfpTemplateFP);
			}
		}


		LogInfo("%*sMUP %2d (%2d) generated %d significant firings\n",
		                10, "",
		                activeMotorUnitIndex,
		                currentMUP->getId(),
		                MUPsRecordedForCurrentMUP);
		/*
		LogInfo("%*swith threshold of %f (max %f) kV/ss\n",
		                14, "",
		                GST_ACCEL_THRESHOLD,
		                MUPMaxAcceleration);
		*/
		ckfree(firingTimeList);
	}

	deleteReportTimer(reportTimer);

	LogInfo("\nGeneration Complete\n\n");

	if (g->use_noise)
	{
		addNoiseToBuffer(EMG, emgBufferLengthInSamples,
		                1000.0/DELTA_T_EMG, g->signalToNoiseRatio);
	}

	if (g->filter_raw_signal)
	{
		filterEmgBuffer(EMG,
		        emgBufferLengthInSamples - 1,
		        1000.0/DELTA_T_EMG);
	}

	/*
	 * save this section of emg
	 * free the memory used
	 * write the emg to file
	 */
	LogInfo("Writing EMG buffer of %d samples\n",
		        emgBufferLengthInSamples);

	for (i = 0; i < emgBufferLengthInSamples; i++)
	{
		if ( ! wFloat(emgFP, EMG[i]))
		{
			Error("EMG write failure\n");
			goto FAIL;
		}
	}


#ifdef  SAVE_ASCII_EMG_DATA
	{
		int i;
		for (i = 0 ; i < emgBufferLengthInSamples; i++)
		{
		    fprintf(emgAsciiFP, "%d %f\n", i, EMG[i]);
		    // fprintf(noisefp->fp, "%d %f\n", i, EMGnoise[i]);
		}
	}

	fclose(emgAsciiFP);
	emgAsciiFP = NULL;
#endif

#ifdef  DUMP_CANNULA
	{
		char filename[FILENAME_MAX];
		FILE *cannulaFP;
		FILE *nocannulaFP;
		int i;

		slnprintf(filename, FILENAME_MAX "%s\\cannula-emg%d.txt",
		                g->output_dir, fileId);
		cannulaFP = fopenpath(filename, "wb");

		slnprintf(filename, FILENAME_MAX, "%s\\nocannula-emg%d.txt",
		                g->output_dir, fileId);
		nocannulaFP = fopenpath(filename, "wb");

		LogDebug(__FILE__, __LINE__,
		        "Dumping debug cannula info\n");
		if (cannulaFP != NULL && nocannulaFP != NULL)
		{
		    for (i = 0 ; i < emgBufferLengthInSamples; i++)
			{
		        fprintf(  cannulaFP, "%d %f\n", i, EMG_cannula[i]);
		        fprintf(nocannulaFP, "%d %f\n", i, EMG_nocannula[i]);
		    }
		}
		fclose(nocannulaFP);
		fclose(  cannulaFP);
	}
	ckfree(EMG_cannula);
	ckfree(EMG_nocannula);
#endif  /* DUMP_CANNULA */

	ckfree(EMG);

	LogInfo("%d MUPs added\n\n", totalMUPs);


	if (currentMUP != NULL)
	{
		delete currentMUP;
		currentMUP = NULL;
	}

	closeFP(emgFP);

	LogInfo("EMG file \"%s%cemg%d.dat\" created.\n",
			g->output_dir,
			OS_PATH_DELIM,
			fileId);
	LogInfo("\n\n");


	/** write out GST file we have been compiling */
	{
		char numbuf[16];
		char *dconame = NULL;

		slnprintf(numbuf, 16, "%d", fileId);
		dconame = strconcat(
		            g->output_dir,
		            OS_PATH_DELIM_STRING,
		            "micro", numbuf, ".gst",
		            NULL
		        );

		// fprintf(stdout, "DCO List before sort:\n");
		// dumpDco(stdout, 4, dco);
		slnprintf(filename, FILENAME_MAX, "%s\\trainMappingTable.dat", g->output_dir);
		sortDcoData(dco, 1, filename);
		// fprintf(stdout, "DCO List after sort:\n");
		// dumpDco(stdout, 4, dco);
		writeDcoFile(dconame, dco);
		LogInfo("Wrote GST file : %s\n", dconame);
		LogInfo("    GST file contains %d of %d generated MUPS -- %5.2f%%\n",
		        MUPsInGst, totalMUPs,
		        (float) (100.0 * MUPsInGst / (double) totalMUPs));
		if (nMUPsOverflow > 0)
		{
			LogWarn("    %d MUPs were generated which could not be recorded in GST file\n",
					nMUPsOverflow);
		}

		mapGoldStandardMupTemplates(g->output_dir);

		ckfree(dconame);
		// dumpDco(stdout, 8, dco);
		// dumpDcoVerbose(stdout, 8, dco);
//		{
//			FILE *fp;
//
//		    slnprintf(filename, FILENAME_MAX, "%s\\gst_mu_map%d.dat",
//		                g->output_dir, fileId);
//		    if ((fp = fopenpath(filename, "w")) == NULL)
//			{
//		        Error("Failed to create gst_mu_map%d.dat file", fileId);
//		        goto FAIL;
//		    }
//		    //fprintf(fp, "# GST motor unit mapping file\n");
//		    //fprintf(fp, "# The first column contains the internal MU\n");
//		    //fprintf(fp, "# numbers, the second the numbers in the ouput\n");
//		    //fprintf(fp, "# GST file\n");
//		    //fprintf(fp, "#\n");
//
//		    /**
//		     * print out the internal MUP numbers we have used
//		     * to record values into the GST file.
//		     */
//		    for (i = 0; i < dco->nTrains_; i++)
//			{
//		        if (dco->trainList_[i].trainId_ > 0)
//				{
//		            fprintf(fp, "  %3ld %3ld\n",
//		                        (long) dco->trainList_[i].userMUPId_,
//		                        (long) i);
//		        }
//		    }
//
//		    fclose(fp);
//		}

		deleteDcoData(dco);
	}

#    ifdef    USE_JITTER_DB
	/** write out the jitter info */
	{
		JitterDB::sGet(JitterDB::RAW)->saveHist();
		JitterDB::sGet(JitterDB::GAUSS)->saveHist();
		JitterDB::sGet(JitterDB::BUFFER)->saveHist();
	}
#    endif
	JitterDB::sCleanup();

	return fileId;


FAIL:   /** clean up on failure */
	if (dco != NULL)        deleteDcoData(dco);
	return -1;
}

