/*
* Implementation of ASDBlocks classes.
*
* Created by Eric Hunsberger - Jan 09, 2009
*/

#include <cstdlib>
#include "ASDBlocks.h"
#include "tinyxml.h"

#define MAX_NUMBER_OF_JITTERS 20000
#define VARIABLE_TO_STRING(val) #val

void ASDJitterBlock::deleteJitterPairBlocks() 
{
	if (m_pJitterPairBlock) 
	{
		delete [] m_pJitterPairBlock;
		m_pJitterPairBlock = NULL;
	}
}

BOOL ASDJitterBlock::createJitterPairBlocks()
{
	deleteJitterPairBlocks();

	if(m_nJitterPairArraySize <= 0 || m_nJitterPairArraySize > MAX_NUMBER_OF_JITTERS) 
	{
		m_sError.Format("Invalid number of jitter pairs: must be greater than %d and less than %d", 0, MAX_NUMBER_OF_JITTERS);
		return FALSE;
	}

	m_pJitterPairBlock = new ASDJitterPairBlock[m_nJitterPairArraySize];

	return TRUE;
}

void ASDJitterBlock::deleteJitterTurnBlocks() 
{
	if (m_pJitterTurnBlock) 
	{
		delete [] m_pJitterTurnBlock;
		m_pJitterTurnBlock = NULL;
	}
}

BOOL ASDJitterBlock::createJitterTurnBlocks() 
{
	deleteJitterTurnBlocks();

	if(m_nJitterTurnArraySize <= 0 || m_nJitterTurnArraySize > MAX_NUMBER_OF_JITTERS) 
	{
		m_sError.Format("Invalid number of jitter turns: must be greater than %d and less than %d", 0, MAX_NUMBER_OF_JITTERS);
		return FALSE;
	}

	m_pJitterTurnBlock = new ASDJitterTurnBlock[m_nJitterTurnArraySize];

	return TRUE;
}

// Constructors
ASDTrainBlock::ASDTrainBlock(ASDio* asdio) 
{
	m_correctPersistenceID = ID_TRAINANLYSIS;
	m_receivedPersistenceID = 0;
	m_pParentASDIO = asdio;

	numberOfMUPs = 0;
	m_pConsecutiveDistanceIDI = NULL;
	m_pConsecutiveDistance = NULL;

	// Set up blocks
	m_pMicroBlock = new ASDMicroBlock(this);
	m_pMacroBlock = NULL; // will be defined later on if needed
	m_pFiringBlock = new ASDFiringBlock(this);
	m_pEnsembleBlock = new ASDEnsembleBlock(this);
}

ASDMicroBlock::ASDMicroBlock(ASDTrainBlock* pTrainBlock) 
{
	m_correctPersistenceID = ID_MICROTMPL3;
	m_receivedPersistenceID = 0;
	m_pParentTrainBlock = pTrainBlock;

	NF_MUP_TmplLength_ = 0;
	m_pNF_MUP_Tmpl = NULL;
	m_NF_MUP_TmplBaselineRMS = 0;
}

ASDMacroBlock::ASDMacroBlock(ASDTrainBlock* pTrainBlock) 
{
	m_correctPersistenceID = ID_MACROTMPL;
	m_receivedPersistenceID = 0;
	m_pParentTrainBlock = pTrainBlock;
}

ASDTemplateBlock::ASDTemplateBlock() 
{
	m_correctPersistenceID = ID_MUPTMPL3;
	m_receivedPersistenceID = 0;

	templateLength_ = 0;
	pTemplateWaveform_ = NULL;
	templateBaselineRMS_ = 0;
}

ASDFiringBlock::ASDFiringBlock(ASDTrainBlock* pTrainBlock)
{
	m_correctPersistenceID = ID_FIRING_DATA;
	m_receivedPersistenceID = 0;
	m_pParentTrainBlock = pTrainBlock;
	IDI_MACID_ = 0;
	IDI_MPACID_ = 0;
	numberOfIDIs_ = 0;
	pIDI_ = NULL;
}

ASDEnsembleBlock::ASDEnsembleBlock(ASDTrainBlock* pTrainBlock) 
{
	m_correctPersistenceID = ID_ENSEMBLE_DATA;
	m_receivedPersistenceID = 0;
	m_pParentTrainBlock = pTrainBlock;

	m_pJitterBlock = new ASDJitterBlock(this);

	m_pSelectedMupNumber = NULL;
	m_pShift = NULL;

	maxTrnInterval_ = 0;
	NF_MupDispersion_ = 0;
	meanTrnArea_ = 0;

}

ASDJitterBlock::ASDJitterBlock(ASDEnsembleBlock* pEnsembleBlock) 
{
	m_correctPersistenceID = ID_JITTER;
	m_receivedPersistenceID = 0;
	m_pParentEnsembleBlock = pEnsembleBlock;

	fibreCount_ = 0;
	m_nJitterPairArraySize = 0;
	m_nJitterTurnArraySize = 0;
	m_nNumberOfJitterPairs = 0;
	m_nNumberOfTurnsForJitter = 0;

	m_pNF_MupTmplIndexOfMFP = NULL;
//	m_pIsTurnFTCalculated = NULL;
	m_pJitterPairBlock = NULL;
	m_pJitterTurnBlock = NULL;
	m_ppPeakTime = NULL;
}

ASDJitterPairBlock::ASDJitterPairBlock() 
{
	m_correctPersistenceID = ID_JITTERPAIR;
	m_receivedPersistenceID = 0;

	m_pFibreABlockedFlag = NULL;
	m_pFibreBBlockedFlag = NULL;
	m_pFibreAPeakTime = NULL;
	m_pFibreBPeakTime = NULL;
}

ASDJitterTurnBlock::ASDJitterTurnBlock() 
{
	m_correctPersistenceID = ID_TURNDATA;
	m_receivedPersistenceID = 0;
}

// Deconstructors
ASDTrainBlock::~ASDTrainBlock() 
{
	if (m_pConsecutiveDistanceIDI) 
		delete m_pConsecutiveDistanceIDI;
	if (m_pConsecutiveDistance) 
		delete m_pConsecutiveDistance;

	if(m_pMicroBlock) 
		delete m_pMicroBlock;
	if(m_pMacroBlock) 
		delete m_pMacroBlock;
	if(m_pFiringBlock) 
		delete m_pFiringBlock;
	if(m_pEnsembleBlock) 
		delete m_pEnsembleBlock;
}

ASDMicroBlock::~ASDMicroBlock() 
{
	if (m_pNF_MUP_Tmpl) 
		delete m_pNF_MUP_Tmpl;
	m_pNF_MUP_Tmpl = NULL;
}

ASDTemplateBlock::~ASDTemplateBlock()
{
	if(pTemplateWaveform_) 
		delete pTemplateWaveform_;
}

ASDEnsembleBlock::~ASDEnsembleBlock() 
{
	if (m_pSelectedMupNumber) 
		delete [] m_pSelectedMupNumber;
	if (m_pShift) 
		delete [] m_pShift;

	if (m_pJitterBlock) 
		delete m_pJitterBlock;
}

ASDJitterBlock::~ASDJitterBlock() 
{
	//if (m_rasterMeanAmplOfTurn) delete [] m_rasterMeanAmplOfTurn;
	//if (m_rasterSumAmplOfTurn) delete [] m_rasterSumAmplOfTurn;

	if (m_ppPeakTime)
	{
		for (int i = 0; i < m_nJitterTurnArraySize; i++)
		{
			if (m_ppPeakTime[i])
				delete m_ppPeakTime[i];
		}
		delete [] m_ppPeakTime;
    }

	if (m_pJitterPairBlock) 
		delete [] m_pJitterPairBlock;
	

	if (m_pJitterTurnBlock) 
		delete [] m_pJitterTurnBlock;
}

ASDJitterPairBlock::~ASDJitterPairBlock() 
{
	if (m_pFibreABlockedFlag) 
		delete m_pFibreABlockedFlag;
	if (m_pFibreBBlockedFlag) 
		delete m_pFibreBBlockedFlag;
	if (m_pFibreAPeakTime) 
		delete m_pFibreAPeakTime;
	if (m_pFibreBPeakTime) 
		delete m_pFibreBPeakTime;
}


// ReleasePointers
void ASDTrainBlock::ReleasePointers() 
{
	m_pConsecutiveDistanceIDI = NULL;
	m_pConsecutiveDistance = NULL;

	if(m_pMicroBlock) 
		m_pMicroBlock->ReleasePointers();
	if(m_pMacroBlock) 
		m_pMacroBlock->ReleasePointers();
	if(m_pFiringBlock) 
		m_pFiringBlock->ReleasePointers();
	if(m_pEnsembleBlock) 
		m_pEnsembleBlock->ReleasePointers();
}
void ASDMicroBlock::ReleasePointers() 
{
	m_pNF_MUP_Tmpl = NULL;
	m_templateBlock.ReleasePointers();
}

void ASDMacroBlock::ReleasePointers() 
{
	m_templateBlock.ReleasePointers();
}

void ASDTemplateBlock::ReleasePointers() 
{
	pTemplateWaveform_ = NULL;
}

void ASDFiringBlock::ReleasePointers() 
{
	pIDI_ = NULL;
}

void ASDEnsembleBlock::ReleasePointers() 
{
	m_pSelectedMupNumber = NULL;
	m_pShift = NULL;

	if(m_pJitterBlock) 
		m_pJitterBlock->ReleasePointers();
}

void ASDJitterBlock::ReleasePointers() 
{
	//m_rasterMeanAmplOfTurn = NULL;
	//m_rasterSumAmplOfTurn = NULL;
	m_pNF_MupTmplIndexOfMFP = NULL;
	m_ppPeakTime = NULL;

	if(m_pJitterPairBlock && m_nJitterPairArraySize > 0)
	{
		for(int i = 0; i < m_nJitterPairArraySize; i++)
			m_pJitterPairBlock[i].ReleasePointers();
		
		m_pJitterPairBlock = NULL;
	}
	
	if(m_pJitterTurnBlock && m_nJitterTurnArraySize > 0)
	{
		for(int i = 0; i < m_nJitterTurnArraySize; i++)
			m_pJitterTurnBlock[i].ReleasePointers();

		m_pJitterTurnBlock = NULL;	
	}
}

void ASDJitterPairBlock::ReleasePointers() 
{
	m_pFibreABlockedFlag = NULL;
	m_pFibreBBlockedFlag = NULL;
	m_pFibreAPeakTime = NULL;
	m_pFibreBPeakTime = NULL;
}

