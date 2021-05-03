
/**
 ** Handle the reading and writing of data structures to hold
 ** PRM data.
 **
 ** $Id: prm.cpp 52 2017-10-08 14:50:28Z andrew $
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

#include "prm.h"

#include "massert.h"
#include "msgisort.h"
#include "stringtools.h"
#include "error.h"
#include "pathtools.h"
#include "mathtools.h"
#include "tclCkalloc.h"
#include "reporttimer.h"
#include "io_utils.h"
#include "tokens.h"

#define		 BLOCK_SIZE			  24

#define		 MAX_INDENT			  16
static char sIndentBuffer[MAX_INDENT];


static const char *sParamName[] = {
	"MicroVPkPk",
	"MicroDuration",
	"MicroPhases",
	"MicroTurns",
	"MicroAAR",
	"MacroVPkPk",
	"MacroArea",
	"MacroNegPkAmp",
	"MacroNegPkArea",
	"MacroNegPkDur",
	"IPImean",
	"IPIsd",
	"IPIcov",
	"FRmean",
	"FRmcd",
	"NF_Jiggle",
	"BJiggle",
	"Jiggle",
	"CNJitter",
	"Blocking",
	"FibreCount",
	"IDRate",
	"NumMups",
	"MMuV",
	"PeakToPeakAmpMUNE",
	"NegPeakAmpMUNE",
	"NegPeakAreaMUNE",
	NULL
};

static const char *sCalculatedParamName[] = {
	"SizeIndex",
	"LogAmplitude",
	NULL
};

PrmParameterDescription sParamDescriptions_[] = {
	{Prm_Micro,
		Prm_MicroVPkPk,
		1,
		"Amplitude",
	"µV"},
	{Prm_Micro,
		Prm_MicroDuration,
		1,
		"Duration",
	"ms"},
	{Prm_Micro,
		Prm_MicroPhases,
		1,
		"Phases",
	""},
	{Prm_Micro,
		Prm_MicroTurns,
		1,
		"Turns",
	""},
	{Prm_Micro,
		Prm_MicroAAR,
		1,
		"AAR",
	"ms"},
	{Prm_Macro,
		Prm_MacroVPkPk,
		1,
		"SurfAmplitude",
	"µV"},
	{Prm_Macro,
		Prm_MacroArea,
		1,
		"SurfArea",
	"µVms"},
	{Prm_Macro,
		Prm_MacroNegPkAmp,
		1,
		"NegPeakAmplitude",
	"µV"},
	{Prm_Macro,
		Prm_MacroNegPkArea,
		1,
		"NegPeakArea",
	"µVms"},
	{Prm_Macro,
		Prm_MacroNegPkDur,
		1,
		"NegPeakDuration",
	"ms"},
	{Prm_IPI,
		Prm_IPImean,
		1,
		"MeanIDI",
	"ms"},
	{Prm_IPI,
		Prm_IPIsd,
		1,
		"IDIStandardDeviation",
	"ms"},
	{Prm_IPI,
		Prm_IPIcov,
		1,
		"IDICoeffVar",
	"ms"},
	{Prm_FR,
		Prm_FRmean,
		1,
		"FiringRate",
	"Hz"},
	{Prm_FR,
		Prm_FRmcd,
		1,
		"FiringRateMeanConsecutiveDifference",
	"Hz"},


	{Prm_Ensemble,
		Prm_NF_Jiggle,
		0,
		"NF_Jiggle",
	""},
	{Prm_Ensemble,
		Prm_BJiggle,
		0,
		"BJiggle",
	""},
	{Prm_Ensemble,
		Prm_Jiggle,
		0,
		"Jiggle",
	""},
	{Prm_Ensemble,
		Prm_CNJitter,
		0,
		"CNJitter",
	"µs"},
	{Prm_Ensemble,
		Prm_Blocking,
		0,
		"PerCentBlocking",
	"%"},
	{Prm_Ensemble,
		Prm_FibreCount,
		0,
		"FibreCount",
	""},

	{Prm_Misc,
		Prm_IDRate,
		1,
		"IdentificationRate",
	"%"},
	{Prm_Misc,
		Prm_NumMups,
		1,
		"NumberOfMUPs",
	""},
	{Prm_Misc,
		Prm_MMuV,
		1,
		"MUMeanVoltage",
	"µV"},
	{Prm_Misc,
		Prm_PeakToPeakAmpMUNE,
		0,
		"PeakToPeakAmpMUNE",
	""},
	{Prm_Misc,
		Prm_NegPeakAmpMUNE,
		0,
		"NegPeakAmpMUNE",
	""},
	{Prm_Misc,
		Prm_NegPeakAreaMUNE,
		0,
		"NegPeakAreaMUNE",
	""}
};

const char *sParamGroupNames_[] = {
	"Contraction",
	"Micro",
	"Macro",
	"IPI",
	"FR",
	"Ensemble",
	"Misc",
	NULL
};

const char *sCalculatedParamGroupNames_[] = {
	"Dummy",
	"Micro",
	NULL
};

static size_t sMaxGroupNameLen_ = 0;
static size_t sMaxParamNameLen_ = 0;
static size_t sMaxUnitLen_ = 0;


PrmParameterDescription sCalculatedParamDescriptions_[] = {
	{Prm_Micro,
		Prm_SizeIndex,
		1,
		"SizeIndex",
	""},
	{Prm_Micro,
		Prm_LogAmplitude,
		0,
		"LogAmplitude",
	""}
};



static int 
sPrmCalc_SizeIndex(
		PrmData * result,
		int trainIndex
	)
{
	/**
	 *
	 * Size index is defined as:
	 *   SI = 2 \ln \mbox{amplitude} +
	 *				\frac{\mbox{area}}{\mbox{amplitude}}
	 */
	double amplitude, AAR, SI;

	amplitude = result->train_[trainIndex].param_[Prm_MicroVPkPk];
	AAR = result->train_[trainIndex].param_[Prm_MicroAAR];

	SI = 2.0 * log(amplitude) + AAR;

	result->train_[trainIndex].calculatedParam_[
					Prm_SizeIndex - Prm_MAX_PARAM
				] = SI;
	return 1;
}

