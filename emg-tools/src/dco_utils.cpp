/**
 **	Handle the reading and writing of data structures to hold
 **	DCO data.
 **
 ** $Id: dco_utils.cpp 57 2020-09-25 19:59:54Z andrew $
 **/

#include "os_defs.h"

#ifndef	MAKEDEPEND
#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifndef	OS_WINDOWS_NT
#include	<unistd.h>
#else
#include	<io.h>
#endif
#include	<fcntl.h>
#include	<stdlib.h>
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

#include "dco.h"

#include "massert.h"
#include "msgisort.h"
#include "listalloc.h"
#include "stringtools.h"
#include "error.h"
#include "pathtools.h"
#include "tclCkalloc.h"
#include "reporttimer.h"
#include "io_utils.h"

//#define DCO_PREVENT_RECURSIVE_LINKING

#define BLOCK_SIZE	  24

#define MAX_INDENT	  16
static char indentBuffer[MAX_INDENT];


static void initializeTrain_(TrainInfo *newTrain);



/** create a blank DCO object */
dcoData *
createDcoData(const char *name)
{
	dcoData *dco;

	if (strlen(name) >= DCO_EMG_NAME_LEN)
	{
		Error("DCO Object name too long '%s' - max %d chars\n",
			  name, DCO_EMG_NAME_LEN);
		return NULL;
	}
	dco = (dcoData *) ckalloc(sizeof(dcoData));

	memset(dco, 0, sizeof(dcoData));
	strlcpy(dco->emgName_, name, DCO_EMG_NAME_LEN);
	dco->nTrains_ = 0;
	dco->numberOfMUPs_ = 0;
	dco->numberOfMUPsAllocated_ = 0;

	return dco;
}

/** destroy one of these objects */
void 
deleteDcoData(dcoData * data)
{
	int i;

	if (data == NULL)
		return;

	if (data->numberOfMUPs_ > 0)
	{
		for (i = 0; i < data->numberOfMUPs_; i++)
		{
			deleteMUP(data->MUP_[i]);
		}
		ckfree(data->MUP_);
	}
	if (data->train_ != NULL)
	{
		for (i = 0; i < data->nTrains_; i++)
		{
			if (data->train_[i].relation_ != NULL)
				ckfree(data->train_[i].relation_);
		}
		ckfree(data->train_);
	}
	ckfree(data);
}

static void
initializeTrain_(TrainInfo *newTrain)
{
	/**
	 ** initialize this data to zero in case it is inserted
	 ** into the list -- otherwise we will just accumulate
	 ** the one which is found from previous efforts
	 **/
	newTrain->userMUPId_ = (-1);
	newTrain->trainId_ = (-1);
	newTrain->nMUPFiringsInTrain_ = 0;

	newTrain->isSuppressed_ = 0;

	newTrain->nRelations_ = 0;
	newTrain->nRelationBlocks_ = 0;
	newTrain->relation_ = NULL;
}

dcoMUP *
createMUP(
		float	firingTime,
		osInt32 bufferOffset,
		osInt32 motorUnitNumber,
		osInt32 mupNumber,
		float	mupDistCertainty,
		osInt32 userMUPId
	)
{
	dcoMUP *mup;

	mup = (dcoMUP *) ckalloc(sizeof(dcoMUP));

	mup->mupFiringTime_ = firingTime;
	mup->mupBufferOffset_ = bufferOffset;
	mup->mupMotorUnitNumber_ = (short) motorUnitNumber;
	mup->mupNumber_ = (short) mupNumber;
	mup->mupDistCertainty_ = mupDistCertainty;
	mup->userMUPId_ = userMUPId;

	return mup;
}

void 
deleteMUP(dcoMUP * data)
{
	if (data == NULL)
		return;
	ckfree(data);
}

static void
growMUPList(dcoData * dcoData)
{
	dcoMUP **oldlist;
	int i, newNumEntries;

	/** append the MUP_ onto the list of old MUPs */
	oldlist = dcoData->MUP_;
	dcoData->numberOfMUPsAllocated_ =
		newNumEntries = dcoData->numberOfMUPs_ + BLOCK_SIZE;

	dcoData->MUP_ = (dcoMUP **)
		ckalloc(sizeof(dcoMUP *) * newNumEntries);
	if (dcoData->numberOfMUPs_ > 0)
	{
		for (i = 0; i < dcoData->numberOfMUPs_; i++)
		{
			dcoData->MUP_[i] = oldlist[i];
		}
		ckfree((void *) oldlist);
	}
	/** NULL the rest of the list */
	for (i = dcoData->numberOfMUPs_; i < dcoData->numberOfMUPsAllocated_; i++)
	{
		dcoData->MUP_[i] = NULL;
	}
}