void ASDJitterTurnBlock::ReleasePointers()
{

}


// Read Blocks
BOOL ASDTrainBlock::ReadBlock(std::ifstream& is) 
{
	// Begin read
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	int bHaveMacro, bHaveMacro2; // Place holders no longer used!
	is	>> m_trainNumber >> numberOfMUPs >> bHaveMacro
		>> bHaveMacro2 >> muActivityDuration_
		>> m_pMicroBlock->NF_MUP_TmplOnsetPosition_ >> m_pMicroBlock->NF_MUP_TmplEndPosition_
		>> m_pFiringBlock->IDImean_ >> m_pFiringBlock->IDIsd_;

	//TRACE("Train number:  %d\n", m_trainNumber);

	if (m_pConsecutiveDistance) 
		delete [] m_pConsecutiveDistance;
	m_pConsecutiveDistance = new float[numberOfMUPs];

	for(int i = 0; i < numberOfMUPs; i++)
		is	>> m_pConsecutiveDistance[i];

	if (m_pConsecutiveDistanceIDI) 
		delete [] m_pConsecutiveDistanceIDI;
	m_pConsecutiveDistanceIDI = new int[numberOfMUPs];

	for(int i = 0; i < numberOfMUPs; i++)
		is	>> m_pConsecutiveDistanceIDI[i];

	// -------------- Micro Template --------------
	if (!m_pMicroBlock->ReadBlock(is))
	{
		m_sError = m_pMicroBlock->getError();
		return FALSE;
	}

	// -------------- Macro Template --------------
    if (bHaveMacro) 
	{
		m_pMacroBlock = new ASDMacroBlock(this);

		if (!m_pMacroBlock->ReadBlock(is)) 
		{
			m_sError = m_pMacroBlock->getError();
			return FALSE;
		}
    }

	// --------------- Firing Data ----------------
	if(!m_pFiringBlock->ReadBlock(is))
	{
		m_sError = m_pFiringBlock->getError();
		return FALSE;
	}

	// -------------- Ensemble Data ---------------
	if(!m_pEnsembleBlock->ReadBlock(is))
	{
		m_sError = m_pEnsembleBlock->getError();
		return FALSE;
	}

	return TRUE;
}

BOOL ASDTrainBlock::ReadXMLBlock(TiXmlElement * trainElement)
{
	TiXmlElement * trainElementTmp;
	BOOL bContinueInner;

	while (trainElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			TiXmlAttribute * trainAttribute = trainElement->FirstAttribute();
			while (trainAttribute)
			{
				double temp = 0;
				const char* attributeName = trainAttribute->Name();
				if (strcmp(attributeName, "trainNumber") == 0)
				{
					trainAttribute->QueryIntValue(&m_trainNumber);
				} else if (strcmp(attributeName, "numberOfMUPsInTrain") == 0)
				{
					trainAttribute->QueryIntValue(&numberOfMUPs);
//				} else if (strcmp(attributeName, "haveMacroFlag") == 0)
//				{
//					trainAttribute->QueryIntValue(&bHaveMacro_);
//				} else if (strcmp(attributeName, "have2ndMacroFlag") == 0)
//				{
//					trainAttribute->QueryIntValue(&bHaveMacro2_);
				} else if (strcmp(attributeName, "durationOfMU_Activity") == 0)
				{
					trainAttribute->QueryDoubleValue(&temp);
					muActivityDuration_ = (float) temp;
//				} else if (strcmp(attributeName, "NF_MUP_TmplOnsetPosition") == 0)
//				{
//					trainAttribute->QueryIntValue(&NF_MUP_Tmpl_OnsetPosition_);
//				} else if (strcmp(attributeName, "NF_MUP_TmplEndPosition") == 0)
//				{
//					trainAttribute->QueryIntValue(&NF_MUP_TmplEndPosition_);
//				} else if (strcmp(attributeName, "meanIDI") == 0)
//				{
//					trainAttribute->QueryDoubleValue(&temp);
//					m_pFiringBlock->IDImean_ = (float) temp;
//				} else if (strcmp(attributeName, "IDIstandardDeviation") == 0)
//				{
//					trainAttribute->QueryDoubleValue(&temp);
//					m_pFiringBlock->IDIsd_ = (float) temp;
				} else if (strcmp(attributeName, "consecutiveDistanceValues") == 0)
				{
					if (m_pConsecutiveDistance) 
						delete [] m_pConsecutiveDistance;
					m_pConsecutiveDistance = new float[numberOfMUPs];
					string consecutiveDistanceValues(trainAttribute->Value());
						string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = consecutiveDistanceValues.find(delimiter)) != string::npos) {
						token = consecutiveDistanceValues.substr(0, pos);
						m_pConsecutiveDistance[i] = (float) atof(token.c_str());
						consecutiveDistanceValues.erase(0, pos + delimiter.length());
						i += 1;
					}
				}else if (strcmp(attributeName, "consecutiveDistanceIDIValues") == 0)
				{
					if (m_pConsecutiveDistanceIDI) 
						delete [] m_pConsecutiveDistanceIDI;
					m_pConsecutiveDistanceIDI = new int[numberOfMUPs];
	
					string consecutiveDistanceIDI_Values(trainAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = consecutiveDistanceIDI_Values.find(delimiter)) != string::npos) {
						token = consecutiveDistanceIDI_Values.substr(0, pos);
						m_pConsecutiveDistanceIDI[i] = atoi(token.c_str());
					consecutiveDistanceIDI_Values.erase(0, pos + delimiter.length());
					i += 1;
				}
				}else if(strcmp(attributeName, "microDataBlock") == 0)
				{
					m_pMicroBlock->ReadXMLBlock(trainElement);
					trainAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				}else if(strcmp(attributeName, "macroDataBlock") == 0)
				{
					m_pMacroBlock = new ASDMacroBlock(this);
					m_pMacroBlock->ReadXMLBlock(trainElement);
					trainAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				}else if(strcmp(attributeName, "firingDataBlock") == 0)
				{
					m_pFiringBlock->ReadXMLBlock(trainElement);
					trainAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				}else if(strcmp(attributeName, "ensembleDataBlock") == 0)
				{
					m_pEnsembleBlock->ReadXMLBlock(trainElement);
					trainAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				} 
				trainAttribute = trainAttribute->Next();
			}
			if(!bContinueInner)
				continue;
			trainElementTmp = trainElement;
			trainElement = trainElement->FirstChildElement();
			if(!trainElement)
			{
				trainElement = trainElementTmp;
				bContinueInner = FALSE;
			}
		}
		trainElement = trainElement->NextSiblingElement();
	}
	return TRUE;
}

BOOL ASDMicroBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;
	int version = abs(m_receivedPersistenceID) / 100;

	is	>> m_numberOfMUPsForNF_MUP_TmplCalc >> m_numberOfMUPsForNFmupJiggleCalc >> m_numberOfMUPsForJiggleCalc
    	>> NF_MUP_TmplLength_ >> m_numberOfSamplesForJiggleCalc;

	is	>> m_NFmupJiggle >> Bjiggle_ >> jiggle_
		>> m_NFmupJiggleMRE >> NF_EnsembleRMS_ >> NF_Threshold_ ;

    is	>> areaAmplitudeRatio_ >> phases_ >> turns_;

	if (version >= 3)
		is >> irregularityCoefficient_ >> alternativeIR_;
	else 
	{ 
		irregularityCoefficient_ = 0; 
		alternativeIR_ = 0; 
	}

	//	restore m_accelTmplate if m_ensembleTmplLength has been set
	if (NF_MUP_TmplLength_) 
		delete [] m_pNF_MUP_Tmpl;
	if (NF_MUP_TmplLength_ < 0) 
		NF_MUP_TmplLength_ = 0;
	
	if(NF_MUP_TmplLength_)
	{
		m_pNF_MUP_Tmpl = new int[NF_MUP_TmplLength_];
		for (int i = 0; i < NF_MUP_TmplLength_; ++i)
			is >> m_pNF_MUP_Tmpl[i];
	}

	// Read Template
	if(!m_templateBlock.ReadBlock(is))
	{
		m_sError = m_templateBlock.getError();
		return FALSE;
	}

	return TRUE;
}