static int 
sPrmCalc_LogAmplitude(
		PrmData * result,
		int trainIndex
	)
{
	double amplitude, logAmplitude;

	amplitude = result->train_[trainIndex].param_[Prm_MicroVPkPk];

	logAmplitude = log10(amplitude);

	result->train_[trainIndex].calculatedParam_[
					Prm_LogAmplitude - Prm_MAX_PARAM
				] = logAmplitude;
	return 1;
}

/**
 * list of function pointers to calculation functions
 */
static int (*sPrmCalculation_[])(PrmData * result, int trainIndex) =
{
	sPrmCalc_SizeIndex,
	sPrmCalc_LogAmplitude
};


static int
calcParamStats(PrmData * result)
{
	double meanSum, varSum;
	double mean;
	int numValid;
	int i, j, k;

	if (result->baseParameterSummary_ != NULL)
	{
		ckfree(result->baseParameterSummary_);
	}
	result->baseParameterSummary_ = (PrmSummaryData *)
		ckalloc(result->numberOfTrainParameters_
				* sizeof(PrmSummaryData));
	memset(result->baseParameterSummary_, 0,
		   result->numberOfTrainParameters_ * sizeof(PrmSummaryData));

	if (result->calcParameterSummary_ != NULL)
	{
		ckfree(result->calcParameterSummary_);
	}
	result->calcParameterSummary_ = (PrmSummaryData *)
		ckalloc((Prm_MAX_CALCULATED_PARAM - Prm_MAX_PARAM)
				* sizeof(PrmSummaryData));
	memset(result->calcParameterSummary_, 0,
		   (Prm_MAX_CALCULATED_PARAM - Prm_MAX_PARAM)
		   * sizeof(PrmSummaryData));

	for (i = 0; i < result->numberOfTrainParameters_; i++)
	{
		meanSum = varSum = 0;
		numValid = 0;
		for (j = 0; j < result->numberOfTrains_; j++)
		{
			if (result->train_[j].isInvalid_ == 0)
			{
				meanSum += result->train_[j].param_[i];
				varSum += SQR(result->train_[j].param_[i]);
				numValid++;
			}
		}

		if (numValid > 0)
		{
			result->baseParameterSummary_[i].valid_ = numValid;
			mean = result->baseParameterSummary_[i].mean_ =
				(double) (meanSum / numValid);

			result->baseParameterSummary_[i].variance_ =
				(double) (1.0 / double (numValid - 1)
						  * (varSum - numValid * mean * mean));
		} else
		{
			result->baseParameterSummary_[i].valid_ = 0;
			result->baseParameterSummary_[i].mean_ =
				result->baseParameterSummary_[i].variance_ = 0;
		}
	}

	for (i = Prm_MAX_PARAM; i < Prm_MAX_CALCULATED_PARAM; i++)
	{
		meanSum = varSum = 0;
		numValid = 0;
		k = i - Prm_MAX_PARAM;

		for (j = 0; j < result->numberOfTrains_; j++)
		{
			if (result->train_[j].isInvalid_ == 0)
			{
				meanSum += result->train_[j].calculatedParam_[k];
				varSum += SQR(result->train_[j].calculatedParam_[k]);
				numValid++;
			}
		}


		if (numValid > 0)
		{
			result->calcParameterSummary_[k].valid_ = numValid;
			mean = result->calcParameterSummary_[k].mean_ =
				(double) (meanSum / numValid);

			result->calcParameterSummary_[k].variance_ =
				(double) (1.0 / double (numValid - 1)
						  * (varSum - numValid * mean * mean));
		} else
		{
			result->calcParameterSummary_[k].valid_ = 0;
			result->calcParameterSummary_[k].mean_ =
				result->baseParameterSummary_[k].variance_ = 0;
		}
	}

	return 1;
}