int 
sTrainCompareFunction(void *listData, void *newData)
{
	TrainInfo *localListData;
	TrainInfo *localNewData;

	localNewData = (TrainInfo *) newData;
	localListData = (TrainInfo *) listData;

	return -(localNewData->trainId_ - localListData->trainId_);
}

int 
sAddTrainInsertFunc(void *targ, void *source, int ctrl)
{
	TrainInfo *localListData;
	TrainInfo *localNewData;

	localNewData = (TrainInfo *) source;
	localListData = (TrainInfo *) targ;


	switch (ctrl)
	{
	case ISORT_INSERT:
		memcpy(localListData, localNewData, sizeof(TrainInfo));
		break;

		/**
		 ** for overwrite, return failure so we know we
		 ** are not inserting in the calling routine
		 **/
	case ISORT_OVERWRITE:
		return 0;
		OS_BREAK;

	default:
		Derror(__FILE__, __LINE__, "Internal Error");
		return 0;
		OS_BREAK;
	}

	return (1);
}

/**
 **	Add a MUP to the list, growing if necessary
 **/
int 
addMUP(dcoData * dcoData, dcoMUP * MUP)
{
	int resultIndex;
	TrainInfo newTrain;
	static int overflowWarning = 0;

	if (dcoData->numberOfMUPs_ == SHRT_MAX)
	{
		if (overflowWarning == 0)
		{
			overflowWarning = 1;
			LogError("Overflow in number of MUPs (max %d) in DCO entry\n",
					 SHRT_MAX);
		}
		return 0;
	}
	/** append the MUP onto the list of old MUPs */
	if (dcoData->numberOfMUPs_ + 1 >= dcoData->numberOfMUPsAllocated_)
	{
		growMUPList(dcoData);
	}
	dcoData->MUP_[dcoData->numberOfMUPs_++] = MUP;

	/**
	 ** initialize this data to zero in case it is inserted
	 ** into the list -- otherwise we will just accumulate
	 ** the one which is found from previous efforts
	 **/
	initializeTrain_(&newTrain);
	newTrain.userMUPId_ = MUP->userMUPId_;
	newTrain.trainId_ = MUP->mupMotorUnitNumber_;

	resultIndex = listInsertSort(
							(void **) &dcoData->train_,
							(void *) &newTrain,
							&dcoData->nTrains_,
							&dcoData->nTrainBlocks_, 8,
							sizeof(TrainInfo),
							sTrainCompareFunction,
							sAddTrainInsertFunc);

	if (resultIndex < 0)
	{
		return 0;
	}
	/** now add one to the train count */
	dcoData->train_[resultIndex].nMUPFiringsInTrain_++;

	return 1;
}

static int
rMUP(FP * fp, dcoMUP * value)
{

	MSG_ASSERT((sizeof(dcoMUP) ==
				(2 * sizeof(float))
				+ (2 * sizeof(osInt16))
				+ (2 * sizeof(osInt32))),
			   "Size of MUP structure has changed");

	MSG_ASSERT((sizeof(dcoMUP) ==
				(2 * 4)
				+ (2 * 2)
				+ (2 * 4)),
			   "Size of MUP structure has changed");

	/** default value for userMUPId_ */
	value->userMUPId_ = (-1);

	if (!rFloat(fp, &value->mupFiringTime_))
		goto FAIL;

	if (!r4byteInt(fp, &value->mupBufferOffset_))
		goto FAIL;

	if (!r2byteInt(fp, &value->mupMotorUnitNumber_))
		goto FAIL;

	if (!r2byteInt(fp, &value->mupNumber_))
		goto FAIL;

	if (!rFloat(fp, &value->mupDistCertainty_))
		goto FAIL;

	return 1;

FAIL:
	Error("Failure reading MUP from file '%s' : %s\n",
		  fp->name, strerror(errno));
	return 0;
}