BOOL ASDMicroBlock::ReadXMLBlock(TiXmlElement * microTmplElement)
{
	TiXmlElement * microTmplElementTmp;
	BOOL bContinueInner;
	while(microTmplElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			TiXmlAttribute * microTmplAttribute = microTmplElement->FirstAttribute();
			while (microTmplAttribute)
			{
				double temp = 0;
				const char* attributeName = microTmplAttribute->Name();

				if (strcmp(attributeName, "validity") == 0)
				{
					microTmplAttribute->QueryIntValue(&m_receivedPersistenceID);
				}else if (strcmp(attributeName, "numberOfMUPsForNF_MUP_TmplCalc") == 0)
				{
					microTmplAttribute->QueryIntValue(&m_numberOfMUPsForNF_MUP_TmplCalc);
				} else if (strcmp(attributeName, "numberOfMUPsForNF_JiggleCalc") == 0)
				{
					microTmplAttribute->QueryIntValue(&m_numberOfMUPsForNF_MUP_TmplCalc);
				} else if (strcmp(attributeName, "numberofMUPsForJiggleCalc") == 0)
				{
					microTmplAttribute->QueryIntValue(&m_numberOfMUPsForJiggleCalc);
				} else if (strcmp(attributeName, "NF_MUP_TmplOnsetPosition") == 0)
				{
					microTmplAttribute->QueryIntValue(&NF_MUP_TmplOnsetPosition_);
				} else if (strcmp(attributeName, "NF_MUP_TmplEndPosition") == 0)
				{
					microTmplAttribute->QueryIntValue(&NF_MUP_TmplEndPosition_);
				} else if (strcmp(attributeName, "NF_MUP_TmplLength") == 0)
				{
					microTmplAttribute->QueryIntValue(&NF_MUP_TmplLength_);
				} else if (strcmp(attributeName, "numberOfSamplesForJiggleCalc") == 0)
				{
					microTmplAttribute->QueryIntValue(&m_numberOfSamplesForJiggleCalc);
				} else if (strcmp(attributeName, "NF_Jiggle") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					m_NFmupJiggle = (float) temp;
				} else if (strcmp(attributeName, "Bjiggle") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					Bjiggle_ = (float) temp;
				} else if (strcmp(attributeName, "jiggle") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					jiggle_ = (float) temp;
				} else if (strcmp(attributeName, "NF_JiggleMRE") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					m_NFmupJiggleMRE = (float) temp;
				} else if (strcmp(attributeName, "NF_Duration") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					NF_Duration_ = (float) temp;
				} else if (strcmp(attributeName, "NF_Area") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					NF_Area_ = (float) temp;
				} else if (strcmp(attributeName, "NF_EnsembleRMS") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					NF_EnsembleRMS_ = (float) temp;
				} else if (strcmp(attributeName, "NF_MUP_TmplBaselineRMS") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					m_NF_MUP_TmplBaselineRMS = (float) temp;
				} else if (strcmp(attributeName, "NF_JiggleThreshold") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					NF_Threshold_ = (float) temp;
				} else if (strcmp(attributeName, "areaAmplitudeRatio") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					areaAmplitudeRatio_ = (float) temp;
				} else if (strcmp(attributeName, "shapeWidth") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					shapeWidth_ = (float) temp;
				} else if (strcmp(attributeName, "numberOfPhases") == 0)
				{
					microTmplAttribute->QueryIntValue(&phases_);
				} else if (strcmp(attributeName, "numberOfTurns") == 0)
				{
					microTmplAttribute->QueryIntValue(&turns_);
				} else if (strcmp(attributeName, "averagePhaseArea") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					irregularityCoefficient_ = (float) temp;
				} else if (strcmp(attributeName, "normalizedIR") == 0)
				{
					microTmplAttribute->QueryDoubleValue(&temp);
					alternativeIR_ = (float) temp;
//				} else if (strcmp(attributeName, "NF_Area") == 0)
//				{
//					microTmplAttribute->QueryDoubleValue(&temp);
//					NF_Area_ = (float) temp;
//				} else if (strcmp(attributeName, "NF_Duration") == 0)
//				{
//					microTmplAttribute->QueryDoubleValue(&temp);
//					NF_Duration_ = (float) temp;
				} else if (strcmp(attributeName, "NF_TemplateWaveformValues") == 0)
				{
					if (m_pNF_MUP_Tmpl) 
						delete [] m_pNF_MUP_Tmpl;
					if (NF_MUP_TmplLength_ < 0) 
						NF_MUP_TmplLength_ = 0;
			
					if(NF_MUP_TmplLength_)
					{
						m_pNF_MUP_Tmpl = new int[NF_MUP_TmplLength_];
	
						string NF_TemplateValues(microTmplAttribute->Value());
						string delimiter = ",";
						int pos = 0;
						int i = 0;
						string token;
						while ((pos = NF_TemplateValues.find(delimiter)) != string::npos) {
							token = NF_TemplateValues.substr(0, pos);
							m_pNF_MUP_Tmpl[i] = (long) atoi(token.c_str());
							NF_TemplateValues.erase(0, pos + delimiter.length());
							i += 1;
						}
					}
				}else if(strcmp(attributeName, "baseTemplateDataBlock") == 0)
				{
					m_templateBlock.ReadXMLBlock(microTmplElement);
					microTmplAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				}
				microTmplAttribute = microTmplAttribute->Next();
			}
			if(!bContinueInner)
				continue;
			microTmplElementTmp = microTmplElement;
			microTmplElement = microTmplElement->FirstChildElement();
			if(!microTmplElement)
			{
				microTmplElement = microTmplElementTmp;
				bContinueInner = FALSE;
			}
		}
		microTmplElement = microTmplElement->NextSiblingElement();
	}
	
	return TRUE;
}

BOOL ASDMacroBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	is >> m_negativePeakArea >> m_negativePeakDuration >> m_negativePeakAmplitude;
    is >> m_negativePeakOnsetPosition >> m_negativePeakEndPosition >> m_negativePeakPosition >> m_positivePeakPosition ;

	if(!m_templateBlock.ReadBlock(is))
	{
		m_sError = m_templateBlock.getError();
		return FALSE;
	}

	return TRUE;
}

BOOL ASDMacroBlock::ReadXMLBlock(TiXmlElement * macroTmplElement) 
{
	TiXmlElement * macroTmplElementTmp;
	BOOL bContinueInner;
	while(macroTmplElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			TiXmlAttribute * macroTmplAttribute = macroTmplElement->FirstAttribute();
			while (macroTmplAttribute)
			{
				double temp = 0;
				const char* attributeName = macroTmplAttribute->Name();

				if (strcmp(attributeName, "validity") == 0)
				{
					macroTmplAttribute->QueryIntValue(&m_receivedPersistenceID);
				}else if (strcmp(attributeName, "negPeakArea") == 0)
				{
					macroTmplAttribute->QueryDoubleValue(&temp);
					m_negativePeakArea = (float) temp;
				} else if (strcmp(attributeName,"negPeakDuration") == 0)
				{
					macroTmplAttribute->QueryDoubleValue(&temp);
					m_negativePeakDuration = (float) temp;
				} else if (strcmp(attributeName, "negPeakAmplitude") == 0)
				{
					macroTmplAttribute->QueryDoubleValue(&temp);
					m_negativePeakAmplitude = (float) temp;
				} else if (strcmp(attributeName, "peakOnset") == 0)
				{
					macroTmplAttribute->QueryIntValue(&m_negativePeakOnsetPosition);
				} else if (strcmp(attributeName, "peakEnd") == 0)
				{
					macroTmplAttribute->QueryIntValue(&m_negativePeakEndPosition);
				} else if (strcmp(attributeName, "peak") == 0)
				{
					macroTmplAttribute->QueryIntValue(&m_negativePeakPosition);
				} else if (strcmp(attributeName, "trough") == 0)
				{
					macroTmplAttribute->QueryIntValue(&m_positivePeakPosition);
				}else if(strcmp(attributeName, "baseTemplateDataBlock") == 0)
				{
					m_templateBlock.ReadXMLBlock(macroTmplElement);
					macroTmplAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				}
				macroTmplAttribute = macroTmplAttribute->Next();
			}
			if(!bContinueInner)
				continue;
			macroTmplElementTmp = macroTmplElement;
			macroTmplElement = macroTmplElement->FirstChildElement();
			if(!macroTmplElement)
			{
				macroTmplElement = macroTmplElementTmp;
				bContinueInner = FALSE;
			}
		}
		macroTmplElement = macroTmplElement->NextSiblingElement();
	}
	return TRUE;
}

BOOL ASDTemplateBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;

	if (!isPersistenceIDok()) 
		return FALSE;

	int version = abs(m_receivedPersistenceID) / 100;

	is	>> numberInAverage_ >> templateLength_;

	if (version >= 2) 
		is	>> samplingRate_ >> maxSlope_ >> maxSlopeIndex_;
	else 
	{
		samplingRate_ = 0; 
		maxSlope_ = 0; 
		maxSlopeIndex_ = -1; 
	}

	if (pTemplateWaveform_) 
		delete pTemplateWaveform_;
	pTemplateWaveform_ = new int[templateLength_];

    for (int i=0; i < templateLength_; ++i)
		is >> pTemplateWaveform_[i];

    is	>> onsetPosition_ >> endPosition_ >> negativePeakPosition_ >> positivePeakPosition_
		>> duration_ >> amplitude_>> area_ ;

 	if (version == 2)
	{
		is >> irregularityCoefficient_ >> alternativeIR_;
	}

	return TRUE;
}
BOOL ASDTemplateBlock::ReadXMLBlock(TiXmlElement * baseTmplElement) 
{

	TiXmlElement * baseTmplElementTmp;
	BOOL bContinueInner;
	while(baseTmplElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			TiXmlAttribute * tmplAttribute = baseTmplElement->FirstAttribute();
			while (tmplAttribute)
			{
				double temp = 0;
				const char* attributeName = tmplAttribute->Name();
				if (strcmp(attributeName, "validity") == 0)
				{
					tmplAttribute->QueryIntValue(&m_receivedPersistenceID);
				}else if (strcmp(attributeName, "numberInAverage") == 0)
				{
					tmplAttribute->QueryIntValue(&numberInAverage_);
				} else if (strcmp(attributeName, "templateLength") == 0)
				{
					tmplAttribute->QueryIntValue(&templateLength_);
				} else if (strcmp(attributeName, "samplingRate") == 0)
				{
					tmplAttribute->QueryIntValue(&samplingRate_);
				} else if (strcmp(attributeName, "maxSlope") == 0)
				{
					tmplAttribute->QueryDoubleValue(&temp);
					maxSlope_ = (float) temp;
				} else if (strcmp(attributeName, "indexOfMaxSlope") == 0)
				{
					tmplAttribute->QueryIntValue(&maxSlopeIndex_);
				} else if (strcmp(attributeName, "baseTemplateWaveformValues") == 0)
				{
					if (pTemplateWaveform_) 
						delete pTemplateWaveform_;
					pTemplateWaveform_ = new int[templateLength_];
			
					string templateValues(tmplAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = templateValues.find(delimiter)) != string::npos) 
					{
						token = templateValues.substr(0, pos);
							pTemplateWaveform_[i] = atoi(token.c_str());
							templateValues.erase(0, pos + delimiter.length());
							i += 1;
					}
				} else if (strcmp(attributeName, "onsetPosition") == 0)
				{
					tmplAttribute->QueryIntValue(&onsetPosition_);
				} else if (strcmp(attributeName, "endPosition") == 0)
				{
					tmplAttribute->QueryIntValue(&endPosition_);
				} else if (strcmp(attributeName, "negativePeakPosition") == 0)
				{
					tmplAttribute->QueryIntValue(&negativePeakPosition_);
				} else if (strcmp(attributeName, "positivePeakPosition") == 0)
				{
					tmplAttribute->QueryIntValue(&positivePeakPosition_);
				} else if (strcmp(attributeName, "duration") == 0)
				{
					tmplAttribute->QueryDoubleValue(&temp);
					duration_ = (float) temp;
				} else if (strcmp(attributeName, "amplitude") == 0)
				{
					tmplAttribute->QueryDoubleValue(&temp);
					amplitude_ = (float) temp;
				} else if (strcmp(attributeName, "area") == 0)
				{
					tmplAttribute->QueryDoubleValue(&temp);
					area_ = (float) temp;
				} else if (strcmp(attributeName, "templateBaselineRMS") == 0)
				{
					tmplAttribute->QueryDoubleValue(&temp);
					templateBaselineRMS_ = (float) temp;
				} 
				tmplAttribute = tmplAttribute->Next();
			}
			baseTmplElementTmp = baseTmplElement;
			baseTmplElement = baseTmplElement->FirstChildElement();
			if(!baseTmplElement)
			{
				baseTmplElement = baseTmplElementTmp;
				bContinueInner = FALSE;
			}
		}
		baseTmplElement = baseTmplElement->NextSiblingElement();
	}
	return TRUE;
}