#ifdef DISABLE_JITTER_10_2010
static int
getNewline(tokenizer * t)
{
	token *token;

	token = tknGetToken(t);
	if (token->type_ != '\n')
		return 0;

	return 1;
}
#endif /* DISABLE_JITTER_10_2010 */

static int
skipToNewline(tokenizer * t)
{
	token *token;

	token = tknGetToken(t);
	while (token->type_ != '\n' && token->type_ != TT_EOF)
		token = tknGetToken(t);

	return token->type_ == '\n' ? 1 : 0;
}


#ifdef DISABLE_JITTER_10_2010
static int
getInt(tokenizer * t, int *target)
{
	token *token;

	token = tknGetToken(t);

	if (token == NULL)
	{
		fprintf(stderr, "No token for integer at line %d\n", tknGetLineNo(t));
		return 0;
	}
	if (token->type_ == TT_EOF)
	{
		fprintf(stderr, "Unexpected EOF at %d\n", tknGetLineNo(t));
		return 0;
	}
	if (token->type_ == TT_INTEGER)
	{
		*target = token->data_.ival_;
	} else
	{
		fprintf(stderr, "Expected integer data at line %d\n", tknGetLineNo(t));
		return 0;
	}

	return 1;
}
#endif /* DISABLE_JITTER_10_2010 */

static int
getFloat(tokenizer * t, double *target)
{
	token *token;

	token = tknGetToken(t);

	if (token == NULL)
	{
		fprintf(stderr, "No token for integer at line %d\n", tknGetLineNo(t));
		return 0;
	}
	if (token->type_ == TT_EOF)
	{
		fprintf(stderr, "Unexpected EOF at %d\n", tknGetLineNo(t));
		return 0;
	}
	if (token->type_ == TT_REAL)
	{
		*target = (float) token->data_.dval_;

	} else if (token->type_ == TT_INTEGER)
	{
		*target = (float) token->data_.ival_;
	} else
	{
		fprintf(stderr, "Expected integer data at line %d\n", tknGetLineNo(t));
		return 0;
	}

	return 1;
}


