/**
 ** Define the PRM file when loaded into memory
 **
 ** $Id: prm.h 51 2017-05-10 18:28:18Z stashuk $
 **/

#ifndef __PRM_DEFINITION_HEADER__
#define __PRM_DEFINITION_HEADER__

#include "os_types.h"

#define	PRM_VALIDITY_OFFSET		25000.0
#define	PRM_VALIDITY_TEST		(PRM_VALIDITY_OFFSET - 5000.0)

enum PrmParamTypeIndex {
	Prm_MicroVPkPk = 0,
	Prm_MicroDuration,
	Prm_MicroPhases,
	Prm_MicroTurns,
	Prm_MicroAAR,
	Prm_MacroVPkPk,
	Prm_MacroArea,
	Prm_MacroNegPkAmp,
	Prm_MacroNegPkArea,
	Prm_MacroNegPkDur,
	Prm_IPImean,
	Prm_IPIsd,
	Prm_IPIcov,
	Prm_FRmean,
	Prm_FRmcd,
	Prm_NF_Jiggle,
	Prm_BJiggle,
	Prm_Jiggle,
	Prm_CNJitter,
	Prm_Blocking,
	Prm_FibreCount,
	Prm_IDRate,
	Prm_NumMups,
	Prm_MMuV,	
	Prm_PeakToPeakAmpMUNE,		
	Prm_NegPeakAmpMUNE,
	Prm_NegPeakAreaMUNE,
	Prm_MAX_PARAM
    };

enum PrmGroupTypeIndex {
	Prm_Contraction = 0,
	Prm_Micro,
	Prm_Macro,
	Prm_IPI,
	Prm_FR,
	Prm_Ensemble,
	Prm_Misc,
	Prm_MAX_GROUP
    };

enum PrmParamExtendedTypeIndex {
	Prm_SizeIndex = Prm_MAX_PARAM,
	Prm_LogAmplitude,
	Prm_MAX_CALCULATED_PARAM
    };

typedef struct PrmParameterDescription {
    PrmGroupTypeIndex group_;
    int type_;
    int includeInSummaryOutput_;
    const char *name_;
    const char *units_;
} PrmParameterDescription;

extern PrmParameterDescription sParamDescriptions_[];
extern const char *sParamGroupNames_[];

extern PrmParameterDescription sCalculatedParamDescriptions_[];
extern const char *sCalculatedParamGroupNames_[];

typedef struct PrmJitterData {
    double MCD_;
    double perCentBlocking_;
} PrmJitterData;

typedef struct PrmTrainData {

    double NF_Jiggle_;
    double bJiggle_;
    double jiggle_;

    osInt32 fibreCount_;
    osInt32 nJitterPairs_;

    PrmJitterData *jitter_;

    int isInvalid_;

    double *param_;
    double *calculatedParam_;

} PrmTrainData;

typedef struct PrmSummaryData {
    double mean_;
    double variance_;
    int valid_;
} PrmSummaryData;

typedef struct PrmData {
    int numberOfTrains_;
    double perCentMVC_;
    int numberOfTrainParameters_;
    int skipLeadingZerosIfPresent_;
    PrmSummaryData *baseParameterSummary_;
    PrmSummaryData *calcParameterSummary_;

    PrmTrainData *train_;
    
} PrmData;

extern PrmData *createPrmData();
extern void deletePrmData(PrmData *data);

extern void dumpPrm(
		FILE *dumpfp,
		int indent,
		PrmData *data
	    );
extern void dumpPrmVerbose(
		FILE *dumpfp,
		int indent,
		PrmData *data
	    );

extern PrmData *readPrmFile(const char *inputFile);
extern int writePrmFile(const char *outputFile, PrmData *data);

extern int prmIsTrainValid(
		PrmData *prm,
		int trainIndex
	    );

extern int prmIsTrainUserInvalid(
		PrmData *prm,
		int trainIndex
	    );

extern int prmIsTrainDQEMGInvalid(
		PrmData *prm,
		int trainIndex
	    );

extern double prmGetParameterValue(
		PrmData *prm,
		int trainIndex,
		PrmParamTypeIndex paramIndex
	    );

extern double prmGetParameterValue(
		PrmData *prm,
		int trainIndex,
		const char *label
	    );

extern const char *prmGetParameterName(PrmParamTypeIndex paramIndex);

extern int prmPrintSummary(
		FILE *fp,
		PrmData *data,
		int indent = 0
	    );
extern int prmPrintSummary(
		FILE *fp,
		const char *filename,
		int indent = 0
	    );

#endif /* __PRM_DEFINITION_HEADER__ */