BOOL ASDFiringBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	is	>> identificationRate_ >> IDImean_ >> IDIsd_ 
		>> IDI_MACID_	>> IDI_MPACID_>> FR_MACID_ >> bIsValid_;

	return TRUE;
}

BOOL ASDFiringBlock::ReadXMLBlock(TiXmlElement * firingElement) 
{
	TiXmlAttribute * firingAttribute = firingElement->FirstAttribute();
	while (firingAttribute)
	{
		double temp = 0;
		const char* attributeName = firingAttribute->Name();

		if (strcmp(attributeName, "identificationRate") == 0)
		{
			firingAttribute->QueryDoubleValue(&temp);
			identificationRate_ = (float) temp;
		} else if (strcmp(attributeName, "meanIDI") == 0)
		{
			firingAttribute->QueryDoubleValue(&temp);
			IDImean_ = (float) temp;
		} else if (strcmp(attributeName, "IDIstd") == 0)
		{
			firingAttribute->QueryDoubleValue(&temp);
			IDIsd_ = (float) temp;
		} else if (strcmp(attributeName, "IDI_MACID") == 0)
		{
			firingAttribute->QueryDoubleValue(&temp);
			IDI_MACID_ = (float) temp;
		} else if (strcmp(attributeName, "IDI_MPACID") == 0)
		{
			firingAttribute->QueryDoubleValue(&temp);
			IDI_MPACID_ = (float) temp;
		} else if (strcmp(attributeName, "FR_MACID") == 0)
		{
			firingAttribute->QueryDoubleValue(&temp);
			FR_MACID_ = (float) temp;
		} else if (strcmp(attributeName, "isValidFlag") == 0)
		{
			firingAttribute->QueryIntValue(&bIsValid_);
		} else if (strcmp(attributeName, "NumberOfIDIs") == 0)
		{
			firingAttribute->QueryIntValue(&numberOfIDIs_);
		} else if (strcmp(attributeName, "IDI_Values") == 0)
		{
			if (pIDI_) 
				delete pIDI_;
			pIDI_ = new float[numberOfIDIs_];
			
			string IDI_Values(firingAttribute->Value());
			string delimiter = ",";
			int pos = 0;
			int i = 0;
			string token;
			while ((pos = IDI_Values.find(delimiter)) != string::npos) 
			{
				token = IDI_Values.substr(0, pos);
				pIDI_[i] = (float)atof(token.c_str());
				IDI_Values.erase(0, pos + delimiter.length());
				i += 1;
			}
		}
		firingAttribute = firingAttribute->Next();
	}
	return TRUE;
}

BOOL ASDEnsembleBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	int trainNum; // dummy, since the train number here is redundant
	is	>> trainNum >> shimmerCoV_ 
		>> m_ensembleTmplLength
		>> m_numberOfMupsAvailableForReplacement >> m_numberOfMupsSelectedForEnsembleAnalysis 
		>> m_lastMupSelectedForEnsembleAnalysis >> m_lastValidMupSelectedForEnsembleAnalysis
		>> fibreCount_ >> jiggle_ 
//		>> m_NFmupJiggle >> Bjiggle_;
		>> Bjiggle_;

	if(m_pSelectedMupNumber) 
		delete [] m_pSelectedMupNumber;
	m_pSelectedMupNumber = NULL;
	
	if(m_pShift) 
		delete [] m_pShift;
	m_pShift = NULL;

//	if(m_pParentTrainBlock->m_pParentASDIO->bEnsembleDataHasBeenProcessed_) 
//	{
		// read selectedMupNumber
		m_pSelectedMupNumber = new int[TARGET_NUM_OF_JITTER_SAMPLES];
		
		for (int i=0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
			is >> m_pSelectedMupNumber[i];
		
		// read shift
		m_pShift = new int[TARGET_NUM_OF_JITTER_SAMPLES];
		
		for (int i=0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
			is >> m_pShift[i];

		// read jitterBlock
		if (!m_pJitterBlock->ReadBlock(is)) 
		{
			m_sError = m_pJitterBlock->getError();
			return FALSE;
		}
//	}
	return TRUE;
}


BOOL ASDEnsembleBlock::ReadXMLBlock(TiXmlElement * ensembleDataElement) 
{
	TiXmlElement * ensembleDataElementTmp;
	BOOL bContinueInner;
	while(ensembleDataElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			TiXmlAttribute * ensembleDataAttribute = ensembleDataElement->FirstAttribute();
			while (ensembleDataAttribute)
			{
				double temp = 0;
				const char* attributeName = ensembleDataAttribute->Name();
				if (strcmp(attributeName, "validity") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&m_receivedPersistenceID);
				}else if (strcmp(attributeName, "shimmerCoV") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					shimmerCoV_ = (float) temp;
				} else if (strcmp(attributeName, "ensembleTmplLength") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&m_ensembleTmplLength);
				} else if (strcmp(attributeName, "numberAvailableForReplacement") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&m_numberOfMupsAvailableForReplacement);
				} else if (strcmp(attributeName, "numberSelected") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&m_numberOfMupsSelectedForEnsembleAnalysis);
				} else if (strcmp(attributeName, "lastSelected") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&m_lastMupSelectedForEnsembleAnalysis);
				} else if (strcmp(attributeName, "lastValid") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&m_lastValidMupSelectedForEnsembleAnalysis);
				} else if (strcmp(attributeName, "fibreCount") == 0)
				{
					ensembleDataAttribute->QueryIntValue(&fibreCount_);
				} else if (strcmp(attributeName, "jiggle") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					jiggle_ = (float) temp;
				} 
//				else if (strcmp(attributeName, "NFmupJiggle") == 0)
//				{
//					ensembleDataAttribute->QueryDoubleValue(&temp);
//					m_NFmupJiggle = (float) temp;
//				} 
				else if (strcmp(attributeName, "NFmupJitter") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					m_NFmupJitter = (float) temp;
				} else if (strcmp(attributeName, "Bjiggle") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					Bjiggle_ = (float) temp;
				} else if (strcmp(attributeName, "meanTurnArea") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					meanTrnArea_ = (float) temp;
				} else if (strcmp(attributeName, "NF_MUP_Dispersion") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					NF_MupDispersion_ = (float) temp;
				} else if (strcmp(attributeName, "maxTurnInterval") == 0)
				{
					ensembleDataAttribute->QueryDoubleValue(&temp);
					maxTrnInterval_ = (float) temp;
				} else if (strcmp(attributeName, "selectedMUP_Numbers") == 0)
				{
					if (m_pSelectedMupNumber) 
						delete m_pSelectedMupNumber;
					m_pSelectedMupNumber = new BOOL[TARGET_NUM_OF_JITTER_SAMPLES];
			
					string numbers(ensembleDataAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = numbers.find(delimiter)) != string::npos) 
					{
						token = numbers.substr(0, pos);
						m_pSelectedMupNumber[i] = atoi(token.c_str());
						numbers.erase(0, pos + delimiter.length());
						i += 1;
					}
				} else if (strcmp(attributeName, "shiftValues") == 0)
				{
					if (m_pShift) 
						delete m_pShift;
					m_pShift = new int[TARGET_NUM_OF_JITTER_SAMPLES];
	
					string values(ensembleDataAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = values.find(delimiter)) != string::npos) 
					{
						token = values.substr(0, pos);
						m_pShift[i] =  atoi(token.c_str());
						values.erase(0, pos + delimiter.length());
						i += 1;
					}
				}else if(strcmp(attributeName, "jitterDataBlock") == 0)
				{
					m_pJitterBlock->ReadXMLBlock(ensembleDataElement);
					ensembleDataAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				}
				ensembleDataAttribute = ensembleDataAttribute->Next();
			}
			if(!bContinueInner)
				continue;
			ensembleDataElementTmp = ensembleDataElement;
			ensembleDataElement = ensembleDataElement->FirstChildElement();
			if(!ensembleDataElement)
			{
				ensembleDataElement = ensembleDataElementTmp;
				bContinueInner = FALSE;
			}
		}
		ensembleDataElement = ensembleDataElement->NextSiblingElement();
	}

/*
is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	int trainNum; // dummy, since the train number here is redundant
	is	>> trainNum >>  

*/
	return TRUE;
}


BOOL ASDJitterBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	is >> fibreCount_;

	if (m_pNF_MupTmplIndexOfMFP) 
		delete m_pNF_MupTmplIndexOfMFP;
	m_pNF_MupTmplIndexOfMFP = new int[fibreCount_];
	for (int i = 0; i < fibreCount_; i++) 
		is >> m_pNF_MupTmplIndexOfMFP[i];

	BOOL temp; 
	is >> temp;
	