static int
checkForNL(tokenizer * t)
{
	token *token;

	token = tknGetToken(t);

	if (token == NULL)
	{
		fprintf(stderr, "No token for integer at line %d\n", tknGetLineNo(t));
		return -1;
	}
	if (token->type_ == TT_EOF)
	{
		fprintf(stderr, "Unexpected EOF at %d\n", tknGetLineNo(t));
		return -1;
	}
	if (token->type_ == '\n')
	{
		return 1;
	}
	tknPushToken(t);

	return 0;
}

static int
computeCalculatedParameters(PrmData * result, int trainIndex)
{
	int i, calcIndex;

	for (i = Prm_MAX_PARAM; i < Prm_MAX_CALCULATED_PARAM; i++)
	{
		calcIndex = i - Prm_MAX_PARAM;
		if (!(*sPrmCalculation_[calcIndex])(result, trainIndex))
		{
			return 0;
		}
	}

	return 1;
}

static int
readPrmTrain(tokenizer * t, PrmData * result, int trainIndex)
{
	double param1, param2, param3;
	int firstLoadIndex;
	int i;

	result->train_[trainIndex].isInvalid_ = 0;

	/**
	 * read in the first three parameters to see
	 * if the first two are zeros they need to skip.
	 *
	 * The third parameter is then used to determine
	 * if amplitude is in the correct range
	 */
	if ( ! getFloat(t, &param1))
		goto FAIL;
	if ( ! getFloat(t, &param2))
		goto FAIL;
	if ( ! getFloat(t, &param3))
		goto FAIL;

	if ((result->skipLeadingZerosIfPresent_)
		&& (param1 == 0.0 && param2 == 0.0))
	{
		result->train_[trainIndex].param_[Prm_MicroVPkPk] = param3;
		firstLoadIndex = Prm_MicroDuration;
	} else
	{
		result->train_[trainIndex].param_[Prm_MicroVPkPk] = param1;
		result->train_[trainIndex].param_[Prm_MicroDuration] = param2;
		result->train_[trainIndex].param_[Prm_MicroPhases] = param3;
		firstLoadIndex = Prm_MicroTurns;
	}


	/** read in the remaining params */
	for (i = firstLoadIndex; i < result->numberOfTrainParameters_; i++)
	{
		if ( ! getFloat(t, &result->train_[trainIndex].param_[i]))
			goto FAIL;
	}


#ifdef DISABLE_JITTER_10_2010
	{
		int checkStatus;

		/** now get the remaining parameters */
		if ( ! getFloat(t, &result->train_[trainIndex].NF_Jiggle_))
			goto FAIL;

		checkStatus = checkForNL(t);
		if (checkStatus < 0)
			goto FAIL;

		if (checkStatus == 0)
		{

			if ( ! getFloat(t, &result->train_[trainIndex].bJiggle_))
				goto FAIL;

			if ( ! getFloat(t, &result->train_[trainIndex].jiggle_))
				goto FAIL;

			if ( ! getInt(t, &result->train_[trainIndex].fibreCount_))
				goto FAIL;

			if ( ! getInt(t, &result->train_[trainIndex].nJitterPairs_))
				goto FAIL;

			/** if there is jitter data, allocate room for it . . . */
			if (result->train_[trainIndex].nJitterPairs_ > 0)
			{

				result->train_[trainIndex].jitter_ = (PrmJitterData *)
					ckalloc(sizeof(PrmJitterData) *
							result->train_[trainIndex].nJitterPairs_);
				memset(result->train_[trainIndex].jitter_, 0,
					   sizeof(PrmJitterData) *
					   result->train_[trainIndex].nJitterPairs_);

				for (i = 0; i < result->train_[trainIndex].nJitterPairs_; i++)
				{
					if ( ! getFloat(t,
									&result->train_[trainIndex]
										.jitter_[i].MCD_))
						goto FAIL;
					if ( ! getFloat(t,
									&result->train_[trainIndex]
										.jitter_[i].perCentBlocking_))
						goto FAIL;
				}
			}
			if ( ! getNewline(t))
				goto FAIL;
		}
	}
#else
	skipToNewline(t);
#endif /* DISABLE_JITTER_10_2010 */


	/**
	 * figure out whether this line is marked invalid,
	 * by examining the parameters on the line.
	 * Any values greater than 20000.0 will indicate
	 * that some part of the line has been marked invalid.
	 *
	 * As the moment, any such value will cause the entire
	 * line to be marked invalid -- this should be enhanced
	 * to allow a different invalid flag to be set depending
	 * on what portion of the line is so marked.
	 */
	for (i = 0; i < result->numberOfTrainParameters_; i++)
	{

		if (result->train_[trainIndex].param_[i] > PRM_VALIDITY_TEST)
		{
			result->train_[trainIndex].param_[i] -= PRM_VALIDITY_OFFSET;
			result->train_[trainIndex].isInvalid_ = 1;
		}
	}


	/**
	 * check if we have a zero IPImean, in which case we
	 * have been marked invalid by DQEMG itself
	 */
	if ((result->train_[trainIndex].isInvalid_ == 0)
			&& (result->train_[trainIndex].param_[Prm_IPImean + 1] == 0))
	{
		result->train_[trainIndex].isInvalid_ = 2;
	}


	if ( ! computeCalculatedParameters(result, trainIndex))
	{
		goto FAIL;
	}


	return 1;

FAIL:
	return 0;
}