static int
wMUP(FP * fp, dcoMUP * value)
{
	MSG_ASSERT((sizeof(dcoMUP) ==
			(2 * sizeof(float)) + (2 * sizeof(osInt16)) + (2 * sizeof(osInt32))),
			   "Size of MUP structure has changed");

	if (!wFloat(fp, value->mupFiringTime_))
		goto FAIL;
	if (!w4byteInt(fp, value->mupBufferOffset_))
		goto FAIL;
	if (!w2byteInt(fp, value->mupMotorUnitNumber_))
		goto FAIL;
	if (!w2byteInt(fp, value->mupNumber_))
		goto FAIL;
	if (!wFloat(fp, value->mupDistCertainty_))
		goto FAIL;

	return 1;

FAIL:
	Error("Failure writing MUP into file '%s' : %s\n",
		  fp->name, strerror(errno));
	return 0;
}

const char *
getTrainRelationString(int type)
{
	switch (type)
	{
	case DCO_TRAIN_REL_TYPE_NONE: return "none";
	case DCO_TRAIN_REL_TYPE_LINKED: return "linked";
	case DCO_TRAIN_REL_TYPE_DISPARATELY_DETECTED: return "disparately detected";
	case DCO_TRAIN_REL_TYPE_SYNCHRONIZED: return "synchronized";
	case DCO_TRAIN_REL_TYPE_EXCLUSIVE: return "exclusive";
	case DCO_TRAIN_REL_TYPE_MERGED: return "merged";
	}

	return "UNKNOWN";
}

void
dumpDco(FILE * fp, int indent, dcoData * data)
{
	const char *delim;
	int i, j;

	slnprintf(indentBuffer, MAX_INDENT, "%*s", indent, "");

	fprintf(fp, "%sDCO '%s'\n", indentBuffer, data->emgName_);
	fprintf(fp, "%s  nMUPs  : %3d\n", indentBuffer, data->numberOfMUPs_);
	fprintf(fp, "%s  nTrains: %3d\n", indentBuffer, data->nTrains_);
	for (i = 0; i < data->nTrains_; i++)
	{
		fprintf(fp, "%s    %2d", indentBuffer, i);
		fprintf(fp, " Train-%02d",
					data->train_[i].trainId_);
		fprintf(fp, ", %3ld firings",
				(long) data->train_[i].nMUPFiringsInTrain_);

		delim = "";
		fprintf(fp, ";  Relns: [");
		for (j = 0; j < data->train_[i].nRelations_; j++)
		{
			fprintf(fp, "%sT%d(%s,%g)", delim,
					data->train_[i].relation_[j].relatedTrainId_,
					getTrainRelationString(
						data->train_[i].relation_[j].type_),
					data->train_[i].relation_[j].latency_);
			delim = ", ";
		}
		fprintf(fp, "]");

		if (data->train_[i].isSuppressed_)
			fprintf(fp, " (SUPPRESSED)");

		fprintf(fp, "\n");
	}
}

void
dumpDcoVerbose(FILE * fp, int indent, dcoData * data)
{
	int i;
	dumpDco(fp, indent, data);

	slnprintf(indentBuffer, MAX_INDENT, "%*s", indent, "");

	for (i = 0; i < data->numberOfMUPs_; i++)
	{
		fprintf(fp, "%s  MUP %d\n", indentBuffer, i);

		fprintf(fp, "%s      firing Time  %f (seconds)\n", indentBuffer,
				data->MUP_[i]->mupFiringTime_);
		fprintf(fp, "%s      bufferOffset %ld\n", indentBuffer,
				(long) data->MUP_[i]->mupFiringTime_);
		fprintf(fp, "%s      motorUnit    %d\n", indentBuffer,
				(int) data->MUP_[i]->mupMotorUnitNumber_);
		fprintf(fp, "%s      MUPNumber   %d\n", indentBuffer,
				(int) data->MUP_[i]->mupNumber_);
		fprintf(fp, "%s      certainty    %f\n", indentBuffer,
				data->MUP_[i]->mupDistCertainty_);
		if (data->MUP_[i]->userMUPId_ >= 0)
		{
			fprintf(fp, "%s      userMUPId   %ld\n", indentBuffer,
					(long) data->MUP_[i]->userMUPId_);
		}
	}
}



/**
 ** Local comparator used in
 **	 int sortdcoData(dcoData *data)
 ** below.
 **/