//	if (m_aIsTurnFTCalculated) 
//	{
//		delete [] m_aIsTurnFTCalculated;
//		m_aIsTurnFTCalculated = NULL;
//	}

	if (temp)
	{
//		m_aIsTurnFTCalculated = new BOOL[fibreCount_];
//		for (int i = 0; i< fibreCount_; i++) 
//			is >> m_aIsTurnFTCalculated[i];
//		m_aIsTurnFTCalculated = new BOOL[fibreCount_];
		BOOL tmp;
		for (int i = 0; i< fibreCount_; i++) 
			is >> tmp;
	}

	is >> m_nJitterPairArraySize >> m_nNumberOfJitterPairs 
	   >> m_nNumberOfTurnsForJitter >> m_nJitterTurnArraySize;

	// -------------------- Jitter Pairs ----------------------
	deleteJitterPairBlocks();

	if (m_nJitterPairArraySize > 0) 
	{
		if(!createJitterPairBlocks()) 
			return FALSE;

		for (int i = 0; i < m_nNumberOfJitterPairs; i++) 
		{
			if (!m_pJitterPairBlock[i].ReadBlock(is)) 
			{
				m_sError = m_pJitterPairBlock[i].getError();
				return FALSE;
			}
		}
	}

	// -------------------- Jitter Turns ----------------------
	deleteJitterTurnBlocks();

	if (m_nJitterTurnArraySize > 0)	
	{
		if(!createJitterTurnBlocks()) 
			return FALSE;

		for (int i = 0; i < m_nJitterTurnArraySize; i++) 
		{
			if (!m_pJitterTurnBlock[i].ReadBlock(is)) 
			{
				m_sError = m_pJitterTurnBlock[i].getError();
				return FALSE;
			}
		}
	}

	// --------------------- Peak Time ------------------------
	if (m_ppPeakTime)
	{
		for (int i = 0; i < m_nJitterTurnArraySize; i++)
		{
			if (m_ppPeakTime[i])
				delete [] m_ppPeakTime[i];
		}
		delete [] m_ppPeakTime;
		m_ppPeakTime = NULL;
    }
	
	if (m_nJitterTurnArraySize > 0)
	{
		m_ppPeakTime = new float*[m_nJitterTurnArraySize];

		for (int i = 0; i < m_nJitterTurnArraySize; i++) 
		{
			m_ppPeakTime[i] = new float[TARGET_NUM_OF_JITTER_SAMPLES];
			memset(m_ppPeakTime[i], 0, sizeof(float)*TARGET_NUM_OF_JITTER_SAMPLES);

			for (int j = 0; j < m_pParentEnsembleBlock->m_numberOfMupsSelectedForEnsembleAnalysis; j++)
				is >> m_ppPeakTime[i][j];
		}
	}

	return TRUE;
}
BOOL ASDJitterBlock::ReadXMLBlock(TiXmlElement * jitterDataElement) 
{
	TiXmlElement * jitterDataElementTmp;
	BOOL bContinueInner;
	while(jitterDataElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			int pairNumber = 0;
			int turnNumber = 0;
			int peakNumber = 0;
			TiXmlAttribute * jitterDataAttribute = jitterDataElement->FirstAttribute();
			while (jitterDataAttribute)
			{
				double temp = 0;
				const char* attributeName = jitterDataAttribute->Name();
//				if (strcmp(attributeName, VARIABLE_TO_STRING(validity)) == 0)
//				{
//					jitterDataAttribute->QueryIntValue(&isValid_);
//				} else 
				if (strcmp(attributeName, "fibreCount") == 0)
				{
					jitterDataAttribute->QueryIntValue(&fibreCount_);
				} else if (strcmp(attributeName, "jitterPairArraySize") == 0)
				{
					jitterDataAttribute->QueryIntValue(&m_nJitterPairArraySize);
					createJitterPairBlocks(); 
				} else if (strcmp(attributeName, "jitterPairDataBlock") == 0)
				{
					m_pJitterPairBlock[pairNumber].ReadXMLBlock(jitterDataElement);
					jitterDataAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				} else if (strcmp(attributeName, "numberOfJitterPairs") == 0)
				{
					jitterDataAttribute->QueryIntValue(&m_nNumberOfJitterPairs);
				} else if (strcmp(attributeName, "jitterTurnArraySize") == 0)
				{
					jitterDataAttribute->QueryIntValue(&m_nJitterTurnArraySize);
					createJitterTurnBlocks(); 
				} else if (strcmp(attributeName, "jitterTurnDataBlock") == 0)
				{
					if(turnNumber < m_nJitterTurnArraySize)
						m_pJitterTurnBlock[turnNumber].ReadXMLBlock(jitterDataElement);

					jitterDataAttribute = NULL;
					bContinueInner = FALSE;
					continue;
				} else if (strcmp(attributeName, "pairNumber") == 0)
				{
					jitterDataAttribute->QueryIntValue(&pairNumber);
				} else if (strcmp(attributeName, "turnNumber") == 0)
				{
					jitterDataAttribute->QueryIntValue(&turnNumber);
				} else if (strcmp(attributeName, "peakNumber") == 0)
				{
					jitterDataAttribute->QueryIntValue(&peakNumber);
				} else if (strcmp(attributeName, "numberOfTurnsForJitter") == 0)
				{
					jitterDataAttribute->QueryIntValue(&m_nNumberOfTurnsForJitter);
				} else if (strcmp(attributeName, "NF_MFP_IndicieValues") == 0)
				{
					if (m_pNF_MupTmplIndexOfMFP) 
						delete m_pNF_MupTmplIndexOfMFP;
					m_pNF_MupTmplIndexOfMFP = new int[fibreCount_];
	
					string indicies(jitterDataAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = indicies.find(delimiter)) != string::npos) 
					{
						token = indicies.substr(0, pos);
						m_pNF_MupTmplIndexOfMFP[i] = atoi(token.c_str());
						indicies.erase(0, pos + delimiter.length());
						i += 1;
					}
				} else if (strcmp(attributeName, "numberOfPeaks") == 0)
				{
					if (m_ppPeakTime)
					{
						for (int i = 0; i < m_nJitterTurnArraySize; i++)
						{
							if (m_ppPeakTime[i])
								delete [] m_ppPeakTime[i];
						}
						delete [] m_ppPeakTime;
						m_ppPeakTime = NULL;
					}
	
					m_ppPeakTime = new float*[m_nJitterTurnArraySize];
					for (int i = 0; i < m_nJitterTurnArraySize; i++) 
					{
						m_ppPeakTime[i] = new float[TARGET_NUM_OF_JITTER_SAMPLES];
						memset(m_ppPeakTime[i], 0, sizeof(float)*TARGET_NUM_OF_JITTER_SAMPLES);
					}
				}else if (strcmp(attributeName, "peakTimeValues") == 0)
				{
					string values(jitterDataAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int j = 0;
					string token;
					while ((pos = values.find(delimiter)) != string::npos) 
					{
						token = values.substr(0, pos);
						m_ppPeakTime[peakNumber][j] = (float)atof(token.c_str());
						values.erase(0, pos + delimiter.length());
						j += 1;
					}
				}
				jitterDataAttribute = jitterDataAttribute->Next();
			}
			if(!bContinueInner)
				continue;

			jitterDataElementTmp = jitterDataElement;
			jitterDataElement = jitterDataElement->FirstChildElement();
			if(!jitterDataElement)
			{
				jitterDataElement = jitterDataElementTmp;
				bContinueInner = FALSE;
			}
		}
		jitterDataElement = jitterDataElement->NextSiblingElement();
	}

/*
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;
*/

	return TRUE;
}

BOOL ASDJitterPairBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	is >> bIsValid_ >> fJitterMCD_ >> fMeanIPI_ >> fPercentBlocking_ >> nFibreAIndex_
	   >> nFibreBIndex_;

	m_pFibreABlockedFlag = new BOOL[TARGET_NUM_OF_JITTER_SAMPLES];
	m_pFibreBBlockedFlag = new BOOL[TARGET_NUM_OF_JITTER_SAMPLES];
	m_pFibreAPeakTime = new float[TARGET_NUM_OF_JITTER_SAMPLES];
	m_pFibreBPeakTime = new float[TARGET_NUM_OF_JITTER_SAMPLES];

	for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; i++)	
	{
		is	>> m_pFibreABlockedFlag[i] >> m_pFibreBBlockedFlag[i]
			>> m_pFibreAPeakTime[i] >> m_pFibreBPeakTime[i];
	}

	return TRUE;
}
BOOL ASDJitterPairBlock::ReadXMLBlock(TiXmlElement * jitterPairElement) 
{
	TiXmlElement * jitterPairElementTmp;
	BOOL bContinueInner;
	while(jitterPairElement)
	{
		bContinueInner = TRUE;
		while(bContinueInner)
		{
			TiXmlAttribute * jitterPairAttribute = jitterPairElement->FirstAttribute();
			while (jitterPairAttribute)
			{
				double temp = 0;
				const char* attributeName = jitterPairAttribute->Name();

				if (strcmp(attributeName, "jitterMCD") == 0)
				{
					jitterPairAttribute->QueryDoubleValue(&temp);
					fJitterMCD_ = (float) temp;
				} else if (strcmp(attributeName, "meanIPI") == 0)
				{
					jitterPairAttribute->QueryDoubleValue(&temp);
					fMeanIPI_ = (float) temp;
				} else if (strcmp(attributeName, "percentBlocking") == 0)
				{
					jitterPairAttribute->QueryDoubleValue(&temp);
					fPercentBlocking_ = (float) temp;
				} else if (strcmp(attributeName, "fibreAIndex") == 0)
				{
					jitterPairAttribute->QueryIntValue(&nFibreAIndex_);
				} else if (strcmp(attributeName, "fibreBIndex") == 0)
				{
					jitterPairAttribute->QueryIntValue(&nFibreBIndex_);
				} else if (strcmp(attributeName, "isValidFlag") == 0)
				{
					jitterPairAttribute->QueryIntValue(&bIsValid_);
				} else if (strcmp(attributeName, "fibreA_BlockedFlagValues") == 0)
				{
					if (m_pFibreABlockedFlag) 
						delete m_pFibreABlockedFlag;
					m_pFibreABlockedFlag = new BOOL[TARGET_NUM_OF_JITTER_SAMPLES];
	
					string flags(jitterPairAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = flags.find(delimiter)) != string::npos) 
					{
						token = flags.substr(0, pos);
						m_pFibreABlockedFlag[i] = (BOOL) atoi(token.c_str());
						flags.erase(0, pos + delimiter.length());
						i += 1;
					}	
				} else if (strcmp(attributeName, "fibreB_BlockedFlagValues") == 0)
				{
					if (m_pFibreBBlockedFlag) 
						delete m_pFibreBBlockedFlag;
					m_pFibreBBlockedFlag = new BOOL[TARGET_NUM_OF_JITTER_SAMPLES];
	
					string flags(jitterPairAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = flags.find(delimiter)) != string::npos) 
					{
						token = flags.substr(0, pos);
						m_pFibreBBlockedFlag[i] = (BOOL) atoi(token.c_str());
						flags.erase(0, pos + delimiter.length());
						i += 1;
					}
				} else if (strcmp(attributeName, "fibreA_PeakTimeValues") == 0)
				{
					if (m_pFibreAPeakTime) 
						delete m_pFibreAPeakTime;
					m_pFibreAPeakTime = new float[TARGET_NUM_OF_JITTER_SAMPLES];
		
					string peaks(jitterPairAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = peaks.find(delimiter)) != string::npos) 
					{
						token = peaks.substr(0, pos);
						m_pFibreAPeakTime[i] = (float) atof(token.c_str());
						peaks.erase(0, pos + delimiter.length());
						i += 1;
					}
				} else if (strcmp(attributeName, "fibreB_PeakTimeValues") == 0)
				{
					if (m_pFibreBPeakTime) 
						delete m_pFibreBPeakTime;
					m_pFibreBPeakTime = new float[TARGET_NUM_OF_JITTER_SAMPLES];
		
					string peaks(jitterPairAttribute->Value());
					string delimiter = ",";
					int pos = 0;
					int i = 0;
					string token;
					while ((pos = peaks.find(delimiter)) != string::npos) 
					{
						token = peaks.substr(0, pos);
						m_pFibreBPeakTime[i] = (float) atof(token.c_str());
						peaks.erase(0, pos + delimiter.length());
						i += 1;
					}
				}
				jitterPairAttribute = jitterPairAttribute->Next();
			}
			if(!bContinueInner)
				continue;

			jitterPairElementTmp = jitterPairElement;
			jitterPairElement = jitterPairElement->FirstChildElement();
			if(!jitterPairElement)
			{
				jitterPairElement = jitterPairElementTmp;
				bContinueInner = FALSE;
			}
		}
		jitterPairElement = jitterPairElement->NextSiblingElement();
	}
	
	return TRUE;
}