static int
loadPrmData(tokenizer * t, PrmData * result)
{
	double param0, param1, param2, param3, param4;
	int checkStatus;
	int i;

	if ( ! getFloat(t, &param0))
		goto FAIL;
	if ( ! getFloat(t, &param1))
		goto FAIL;
	if ( ! getFloat(t, &param2))
		goto FAIL;
	checkStatus = checkForNL(t);
	if (checkStatus < 0)
		goto FAIL;

	if (checkStatus == 0)
	{
		/** no newline, so keep parsing values */
		if ( ! getFloat(t, &param3))
		{
			goto FAIL;
		}
		if ( ! getFloat(t, &param4))
		{
			goto FAIL;
		}
		result->perCentMVC_ = param0;
		result->numberOfTrains_ = (int) param3;
		result->numberOfTrainParameters_ = ((int) param4);
		result->skipLeadingZerosIfPresent_ = 2;

		if ( ! skipToNewline(t))
		{
			goto FAIL;
		}
	} else
	{
		/** the line ended, so use old format */
		result->perCentMVC_ = param0;
		result->numberOfTrains_ = (int) param1;
		result->numberOfTrainParameters_ = (int) param2;
		result->skipLeadingZerosIfPresent_ = 0;
	}



	/** now allocate the main part of the data */
	if (result->numberOfTrains_ > 0)
	{
		result->train_ = (PrmTrainData *)
				ckalloc(sizeof(PrmTrainData) * result->numberOfTrains_);
		memset(result->train_, 0,
		   		sizeof(PrmTrainData) * result->numberOfTrains_);
	} else
	{
		result->train_ = NULL;
	}


	if (result->numberOfTrainParameters_ > 0)
	{
		for (i = 0; i < result->numberOfTrains_; i++)
		{

			result->train_[i].param_ = (double *)
				ckalloc(sizeof(double)
						* result->numberOfTrainParameters_);
			memset(result->train_[i].param_, 0,
				   sizeof(double)
				   * result->numberOfTrainParameters_);

			result->train_[i].calculatedParam_ = (double *)
				ckalloc(sizeof(double)
						* (Prm_MAX_CALCULATED_PARAM - Prm_MAX_PARAM));
			memset(result->train_[i].calculatedParam_, 0,
				   sizeof(double)
				   * (Prm_MAX_CALCULATED_PARAM - Prm_MAX_PARAM));
		}
	}


	for (i = 0; i < result->numberOfTrains_; i++)
	{
		if ( ! readPrmTrain(t, result, i))
		{
			goto FAIL;
		}
	}

	if ( ! calcParamStats(result))
	{
		goto FAIL;
	}

	return 1;

FAIL:
	return 0;
}