static int 
MUPComparator(const void *MUP_A, const void *MUP_B)
{
	float time_A, time_B, diff;

	time_A = (*((dcoMUP **) MUP_A))->mupFiringTime_;
	time_B = (*((dcoMUP **) MUP_B))->mupFiringTime_;

	/**
	 **	can't just return result of subtraction because of
	 **	roundoff problems
	 **/
	diff = (time_A - time_B);
	if (diff == 0.0)
		return 0;
	if (diff > 0)
		return 1;
	return (-1);
}

/**
 ** Sort the MUPs by time and stamp them with ids in
 ** ascending order
 **/
void
sortDcoData(dcoData * data, int useTrainIdForMotorUnitId, char *mappingTableFilename)
{
	int i;
	int *trainMapping;
	int maxMotorUnitNumber;
	FILE *mappingTableFP;

	/*
	printf("		Sorting %d MUPs\n", data->numberOfMUPs);
	*/
	qsort(data->MUP_, data->numberOfMUPs_,
		  sizeof(dcoMUP *), MUPComparator);

	/*
	 * if we are to do the mapping, allocate a table
	 * for quick lookups of what we should have as the mapping
	 */
	if (useTrainIdForMotorUnitId)
	{
		maxMotorUnitNumber = 0;
		for (i = 0; i < data->nTrains_; i++)
		{
			if (maxMotorUnitNumber < data->train_[i].trainId_)
				maxMotorUnitNumber = data->train_[i].trainId_;
		}

		trainMapping = (int *)
			ckalloc((maxMotorUnitNumber + 1) * sizeof(int));

		/* plug the table with (-1)'s so we see errors */
		for (i = 0; i <= maxMotorUnitNumber; i++)
		{
			trainMapping[i] = (-1);
		}

		/* build up the mappings on top of the (-1)'s */
		for (i = 0; i < data->nTrains_; i++)
		{
			trainMapping[data->train_[i].trainId_] = i;
		}

		if (mappingTableFilename != NULL)
		{
			mappingTableFP = fopenpath(mappingTableFilename, "w");

			for (i = 0; i <= maxMotorUnitNumber; i++)
			{
				if (trainMapping[ i ] != -1)
					fprintf(mappingTableFP, "%3d -> %d\n", i, trainMapping[ i ]);
			}
			fclose(mappingTableFP);
		}

	}
	for (i = 0; i < data->numberOfMUPs_; i++)
	{
		data->MUP_[i]->mupNumber_ = i;

		if (useTrainIdForMotorUnitId)
		{
			MSG_ASSERT(trainMapping[data->MUP_[i]->mupMotorUnitNumber_] >= 0,
					   "Internal error in train mapping table\n");

			data->MUP_[i]->mupMotorUnitNumber_ = trainMapping[
												   data->MUP_[i]->mupMotorUnitNumber_
				];
		}
	}

	if (useTrainIdForMotorUnitId)
	{
		ckfree(trainMapping);
	}
}

/**
 **	Add in a dependency relation
 **/
static int
addDcoTrainDependency_(
		dcoData *dco,
		int relLeadTrainId,
		int relSubTrainId,
		float relLatencyShift,
		osInt32 relType
	)
{
	TrainRelation *newRelation;
	int i, trainIndex;

	trainIndex = (-1);
	for (i = 0; i < dco->nTrains_; i++)
	{
		if (dco->train_[i].trainId_ == relLeadTrainId)
		{
			trainIndex = i;
			break;
		}
	}
	
	if (trainIndex < 0)	return -1;


	/** add the relation to this train */
	if (listCheckSize(dco->train_[trainIndex].nRelations_ + 1,
			(void **) &dco->train_[trainIndex].relation_,
			&dco->train_[trainIndex].nRelationBlocks_, 4,
			sizeof(TrainRelation)) <= 0)
	{
		return -1;
	}

	newRelation = &dco->train_[trainIndex].relation_[
				dco->train_[trainIndex].nRelations_++
			];
	newRelation->relatedTrainId_ = relSubTrainId;
	newRelation->type_ = relType;
	newRelation->latency_ = relLatencyShift;

	/* suppress the higher numbered parts of linked trains */
	if (relLeadTrainId > relSubTrainId &&
			(relType == DCO_TRAIN_REL_TYPE_LINKED
					|| relType == DCO_TRAIN_REL_TYPE_MERGED))
		dco->train_[trainIndex].isSuppressed_ = 1;

	return 1;
}