BOOL ASDJitterTurnBlock::ReadBlock(std::ifstream& is) 
{
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) 
		return FALSE;

	int type;
	is	>> type >> time_ >> ampl_ >> rise_ >> fall_ >> widthInSamples_ >> sharpness_
		>> num_ >> index_;
	type_ = (TurnType)type;

	return TRUE;
}
BOOL ASDJitterTurnBlock::ReadXMLBlock(TiXmlElement * jitterTurnElement) 
{
//	TiXmlElement * jitterTurnElementTmp;
//	BOOL bContinueInner;
//	while(jitterTurnElement)
//	{
//		bContinueInner = TRUE;
//		while(bContinueInner)
//		{
			TiXmlAttribute * jitterTurnAttribute = jitterTurnElement->FirstAttribute();
			while (jitterTurnAttribute)
			{
				double temp = 0;
				int type;
				const char* attributeName = jitterTurnAttribute->Name();

				if (strcmp(attributeName, "type") == 0)
				{
					jitterTurnAttribute->QueryIntValue(&type);
					type_ = (TurnType)type;
				} else if (strcmp(attributeName, "time") == 0)
				{
					jitterTurnAttribute->QueryDoubleValue(&temp);
					time_ = (float) temp;
				} else if (strcmp(attributeName, "amplitude") == 0)
				{
					jitterTurnAttribute->QueryDoubleValue(&temp);
					ampl_ = (float) temp;
				} else if (strcmp(attributeName, "rise") == 0)
				{
					jitterTurnAttribute->QueryDoubleValue(&temp);
					rise_ = (float) temp;
				} else if (strcmp(attributeName, "fall") == 0)
				{
					jitterTurnAttribute->QueryDoubleValue(&temp);
					fall_ = (float) temp;
				} else if (strcmp(attributeName, "sharpness") == 0)
				{
					jitterTurnAttribute->QueryDoubleValue(&temp);
					sharpness_ = (float) temp;
				} else if (strcmp(attributeName, "widthInSamples") == 0)
				{
					jitterTurnAttribute->QueryIntValue(&widthInSamples_);
				} else if (strcmp(attributeName, "jitterTurnNumber") == 0)
				{
					jitterTurnAttribute->QueryIntValue(&num_);
				} else if (strcmp(attributeName, "index") == 0)
				{
					jitterTurnAttribute->QueryIntValue(&index_);
				}
				jitterTurnAttribute = jitterTurnAttribute->Next();
			}
//			if(!bContinueInner)
//				continue;

//			jitterTurnElementTmp = jitterTurnElement;
//			jitterTurnElement = jitterTurnElement->FirstChildElement();
//			if(!jitterTurnElement)
//			{
//				jitterTurnElement = jitterTurnElementTmp;
//				bContinueInner = FALSE;
//			}
//		}
//		jitterTurnElement = jitterTurnElement->NextSiblingElement();
//	}
	return TRUE;
}

BOOL ASDTrainBlock::WriteXMLBlock(TiXmlElement * train)
{
	train->SetAttribute("trainNumber", m_trainNumber);
	train->SetAttribute("numberOfMUPsInTrain", numberOfMUPs);
	train->SetDoubleAttribute("durationOfMU_Activity", muActivityDuration_);

	TiXmlElement * CD_element = new TiXmlElement( "consecutiveDistances" );

	string temp = "";
	for(int i = 0; i < numberOfMUPs; i++)
		temp += to_string(m_pConsecutiveDistance[i]) + ",";
	CD_element->SetAttribute("consecutiveDistanceValues", temp.c_str());
	train->LinkEndChild(CD_element); // link the CD element to the train node
	

	TiXmlElement * CD_IDI_element = new TiXmlElement( "consecutiveDistanceIDIs" );

	temp = "";
	for(int i = 0; i < numberOfMUPs; i++)
		temp += to_string(m_pConsecutiveDistanceIDI[i]) + ",";
	CD_IDI_element->SetAttribute("consecutiveDistanceIDIValues", temp.c_str());
	train->LinkEndChild(CD_IDI_element); // link the CD IDI element to the train node


/*
	TiXmlElement * consecutiveDistance = new TiXmlElement( "ConsecutiveDistances" );
	for(int i = 0; i < numberOfMUPs; i++)
	{
		TiXmlElement * value = new TiXmlElement( "Value" );
		char str[20];
		sprintf(str, "%4.6f", m_pConsecutiveDistance[i]);
		value->LinkEndChild(new TiXmlText(str));
		consecutiveDistance->LinkEndChild(value);
	}
	train->LinkEndChild(consecutiveDistance);

	TiXmlElement * consecutiveDistanceIDI = new TiXmlElement( "ConsecutiveDistanceIDIs" );
	for(int i = 0; i < numberOfMUPs; i++)
	{
		TiXmlElement * value = new TiXmlElement( "Value" );
		char str[20];
		sprintf(str, "%4.6f", m_pConsecutiveDistanceIDI[i]);
		value->LinkEndChild(new TiXmlText(str));
		consecutiveDistanceIDI->LinkEndChild(value);
	}
	train->LinkEndChild(consecutiveDistanceIDI);
*/


	TiXmlElement * microElement = new TiXmlElement( "microTemplateData" );
	m_pMicroBlock->WriteXMLBlock(microElement);
	train->LinkEndChild(microElement); // link the micro element to the train element

	if (m_pParentASDIO->bHaveMacro_)
	{
		TiXmlElement * macroElement = new TiXmlElement( "macroTemplateData" );
		m_pMacroBlock->WriteXMLBlock(macroElement);
		train->LinkEndChild(macroElement);// link the macro data element to the train element
	}

	TiXmlElement * firingElement = new TiXmlElement( "firingData" );
	m_pFiringBlock->WriteXMLBlock(firingElement);
	train->LinkEndChild(firingElement);// link the firing data element to the train element


	TiXmlElement * ensembleElement = new TiXmlElement( "ensembleData" );
	m_pEnsembleBlock->WriteXMLBlock(ensembleElement);
	train->LinkEndChild(ensembleElement); // link the ensmeble data element to the train element

	return TRUE;
}

// WriteBlock
BOOL ASDTrainBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	BOOL bHaveMacro, bHaveMacro2;
	bHaveMacro = 0;
	bHaveMacro2 = 0; // Write dummy values for now
	os	<< m_trainNumber << " " << numberOfMUPs << " " << bHaveMacro	<< " " 
		<< bHaveMacro2 << " " << muActivityDuration_ << " " 
		<< m_pMicroBlock->NF_MUP_TmplOnsetPosition_ << " " << m_pMicroBlock->NF_MUP_TmplEndPosition_ << " " 
		<< m_pFiringBlock->IDImean_ << " " << m_pFiringBlock->IDIsd_ << std::endl;
	
    for(int i = 0; i < numberOfMUPs; i++)
		os	<< m_pConsecutiveDistance[i] << " ";

	os << std::endl;

    for(int i = 0; i < numberOfMUPs; i++)
		os	<< m_pConsecutiveDistanceIDI[i] << " " ;

	os << std::endl;

	m_pMicroBlock->WriteBlock(os);
    
	if (m_pParentASDIO->bHaveMacro_)
		m_pMacroBlock->WriteBlock(os);
	
	m_pFiringBlock->WriteBlock(os);
	
	m_pEnsembleBlock->WriteBlock(os);

	os << std::endl;
	
	return TRUE;
}

BOOL ASDMicroBlock::WriteXMLBlock(TiXmlElement * micro)
{
	micro->SetAttribute("microDataBlock", "");
	micro->SetAttribute("validity", getValidityInteger());
	micro->SetAttribute("numberOfMUPsForNF_JiggleCalc", m_numberOfMUPsForNF_MUP_TmplCalc);
	micro->SetAttribute("numberOfSamplesForJiggleCalc", m_numberOfSamplesForJiggleCalc);
	micro->SetDoubleAttribute("Bjiggle", Bjiggle_);
	micro->SetDoubleAttribute("jiggle", jiggle_);
	micro->SetDoubleAttribute("areaAmplitudeRatio", areaAmplitudeRatio_);
	micro->SetDoubleAttribute("shapeWidth", shapeWidth_);
	micro->SetAttribute("numberOfPhases", phases_);
	micro->SetAttribute("numberOfTurns", turns_);
	micro->SetDoubleAttribute("averagePhaseArea", irregularityCoefficient_);
	micro->SetDoubleAttribute("normalizedIR", alternativeIR_);

	TiXmlElement * baseTmplElement = new TiXmlElement( "baseMicroTemplateData" );
	micro->LinkEndChild(baseTmplElement); //link the base template element to the micro element
	m_templateBlock.WriteXMLBlock(baseTmplElement);

	TiXmlElement * NF_TmplElement = new TiXmlElement( "NF_TemplateData" );
	micro->LinkEndChild(NF_TmplElement); // link the NF template element to the micro element
	NF_TmplElement->SetAttribute("numberOfMUPsForNF_MUP_TmplCalc", m_numberOfMUPsForNF_MUP_TmplCalc);
	NF_TmplElement->SetAttribute("NF_MUP_TmplLength", NF_MUP_TmplLength_);
	NF_TmplElement->SetAttribute("NF_MUP_TmplOnsetPosition", NF_MUP_TmplOnsetPosition_);
	NF_TmplElement->SetAttribute("NF_MUP_TmplEndPosition", NF_MUP_TmplEndPosition_);
	NF_TmplElement->SetAttribute("numberOfMUPsForNF_JiggleCalc", m_numberOfMUPsForNF_MUP_TmplCalc);
	NF_TmplElement->SetDoubleAttribute("NF_Jiggle", m_NFmupJiggle);
	NF_TmplElement->SetDoubleAttribute("NF_JiggleMRE", m_NFmupJiggleMRE);
	NF_TmplElement->SetDoubleAttribute("NF_EnsembleRMS", NF_EnsembleRMS_);
	NF_TmplElement->SetDoubleAttribute("NF_MUP_TmplBaselineRMS", m_NF_MUP_TmplBaselineRMS);
	NF_TmplElement->SetDoubleAttribute("NF_JiggleThreshold", NF_Threshold_);
	NF_TmplElement->SetDoubleAttribute("NF_Area", NF_Area_);
	NF_TmplElement->SetDoubleAttribute("NF_Duration", NF_Duration_);

	TiXmlElement * valuesElement = new TiXmlElement( "NF_TemplateWaveform" );
	NF_TmplElement->LinkEndChild(valuesElement); // link the values element to the NF template element
	string temp = "";
	for(int i = 0; i < NF_MUP_TmplLength_; i++)
		temp += to_string(m_pNF_MUP_Tmpl[i]) + ",";
	valuesElement->SetAttribute("NF_TemplateWaveformValues", temp.c_str());

	return TRUE;
}

