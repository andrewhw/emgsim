/**
 ** Define structures used throughout the simulator
 **
 ** $Id: dco.h 56 2019-06-12 12:16:36Z andrew $
 **/

#ifndef __MAKE_DCO_HEADER__
#define __MAKE_DCO_HEADER__

#include "os_types.h"

#define	DCO_EMG_NAME_LEN				60
#define	DCO_TRAIN_REL_MAGIC_WORD		0xfded

#define	DCO_TRAIN_REL_TYPE_NONE					0
#define	DCO_TRAIN_REL_TYPE_LINKED				1
#define	DCO_TRAIN_REL_TYPE_DISPARATELY_DETECTED	2
#define	DCO_TRAIN_REL_TYPE_SYNCHRONIZED			3
#define	DCO_TRAIN_REL_TYPE_EXCLUSIVE			4
#define	DCO_TRAIN_REL_TYPE_MERGED				5


/**
 ** Definitions for a DCO file construct
 **/
typedef struct dcoMUP
{
    float	mupFiringTime_;		/** firing time in seconds */
    osInt32	mupBufferOffset_;		/** file offset in bytes */
    osInt16	mupMotorUnitNumber_;	/** assigned motor unit (class) */
    osInt16	mupNumber_;			/** MUP number within train */
    float	mupDistCertainty_;     /** classifier distance certainty */
    osInt32	userMUPId_;				/** users can store values here, but
                                    they are not persisted */
} dcoMUP;


/** used for tracking dependencies */
typedef struct TrainRelation
{
    osInt16	relatedTrainId_;
    osInt32	type_;
    float	latency_;
} TrainRelation;


/** Info on a particular train */
typedef struct TrainInfo
{
    osInt16	trainId_;
    osInt32	userMUPId_;
    osInt32	nMUPFiringsInTrain_;

	int isSuppressed_;

    osInt32	nRelations_;
    osInt32	nRelationBlocks_;
    TrainRelation *relation_;
} TrainInfo;


/**
 ** Definitions for a DCO file construct
 **/
typedef struct dcoData
{
    char	emgName_[DCO_EMG_NAME_LEN];
    osInt16	numberOfMUPs_;
    osInt16	numberOfMUPsAllocated_;

                /** used in the list management */
    osInt32	nTrains_;
    osInt32	nTrainBlocks_;
    TrainInfo *train_;

    dcoMUP	**MUP_;
} dcoData;


enum DcoLoadFlags {
		DCO_ADD_TRAILING_MUPS = 0x01
	};



extern dcoData *createDcoData(const char *name);
extern void deleteDcoData(dcoData *data);

extern dcoMUP *createMUP(
		float firingTime,
		osInt32 bufferOffset,
		osInt32 motorUnit,
		osInt32 MUPNumber,
		float certainty,
		osInt32 userMUPId = (-1)
            );
extern void deleteMUP(dcoMUP *data);

extern int addMUP(dcoData *dcoData, dcoMUP *MUP);

extern void dumpDco(
		FILE *dumpfp,
		int indent,
		dcoData *data
            );
extern void dumpDcoVerbose(
		FILE *dumpfp,
		int indent,
		dcoData *data
            );
extern dcoData *readDcoFile(
		const char *inputFile,
		int flags = DCO_ADD_TRAILING_MUPS
            );
extern int writeDcoFile(
		const char *outputFile,
		dcoData *data
            );
extern void sortDcoData(
		dcoData *data,
		int useTrainIdForMotorUnitId,
		char *filename = NULL
            );

#endif /* __MAKE_DCO_HEADER__ */