/**
 **	Read in a DCO file
 **/
dcoData *
readDcoFile(const char *inputFile, int flags)
{
	dcoData *dco = NULL;
	FP *ifp;
	char nameBuffer[DCO_EMG_NAME_LEN];
	osInt16 refNTrains, refNMUPs;
	osUint16 magicWord;
	osInt32 relType;
	osInt16 relUnit;
	osInt16 nRelations;
	float relLatencyShift;
	dcoMUP loadMUP, *newMUP;
	int nRead;
	int i, j;
	//size_t offset;


	if ((ifp = openFP(inputFile, "rb")) == NULL)
		goto FAIL;

	if (!rGeneric(ifp, nameBuffer, DCO_EMG_NAME_LEN))
	{
		return NULL;
	}
	dco = createDcoData(nameBuffer);

	if (!r2byteInt(ifp, &refNTrains))
		goto FAIL;
	if (!r2byteInt(ifp, &refNMUPs))
		goto FAIL;


	/* number of trains may be (should be) negative */
	if (refNTrains < 0)
		refNTrains *= (-1);
	else
		Warning("File continues after last declared MUP\n");


	/* load up the MUP information */
	for (i = 0; i < refNMUPs; i++)
	{

		if ( ! rMUP(ifp, &loadMUP))
		{
			Error("Failed reading MUP %d\n", i + 1);
			goto FAIL;
		}
		newMUP = createMUP(
						   loadMUP.mupFiringTime_,
						   loadMUP.mupBufferOffset_,
						   loadMUP.mupMotorUnitNumber_,
						   loadMUP.mupNumber_,
						   loadMUP.mupDistCertainty_);
		if (addMUP(dco, newMUP) == 0)
			deleteMUP(newMUP);
	}


	/* sanity checks */
	if (dco->nTrains_ != refNTrains)
	{
		Error("Num Trains found in DCO (%d) != num declared (%d)\n",
			  dco->nTrains_, refNTrains);
	}
	if (dco->numberOfMUPs_ != refNMUPs)
	{
		Error("Num MUPs in DCO (%d) != num in file (%d)\n",
			  dco->numberOfMUPs_, refNMUPs);
	}


	/*
	 * Now look for train dependency relationship information
	 */

	//offset = ftell(ifp->fp);



	/*
	 * read the magic word -- we use fread for this directly, 
	 * as it is not an error if we don't find data here
	 */
	nRead = fread(&magicWord, 1, sizeof(osUint16), ifp->fp);
	if (nRead > 0)
	{
		if (magicWord == DCO_TRAIN_REL_MAGIC_WORD)
		{
			for (i = 0; i < refNTrains; i++)
			{
				if (!r2byteInt(ifp, &nRelations))	goto FAIL;
				for (j = 0; j < nRelations; j++)
				{
					if (!r4byteInt(ifp, &relType))	goto FAIL;
					if (!r2byteInt(ifp, &relUnit))	goto FAIL;
					if (!rFloat(ifp, &relLatencyShift))	goto FAIL;

					if (relUnit >= 0 && relUnit < refNTrains)
					{
						/* train "i" is related to train "relUnit" */
						if (addDcoTrainDependency_(dco, dco->train_[i].trainId_,
								relUnit, relLatencyShift, relType) < 0) goto FAIL;
					}
				}
			}
		}
	}

	closeFP(ifp);

	return dco;

FAIL:
	return NULL;
}


/**
 **	Write out a DCO file
 **/
int 
writeDcoFile(
		const char *outputFile,
		dcoData * data
	)
{
	FP *ofp;
	int i;

	if ((ofp = openFP(outputFile, "wb")) == NULL)
		return 0;

	if (!wGeneric(ofp, data->emgName_, DCO_EMG_NAME_LEN))
	{
		return 0;
	}
	if (!w2byteInt(ofp, (short) data->nTrains_))
		return 0;
	if (!w2byteInt(ofp, data->numberOfMUPs_))
		return 0;

	for (i = 0; i < data->numberOfMUPs_; i++)
	{
		if (!wMUP(ofp, data->MUP_[i]))
			return 0;
	}

	closeFP(ofp);
	return 1;
}