BOOL ASDMicroBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os	<< m_numberOfMUPsForNF_MUP_TmplCalc  << " " << m_numberOfMUPsForNF_MUP_TmplCalc << " "<< m_numberOfMUPsForJiggleCalc << " "
		<< NF_MUP_TmplLength_ << " " << m_numberOfSamplesForJiggleCalc << std::endl;
    os	<< m_NFmupJiggle << " " << Bjiggle_ << " " << jiggle_ << " "
		<< m_NFmupJiggleMRE << " " << NF_EnsembleRMS_ << " " << NF_Threshold_ << std::endl;

	os	<< areaAmplitudeRatio_ << " " << phases_ << " " << turns_ << std::endl;
	os  << irregularityCoefficient_ << " " << alternativeIR_ << std::endl;

	if (m_pNF_MUP_Tmpl) 
	{
		for (int i = 0; i < NF_MUP_TmplLength_; ++i)
			os << m_pNF_MUP_Tmpl[i] << " ";
		
		os << std::endl;
    }

	m_templateBlock.WriteBlock(os);

    return TRUE;
}

BOOL ASDMacroBlock::WriteXMLBlock(TiXmlElement * macro)
{
	macro->SetAttribute("macroDataBlock", "");
	macro->SetAttribute("validity", getValidityInteger());
	macro->SetDoubleAttribute("negPeakArea", m_negativePeakArea);
	macro->SetDoubleAttribute("negPeakDuration", m_negativePeakDuration);
	macro->SetDoubleAttribute("negPeakAmplitude", m_negativePeakAmplitude);
	macro->SetAttribute("peakOnset", m_negativePeakOnsetPosition);
	macro->SetAttribute("peakEnd", m_negativePeakEndPosition);
	macro->SetAttribute("peak", m_negativePeakPosition);
	macro->SetAttribute("trough", m_positivePeakPosition);

	TiXmlElement * baseTemplate = new TiXmlElement( "baseMacroTemplateData" );
	macro->LinkEndChild(baseTemplate); // link the base template element to the macro element
	m_templateBlock.WriteXMLBlock(baseTemplate);

	return TRUE;
}

BOOL ASDMacroBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os	<< m_negativePeakArea	<< " " << m_negativePeakDuration << " " << m_negativePeakAmplitude << std::endl;
    os	<< m_negativePeakOnsetPosition	<< " " << m_negativePeakEndPosition << " " << m_negativePeakPosition << " " << m_positivePeakPosition << std::endl;

    m_templateBlock.WriteBlock(os);

    return TRUE;
}

BOOL ASDTemplateBlock::WriteXMLBlock(TiXmlElement * baseTemplate)
{
	baseTemplate->SetAttribute("baseTemplateDataBlock", "");
	baseTemplate->SetAttribute("validity", getValidityInteger());
	baseTemplate->SetAttribute("numberInAverage", numberInAverage_);
	baseTemplate->SetAttribute("templateLength", templateLength_);
	baseTemplate->SetAttribute("samplingRate", samplingRate_);
	baseTemplate->SetDoubleAttribute("maxSlope", maxSlope_);
	baseTemplate->SetAttribute("indexOfMaxSlope", maxSlopeIndex_);
	baseTemplate->SetAttribute("onsetPosition", onsetPosition_);
	baseTemplate->SetAttribute("endPosition", endPosition_);
	baseTemplate->SetAttribute("negativePeakPosition", negativePeakPosition_);
	baseTemplate->SetAttribute("positivePeakPosition", positivePeakPosition_);
	baseTemplate->SetDoubleAttribute("duration", duration_);
	baseTemplate->SetDoubleAttribute("amplitude", amplitude_);
	baseTemplate->SetDoubleAttribute("area", area_);
	baseTemplate->SetDoubleAttribute("templateBaselineRMS", templateBaselineRMS_);

	TiXmlElement * valuesElement = new TiXmlElement( "baseTemplateWaveform" );
	baseTemplate->LinkEndChild(valuesElement); // link the values element tot the base template element
	string temp = "";
	for (int i = 0; i < templateLength_; ++i)
		temp += to_string(pTemplateWaveform_[i]) + ",";
	valuesElement->SetAttribute("baseTemplateWaveformValues", temp.c_str());
	delete pTemplateWaveform_;
	pTemplateWaveform_ = NULL;

	return TRUE;
}

BOOL ASDTemplateBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

    os	<< numberInAverage_ << " " << templateLength_ << std::endl;
	os	<< samplingRate_ << " " << maxSlope_ << " " << maxSlopeIndex_ << std::endl;
    
	for (int i = 0; i < templateLength_; ++i)
		os << pTemplateWaveform_[i] << " ";  
	
	// This is actually the scaled template pointer from the MUP_Template saveOn method
	delete pTemplateWaveform_;

    os	<< std::endl;

    os	<< onsetPosition_ << " " << endPosition_ << " " << negativePeakPosition_ << " " << positivePeakPosition_ << " "
		<< duration_ << " " << amplitude_ << " " << area_ << std::endl;

    return TRUE;
}

BOOL ASDFiringBlock::WriteXMLBlock(TiXmlElement * firing)
{
	firing->SetAttribute("firingDataBlock", "");
	firing->SetAttribute("validity", getValidityInteger());
	firing->SetDoubleAttribute("identificationRate", identificationRate_);
	firing->SetDoubleAttribute("meanIDI", IDImean_);
	firing->SetDoubleAttribute("IDIstd", IDIsd_);
	firing->SetDoubleAttribute("IDI_MACID", IDI_MACID_);
	firing->SetDoubleAttribute("IDI_MPACID", IDI_MPACID_);
	firing->SetDoubleAttribute("FR_MACID", FR_MACID_);
	firing->SetAttribute("numberOfIDIs", numberOfIDIs_);
	firing->SetAttribute("isValidFlag", bIsValid_);

	TiXmlElement * valuesElement = new TiXmlElement( "IDIs" );
	firing->LinkEndChild(valuesElement); // link the values element tot the base template element
	string temp = "";
	for (int i = 0; i < numberOfIDIs_; ++i)
		temp += to_string(pIDI_[i]) + ",";
	valuesElement->SetAttribute("IDI_Values", temp.c_str());

	return TRUE;
}

BOOL ASDFiringBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os	<< identificationRate_ << " " << IDImean_ << " " << IDIsd_ << " "
		<< IDI_MACID_ << " " << IDI_MPACID_ << " " << FR_MACID_ << " " << bIsValid_ << std::endl;
	
	return TRUE;
}


BOOL ASDEnsembleBlock::WriteXMLBlock(TiXmlElement * ensemble)
{
	ensemble->SetAttribute("ensembleDataBlock", "");
	ensemble->SetAttribute("validity", getValidityInteger());
	int m_trainNumber = m_pParentTrainBlock->m_trainNumber;
	ensemble->SetAttribute("trainNumber", m_trainNumber);
	ensemble->SetDoubleAttribute("shimmerCoV", shimmerCoV_);
	ensemble->SetAttribute("ensembleTmplLength", m_ensembleTmplLength);
	ensemble->SetAttribute("numberAvailableForReplacement", m_numberOfMupsAvailableForReplacement);
	ensemble->SetAttribute("numberSelected", m_numberOfMupsSelectedForEnsembleAnalysis);
	ensemble->SetAttribute("lastSelected", m_lastMupSelectedForEnsembleAnalysis);
	ensemble->SetAttribute("lastValid", m_lastValidMupSelectedForEnsembleAnalysis);
	ensemble->SetAttribute("fibreCount", fibreCount_);
	ensemble->SetDoubleAttribute("jiggle", jiggle_);
//	ensemble->SetDoubleAttribute("NFmupJiggle", m_NFmupJiggle);
	ensemble->SetDoubleAttribute("NFmupJitter", m_NFmupJitter);
	ensemble->SetDoubleAttribute("Bjiggle", Bjiggle_);
	ensemble->SetDoubleAttribute("meanTurnArea", meanTrnArea_);
	ensemble->SetDoubleAttribute("NF_MUP_Dispersion", NF_MupDispersion_);
	ensemble->SetDoubleAttribute("maxTurnInterval", maxTrnInterval_);

	TiXmlElement * SM_element = new TiXmlElement( "selectedMUPs" );
	string temp = "";
	for(int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; i++)
		temp += to_string(m_pSelectedMupNumber[i]) + ",";
	SM_element->SetAttribute("selectedMUP_Numbers", temp.c_str());
	// link the selected MUP number element to the ensemble element
	ensemble->LinkEndChild(SM_element); 

	TiXmlElement * shiftsElement = new TiXmlElement( "MUP_Shifts" );
	temp = "";
	for(int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; i++)
		temp += to_string(m_pShift[i]) + ",";
	shiftsElement->SetAttribute("shiftValues", temp.c_str());
	// link the shift element to the ensemble element
	ensemble->LinkEndChild(shiftsElement); 

	TiXmlElement * jitterDataElement = new TiXmlElement( "jitterData" );
	m_pJitterBlock->WriteXMLBlock(jitterDataElement);
	// link the jitter Data element to the ensemble element
	ensemble->LinkEndChild(jitterDataElement);

	return TRUE;
}