/** create a blank PRM object */
PrmData *
createPrmData()
{
	PrmData *result;

	result = (PrmData *) ckalloc(sizeof(PrmData));

	memset(result, 0, sizeof(PrmData));

	return result;
}

/** destroy one of these objects */
void
deletePrmData(PrmData * data)
{
	int i;

	if (data != NULL)
	{
		if (data->train_ != NULL)
		{
			for (i = 0; i < data->numberOfTrains_; i++)
			{
				if (data->train_[i].nJitterPairs_ > 0)
					ckfree(data->train_[i].jitter_);
			}
			ckfree(data->train_);
		}
		ckfree(data);
	}
}

void
dumpPrm(FILE * fp, int indent, PrmData * data)
{
	int i, j;

	slnprintf(sIndentBuffer, MAX_INDENT, "%*s", indent, "");

	fprintf(fp, "%sPRM File:\n", sIndentBuffer);
	fprintf(fp, "%s%s %% MVC, %d params, %d trains\n",
			sIndentBuffer, niceDouble(data->perCentMVC_),
			data->numberOfTrainParameters_, data->numberOfTrains_);
	fprintf(fp, "\n");

	for (i = 0; i < data->numberOfTrains_; i++)
	{
		fprintf(fp, "%sTrain %d\n", sIndentBuffer, i);
		fprintf(fp, "%sParams:\n", sIndentBuffer);
		fprintf(fp, "%s", sIndentBuffer);
		for (j = 0; j < data->numberOfTrainParameters_; j++)
		{
			fprintf(fp, "%s ", niceDouble(data->train_[i].param_[j]));
		}
		fprintf(fp, "\n");
		fprintf(fp, "%sFibre Count : %d\n",
				sIndentBuffer,
				data->train_[i].fibreCount_);
		fprintf(fp, "%sNF_Jiggle %s  BJiggle %s  Jiggle %s\n",
				sIndentBuffer,
				niceDouble(data->train_[i].NF_Jiggle_),
				niceDouble(data->train_[i].bJiggle_),
				niceDouble(data->train_[i].jiggle_));
		fprintf(fp, "%s%d Jitter Pairs (MCD, block):\n", sIndentBuffer,
				data->train_[i].nJitterPairs_);
		for (j = 0; j < data->train_[i].nJitterPairs_; j++)
		{
			fprintf(fp, "%s%3d : %3s %3s\n",
					sIndentBuffer, j,
					niceDouble(data->train_[i].jitter_[j].MCD_),
					niceDouble(
							   data->train_[i].jitter_[j].perCentBlocking_
							   ));
		}
		fprintf(fp, "\n");
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

}

/**
 ** Write out a PRM file
 **/
int
writePrmFile(const char *outputFile, PrmData * data)
{
	FP *ofp;

	if ((ofp = openFP(outputFile, "wb")) == NULL)
		return 0;

	closeFP(ofp);
	return 1;
}

/**
 ** Read in a PRM file
 **/
PrmData *
readPrmFile(const char *inputFile)
{
	PrmData *result = NULL;
	tokenizer *t;
	FP *ifp;


	if ((ifp = openFP(inputFile, "rb")) == NULL)
		return NULL;

	t = tknGetTokenizer(ifp->fp);
	tknSetOptions(t, (TTOPT_RETURN_CR | TTOPT_RETURN_COMMENTS));

	result = createPrmData();

	if ( ! loadPrmData(t, result))
		goto FAIL;

	tknDeleteTokenizer(t);
	closeFP(ifp);

	return result;


FAIL:
	deletePrmData(result);
	return NULL;
}

int
prmIsTrainValid(PrmData * prm, int trainIndex)
{
	MSG_ASSERT(prm != NULL, "Null pointer passed to prmIsTrainValid()");
	MSG_ASSERT(trainIndex < prm->numberOfTrains_,
			   "Index out of range for file in prmIsTrainValid()");
	return prm->train_[trainIndex].isInvalid_ == 0 ? 1 : 0;
}

int
prmIsTrainUserInvalid(PrmData * prm, int trainIndex)
{
	MSG_ASSERT(prm != NULL, "Null pointer passed to prmIsTrainValid()");
	MSG_ASSERT(trainIndex < prm->numberOfTrains_,
			   "Index out of range for file in prmIsTrainValid()");
	return prm->train_[trainIndex].isInvalid_ == 1 ? 1 : 0;
}

int
prmIsTrainDQEMGInvalid(PrmData * prm, int trainIndex)
{
	MSG_ASSERT(prm != NULL, "Null pointer passed to prmIsTrainValid()");
	MSG_ASSERT(trainIndex < prm->numberOfTrains_,
			   "Index out of range for file in prmIsTrainValid()");
	return prm->train_[trainIndex].isInvalid_ == 2 ? 1 : 0;
}

const char *
prmGetParameterName(PrmParamTypeIndex paramIndex)
{
	if (paramIndex < 0 || paramIndex >= Prm_MAX_PARAM)
		return "<bad index>";

	return sParamName[paramIndex];
}

const char *
prmGetCalculatedParameterName(PrmParamTypeIndex paramIndex)
{
	if ((paramIndex < 0) ||
		(paramIndex >= (Prm_MAX_CALCULATED_PARAM - Prm_MAX_PARAM)))
		return "<bad index>";

	return sCalculatedParamName[paramIndex];
}

double
prmGetParameterValue(
		PrmData * prm,
		int trainIndex,
		PrmParamTypeIndex paramIndex
	)
{
	MSG_ASSERT(prm != NULL, "Null pointer passed to prmGetParamter()");

	MSG_ASSERT(trainIndex < prm->numberOfTrains_,
			   "Index out of range for file in prmGetParamter()");

	MSG_ASSERT(paramIndex < Prm_MAX_PARAM,
			   "Index past mad possible of range in prmGetParamter()");

	MSG_ASSERT(paramIndex < prm->numberOfTrainParameters_,
			   "Index out of range for file in prmGetParamter()");

	return prm->train_[trainIndex].param_[paramIndex];
}

double
prmGetParameterValue(PrmData * prm, int trainIndex, const char *label)
{
	int i;

	MSG_ASSERT(prm != NULL, "Null pointer passed to prmGetParamter()");

	MSG_ASSERT(trainIndex < prm->numberOfTrains_,
			   "Index out of range for file in prmGetParamter()");

	/** first look using the parameter names */
	for (i = 0; i < Prm_MAX_PARAM; i++)
	{
		if (strcmp(label, sParamName[i]) == 0)
		{
			return prm->train_[trainIndex].param_[i];
		}
	}

	/** next, look using the description names */
	for (i = 0; i < Prm_MAX_PARAM; i++)
	{
		if (strcmp(label, sParamDescriptions_[i].name_) == 0)
		{
			return prm->train_[trainIndex].param_[i];
		}
	}

	/** if that also fails, look for a calculated value */
	for (i = 0; i + Prm_MAX_PARAM < Prm_MAX_CALCULATED_PARAM; i++)
	{
		if (strcmp(label, sCalculatedParamDescriptions_[i].name_) == 0)
		{
			return prm->train_[trainIndex].calculatedParam_[i];
		}
	}

	/** return -ve infinity */
	return log(0.0);
}

int
prmPrintSummary(FILE * fp, PrmData * data, int indent)
{
	char printMean[128];
	char printVar[128];
	const char *lastGroup = NULL;
	const char *groupName;
	const int DATA_LEN = 8;
	int i, j;

	if (sMaxParamNameLen_ == 0)
	{
		for (i = 0; i < data->numberOfTrainParameters_; i++)
		{
			if (sMaxParamNameLen_ < strlen(sParamDescriptions_[i].name_))
			{
				sMaxParamNameLen_ = strlen(sParamDescriptions_[i].name_);
			}
		}
		for (i = 0; i < data->numberOfTrainParameters_; i++)
		{
			if (sMaxUnitLen_ < strlen(sParamDescriptions_[i].units_))
			{
				sMaxUnitLen_ = strlen(sParamDescriptions_[i].units_);
			}
		}
		for (i = 0; sParamGroupNames_[i] != NULL; i++)
		{
			if (sMaxGroupNameLen_ < strlen(sParamGroupNames_[i]))
			{
				sMaxGroupNameLen_ = strlen(sParamGroupNames_[i]);
			}
		}
	}
	fprintf(fp, "%*s%-*s %-*s : %*s %-*s  %*s\n",
			indent, "",
			(int) sMaxGroupNameLen_, "Group",
			(int) sMaxParamNameLen_, "Parameter",
			DATA_LEN, "Mean",
			(int) sMaxUnitLen_, "",
			DATA_LEN, "Std. Dev.");

	for (i = 0; i < data->numberOfTrainParameters_; i++)
	{

		if ( ! sParamDescriptions_[i].includeInSummaryOutput_)
			continue;

		groupName = sParamGroupNames_[sParamDescriptions_[i].group_];

		if (lastGroup == groupName)
		{
			groupName = "";
		} else
		{
			lastGroup = groupName;
			fprintf(fp, "\n");
		}

		if (data->baseParameterSummary_[i].valid_ > 0)
		{
			slnprintf(printMean, 128, "%*.2f", DATA_LEN,
					  data->baseParameterSummary_[i].mean_);
			slnprintf(printVar, 128, "%*.2f", DATA_LEN,
					  sqrt(data->baseParameterSummary_[i].variance_));
			trimFloatBuffer(printMean);
			trimFloatBuffer(printVar);
		} else
		{
			strlcpy(printMean, "     --", 128);
			strlcpy(printVar,  "     --", 128);
		}

		fprintf(fp, "%*s%-*s %-*s : %-*s %-*s  %-*s   %d\n",
				indent, "",
				(int) sMaxGroupNameLen_, groupName,
				(int) sMaxParamNameLen_, sParamDescriptions_[i].name_,
				DATA_LEN, printMean,
				(int) sMaxUnitLen_, sParamDescriptions_[i].units_,
				DATA_LEN, printVar,
				data->baseParameterSummary_[i].valid_);
	}


	/** now the calculated parameters */
	lastGroup = NULL;
	for (i = Prm_MAX_PARAM; i < Prm_MAX_CALCULATED_PARAM; i++)
	{

		j = i - Prm_MAX_PARAM;

		if ( ! sCalculatedParamDescriptions_[j].includeInSummaryOutput_)
			continue;

		groupName = sCalculatedParamGroupNames_[
				sCalculatedParamDescriptions_[j].group_
			];

		if (lastGroup == groupName)
		{
			groupName = "";
		} else
		{
			lastGroup = groupName;
			fprintf(fp, "\n");
		}

		if (data->calcParameterSummary_[j].valid_ > 0)
		{
			slnprintf(printMean, 128, "%*.2f", DATA_LEN,
					  data->calcParameterSummary_[j].mean_);
			slnprintf(printVar, 128, "%*.2f", DATA_LEN,
					  sqrt(data->calcParameterSummary_[j].variance_));
			trimFloatBuffer(printMean);
			trimFloatBuffer(printVar);
		} else
		{
			strlcpy(printMean, "     --", 128);
			strlcpy(printVar,  "     --", 128);
		}

		fprintf(fp, "%*s%-*s %-*s : %-*s %-*s  %-*s   %d\n",
				indent, "",
				(int) sMaxGroupNameLen_, groupName,
				(int) sMaxParamNameLen_,
				sCalculatedParamDescriptions_[j].name_,
				DATA_LEN, printMean,
				(int) sMaxUnitLen_,
				sCalculatedParamDescriptions_[j].units_,
				DATA_LEN, printVar,
				data->calcParameterSummary_[j].valid_);
	}

	return !ferror(fp);
}

int
prmPrintSummary(FILE * ofp, const char *filename, int indent)
{
	PrmData *data;
	int status = 0;

	data = readPrmFile(filename);
	if (data == NULL)
	{
		fprintf(stderr, "Cannot read input file '%s'\n",
				filename);
		return 0;
	}
	prmPrintSummary(ofp, data, indent);

	status = 1;

	deletePrmData(data);
	return status;
}