BOOL ASDEnsembleBlock::WriteBlock(std::ofstream& os) const
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os	<< m_pParentTrainBlock->m_trainNumber << " " << shimmerCoV_  << " " 
		<< m_ensembleTmplLength << " "
		<< m_numberOfMupsAvailableForReplacement << " " << m_numberOfMupsSelectedForEnsembleAnalysis << " " 
		<< m_lastMupSelectedForEnsembleAnalysis << " " << m_lastValidMupSelectedForEnsembleAnalysis << " "
		<< fibreCount_ << " " << jiggle_ << " "  
//		<< m_NFmupJiggle << " " << Bjiggle_ << std::endl;
		<< " " << Bjiggle_ << std::endl;
	
//	if(m_pParentTrainBlock->m_pParentASDIO->bEnsembleDataHasBeenProcessed_) 
//	{
		for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
			os << m_pSelectedMupNumber[i] << " ";
	
		os << std::endl;
	
		for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
			os << m_pShift[i] << " ";
	
		os << std::endl;

		m_pJitterBlock->WriteBlock(os);
//	}
	
    return TRUE;
}


BOOL ASDJitterBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os << fibreCount_ << " ";

	// ---------------- AccelTmplIndexOf ----------------
	for (int i = 0; i < fibreCount_; i++)
		os << m_pNF_MupTmplIndexOfMFP[i] << " ";
	os << std::endl;

	// ---------------- TurnFTCalculated ----------------
//	if (m_aIsTurnFTCalculated) 
//	{
//		os << TRUE << " ";	// indicate that TurnFTCalculated exists

//		for (int i = 0; i < fibreCount_; i++)
//			os << m_aIsTurnFTCalculated[i] << " ";
//			os << TRUE << " ";
//		os << std::endl;
//	} 
//	else
		os << FALSE << std::endl;	// indicate that TurnFTCalculated does not exist

	// ------------------ Array Sizes -------------------
	os << m_nJitterPairArraySize << " " << m_nNumberOfJitterPairs << " "
	   << m_nNumberOfTurnsForJitter << " " << m_nJitterTurnArraySize << std::endl;

	// --------------- Jitter Pair Blocks ---------------
	for (int i = 0; i < m_nNumberOfJitterPairs; i++)
		m_pJitterPairBlock[i].WriteBlock(os);

	// --------------- Jitter Turn Blocks ---------------
	for (int i = 0; i < m_nJitterTurnArraySize; i++)
		m_pJitterTurnBlock[i].WriteBlock(os);

	// ------------------- Peak Times -------------------
	for (int i = 0; i < m_nJitterTurnArraySize; i++)
		for (int j = 0; j < m_pParentEnsembleBlock->m_numberOfMupsSelectedForEnsembleAnalysis; j++)
			os << m_ppPeakTime[i][j] << " ";

	if(m_nJitterTurnArraySize > 0 && m_pParentEnsembleBlock->m_numberOfMupsSelectedForEnsembleAnalysis > 0)
		os << std::endl; // only do this if m_ppPeakTime was written

	return TRUE;
}

BOOL ASDJitterBlock::WriteXMLBlock(TiXmlElement * jitterBlockElement)const  
{
	jitterBlockElement->SetAttribute("jitterDataBlock", "");
	jitterBlockElement->SetAttribute("validity", getValidityInteger());
	jitterBlockElement->SetAttribute("fibreCount", fibreCount_);
	jitterBlockElement->SetAttribute("turnFiringTimesCalculated", FALSE);
	jitterBlockElement->SetAttribute("jitterPairArraySize", m_nJitterPairArraySize);
	jitterBlockElement->SetAttribute("numberOfJitterPairs", m_nNumberOfJitterPairs);
	jitterBlockElement->SetAttribute("jitterTurnArraySize", m_nJitterTurnArraySize);
	jitterBlockElement->SetAttribute("numberOfTurnsForJitter", m_nNumberOfTurnsForJitter);

	TiXmlElement * indiciesElement = new TiXmlElement( "NF_MFP_Indicies" );

	string temp = "";

	for (int i = 0; i < fibreCount_; ++i)
		temp += to_string(m_pNF_MupTmplIndexOfMFP[i]) + ",";
	indiciesElement->SetAttribute("NF_MFP_IndicieValues", temp.c_str());
	// link the NF MUP indicies element to the jitter element
	jitterBlockElement->LinkEndChild(indiciesElement); 


	CString out;
	TiXmlElement * element;
	// --------------- Jitter Pair Blocks ---------------
	for (int i = 0; i < m_nNumberOfJitterPairs; i++)
	{
//		out.Format("JitterPair%2d", i);
//		element = new TiXmlElement(out);
		element = new TiXmlElement("jitterPairData");
		element->SetAttribute("pairNumber", i);
		m_pJitterPairBlock[i].WriteXMLBlock(element);
		// link the ith jitter pair data element to the jitter data element
		jitterBlockElement->LinkEndChild(element);
	}


	// --------------- Jitter Turn Blocks ---------------
	for (int i = 0; i < m_nJitterTurnArraySize; i++)
	{
//		out.Format("JitterTurn%2d", i);
//		element = new TiXmlElement(out);
		element = new TiXmlElement("jitterTurnData");
		element->SetAttribute("turnNumber", i);
		m_pJitterTurnBlock[i].WriteXMLBlock(element);
		// link the ith jitter turn data element to the jitter element
		jitterBlockElement->LinkEndChild(element);
	}
	

	// ------------------- Peak Times -------------------
	TiXmlElement * peaksElement = new TiXmlElement("peakTimes");
	peaksElement->SetAttribute("numberOfPeaks", m_nJitterTurnArraySize);
	for (int i = 0; i < m_nJitterTurnArraySize; ++i)
	{
		element = new TiXmlElement("peakTimesFor");
		element->SetAttribute("peakNumber", i);
		temp = "";
		for (int j = 0; j < m_pParentEnsembleBlock->m_numberOfMupsSelectedForEnsembleAnalysis; j++)
			temp += to_string(m_ppPeakTime[i][j]) + ",";
		element->SetAttribute("peakTimeValues", temp.c_str());
		// link the ith peak times element to the peaks element
		peaksElement->LinkEndChild(element);
	}
	// link the peak element to the jitter element
	jitterBlockElement->LinkEndChild(peaksElement);

	return TRUE;
}

BOOL ASDJitterPairBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os << bIsValid_ << " " << fJitterMCD_ << " " << fMeanIPI_ << " "
	   << fPercentBlocking_ << " " << nFibreAIndex_ << " " << nFibreBIndex_ << std::endl;

	for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; i++)	
	{
		os	<< m_pFibreABlockedFlag[i] << " "	<< m_pFibreBBlockedFlag[i] << " "
			<< m_pFibreAPeakTime[i] << " "	<< m_pFibreBPeakTime[i] << " ";
	}

	os << std::endl;

	return TRUE;
}

BOOL ASDJitterPairBlock::WriteXMLBlock(TiXmlElement * jitterPairData) const 
{
	jitterPairData->SetAttribute("jitterPairDataBlock", "");
	jitterPairData->SetAttribute("validity", getValidityInteger());
	jitterPairData->SetAttribute("isValidFlag", bIsValid_);
	jitterPairData->SetDoubleAttribute("jitterMCD", fJitterMCD_);
	jitterPairData->SetDoubleAttribute("meanIPI", fMeanIPI_);
	jitterPairData->SetDoubleAttribute("percentBlocking", fPercentBlocking_);
	jitterPairData->SetAttribute("fibreAIndex", nFibreAIndex_);
	jitterPairData->SetAttribute("fibreBIndex", nFibreBIndex_);


	string temp = "";
	TiXmlElement *	element = new TiXmlElement("fibreA_BlockedFlags");
	for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
		temp += to_string(m_pFibreABlockedFlag[i]) + ",";
	element->SetAttribute("fibreA_BlockedFlagValues", temp.c_str());
	// link the fibre A Blocked Flags element to the jitter pair element
	jitterPairData->LinkEndChild(element);
	
	temp = "";
	element = new TiXmlElement("fibreB_BlockedFlags");
	for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
		temp += to_string(m_pFibreBBlockedFlag[i]) + ",";
	element->SetAttribute("fibreB_BlockedFlagValues", temp.c_str());
	// link the fibre B Blocked Flags element to the jitter pair element
	jitterPairData->LinkEndChild(element);

	temp = "";
	element = new TiXmlElement("fibreA_peakTimes");
	for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
		temp += to_string(m_pFibreAPeakTime[i]) + ",";
	element->SetAttribute("fibreA_PeakTimeValues", temp.c_str());
	// link the fibre A peak time element to the jitter pair element
	jitterPairData->LinkEndChild(element);
	
	temp = "";
	element = new TiXmlElement("fibreB_PeakTimes");
	for (int i = 0; i < TARGET_NUM_OF_JITTER_SAMPLES; ++i)
		temp += to_string(m_pFibreBPeakTime[i]) + ",";
	element->SetAttribute("fibreB_PeakTimeValues", temp.c_str());
	// link the fibre B peak time element to the jitter pair element
	jitterPairData->LinkEndChild(element);

	return TRUE;
}

BOOL ASDJitterTurnBlock::WriteBlock(std::ofstream& os) const 
{
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os << type_ << " " << time_ << " " << ampl_ << " " << rise_<< " " << fall_<< " "
		<< widthInSamples_ << " " << sharpness_ << " " << num_ << " " << index_ << std::endl;

	return TRUE;
}

BOOL ASDJitterTurnBlock::WriteXMLBlock(TiXmlElement * jitterTurnData) const 
{
	jitterTurnData->SetAttribute("jitterTurnDataBlock", "");
	jitterTurnData->SetAttribute("validity", getValidityInteger());
//	jitterTurnData->SetAttribute("isValidFlag", bIsValid_);
	jitterTurnData->SetAttribute("type", type_);
	jitterTurnData->SetDoubleAttribute("time", time_);
	jitterTurnData->SetDoubleAttribute("amplitude", ampl_);
	jitterTurnData->SetDoubleAttribute("rise", rise_);
	jitterTurnData->SetDoubleAttribute("fall", fall_);
	jitterTurnData->SetAttribute("widthInSamples", widthInSamples_);
	jitterTurnData->SetDoubleAttribute("sharpness", sharpness_);
	jitterTurnData->SetAttribute("jitterTurnNumber", num_);
	jitterTurnData->SetAttribute("index", index_);

	return TRUE;
}
