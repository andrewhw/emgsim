/*
* These classes represent the various sections (i.e., "blocks") in an ASD file. They are each able to read and write their corresponding block.
*
* Created by Eric Hunsberger - Jan 09, 2009
*/

#if !defined(AFX_ASDBlock_H__9E1E97BE_E327_4e46_BC22_053A3CF677C7__INCLUDED_)
#define AFX_ASDBlock_H__9E1E97BE_E327_4e46_BC22_053A3CF677C7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <iostream>
#include <fstream>
#include <string>
#include "afxwin.h" //CString class

#include "ASDIO.h"
#include "tinyxml.h"
//#include "Jitter.h"

#include <string>
#include <sstream>

#define TARGET_NUM_OF_JITTER_SAMPLES 51

class ASDMicroBlock;
class ASDMacroBlock;
class ASDFiringBlock;
class ASDEnsembleBlock;
class ASDJitterBlock;
class ASDJitterPairBlock;
class ASDJitterTurnBlock;

template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}
//----------------------------------------------------------------------------

class ASDTrainBlock : public ASDBlock
{
public:
	int m_trainNumber;
	int numberOfMUPs;
//	BOOL bHaveMacro_;
//	BOOL bHaveMacro2_;
	float muActivityDuration_;
//	int spikeTemplateStartPosition_;
//	int spikeTemplateEndPosition_;

	int* m_pConsecutiveDistanceIDI;
	float* m_pConsecutiveDistance;

	ASDMicroBlock* m_pMicroBlock;
	ASDMacroBlock* m_pMacroBlock;
	ASDFiringBlock* m_pFiringBlock;
	ASDEnsembleBlock* m_pEnsembleBlock;

	ASDio* m_pParentASDIO;
	
public:
	ASDTrainBlock(ASDio* asdio);
	~ASDTrainBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	BOOL ASDTrainBlock::ReadXMLBlock(TiXmlNode * trainNode);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL WriteXMLBlock(TiXmlElement * train);
	BOOL ReadXMLBlock(TiXmlElement * train);
	virtual void ReleasePointers();
};

class ASDEnsembleBlock : public ASDBlock 
{
public:

	int		m_ensembleTmplLength;
	int		m_numberOfMupsAvailableForReplacement;
	int		m_numberOfMupsSelectedForEnsembleAnalysis;
	int		m_lastMupSelectedForEnsembleAnalysis;
	int		m_lastValidMupSelectedForEnsembleAnalysis;

	int		fibreCount_;
	float	shimmerCoV_;
	float	jiggle_;
//	float	m_NFmupJiggle;
	float	m_NFmupJitter;
	float	Bjiggle_;
	float	maxTrnInterval_;
	float	NF_MupDispersion_;
	float	meanTrnArea_;

	int*	m_pSelectedMupNumber;
	int*	m_pShift;
	ASDJitterBlock* m_pJitterBlock;

	ASDTrainBlock* m_pParentTrainBlock;

public:
	ASDEnsembleBlock(ASDTrainBlock* pTrainBlock);
	~ASDEnsembleBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL WriteXMLBlock(TiXmlElement * ensemble);
	BOOL ReadXMLBlock(TiXmlElement * ensemble);
	virtual void ReleasePointers();
};

class ASDTemplateBlock : public ASDBlock
{
public:
	int numberInAverage_;
	int templateLength_;
	int samplingRate_;

	float maxSlope_; 
    int maxSlopeIndex_; 

    int* pTemplateWaveform_;

    int onsetPosition_;
	int endPosition_;
	int negativePeakPosition_;
	int positivePeakPosition_;

	float duration_;
	float amplitude_;
	float area_;
	float irregularityCoefficient_;
	float alternativeIR_;
	float templateBaselineRMS_;

public:
	ASDTemplateBlock();
	~ASDTemplateBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL WriteXMLBlock(TiXmlElement * baseTemplate);
	BOOL ReadXMLBlock(TiXmlElement * baseTemplate);
	virtual void ReleasePointers();
};

class ASDMicroBlock : public ASDBlock
{
public:
	int		m_numberOfMUPsForNF_MUP_TmplCalc;
	int		m_numberOfMUPsForNFmupJiggleCalc;
	int		m_numberOfMUPsForJiggleCalc;
	int		m_numberOfSamplesForJiggleCalc;

	float	m_NFmupJiggle;
//	float	m_NFmupJitter;
	float	Bjiggle_; 
	float	jiggle_;
	float	m_NFmupJiggleMRE; 
	float	NF_EnsembleRMS_;
	float	m_NF_MUP_TmplBaselineRMS;
	float	NF_Threshold_;

    float	areaAmplitudeRatio_; 
    float	shapeWidth_; 
	int		phases_;
	int		turns_;
	float	irregularityCoefficient_;
	float	alternativeIR_;

	float	NF_Area_;
	float	NF_Duration_;

	int *	m_pNF_MUP_Tmpl;
	int		NF_MUP_TmplLength_;
	int		NF_MUP_TmplOnsetPosition_;
	int		NF_MUP_TmplEndPosition_;

	ASDTemplateBlock m_templateBlock;

	ASDTrainBlock* m_pParentTrainBlock;

public:
	ASDMicroBlock(ASDTrainBlock* pTrainBlock);
	~ASDMicroBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL WriteXMLBlock(TiXmlElement * micro);
	BOOL ReadXMLBlock(TiXmlElement * micro);
	virtual void ReleasePointers();
};

class ASDMacroBlock : public ASDBlock
{
public:
	float m_negativePeakArea;
	float m_negativePeakDuration;
	float m_negativePeakAmplitude;
    int m_negativePeakOnsetPosition;
	int m_negativePeakEndPosition;
	int m_negativePeakPosition;
	int m_positivePeakPosition;

	ASDTemplateBlock m_templateBlock;

	ASDTrainBlock* m_pParentTrainBlock;

public:
	ASDMacroBlock(ASDTrainBlock* pTrainBlock);

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL WriteXMLBlock(TiXmlElement * macro);
	BOOL ReadXMLBlock(TiXmlElement * macro);
	virtual void ReleasePointers();
};

class ASDFiringBlock : public ASDBlock
{
public:
	float IDImean_;
	float IDIsd_;

	float identificationRate_;
	float IDI_MACID_;
	float IDI_MPACID_;
	float FR_MACID_;
	BOOL bIsValid_;

	int numberOfIDIs_;
	float * pIDI_;

	ASDTrainBlock* m_pParentTrainBlock;

public:
	ASDFiringBlock(ASDTrainBlock* pTrainBlock);

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL WriteXMLBlock(TiXmlElement * firing);
	BOOL ReadXMLBlock(TiXmlElement * firing);
	virtual void ReleasePointers();
};

class ASDJitterBlock : public ASDBlock 
{
public:
	int fibreCount_;

	int m_nJitterPairArraySize;
	int m_nNumberOfJitterPairs;
	int m_nNumberOfTurnsForJitter;
	int m_nJitterTurnArraySize;

	int* m_pNF_MupTmplIndexOfMFP;
//	BOOL* m_pIsTurnFTCalculated;
	ASDJitterPairBlock* m_pJitterPairBlock;
	ASDJitterTurnBlock* m_pJitterTurnBlock;
	//float* m_rasterMeanAmplOfTurn;
	//float* m_rasterSumAmplOfTurn;
	float** m_ppPeakTime;

	ASDEnsembleBlock* m_pParentEnsembleBlock;

public:
	ASDJitterBlock(ASDEnsembleBlock* ensembleBlock);
	~ASDJitterBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	BOOL ReadXMLBlock(TiXmlElement * jitterData);
	BOOL WriteXMLBlock(TiXmlElement * jitterData) const;
	virtual void ReleasePointers();

	void deleteJitterPairBlocks();
	BOOL createJitterPairBlocks();
	void deleteJitterTurnBlocks();
	BOOL createJitterTurnBlocks();
};

class ASDJitterTurnBlock : public ASDBlock 
{
public:
	enum  TurnType { Negative, Positive};

	TurnType type_;
	float time_;
	float ampl_;
	float rise_;
	float fall_;
	int widthInSamples_;
	float sharpness_;
	int num_;
	int index_;

public:
	ASDJitterTurnBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	virtual BOOL ReadXMLBlock(TiXmlElement * jitterTurnData);
	virtual BOOL WriteXMLBlock(TiXmlElement * jitterTurnData) const;
	virtual void ReleasePointers();
};

class ASDJitterPairBlock : public ASDBlock 
{
public:
	BOOL bIsValid_;
	float fJitterMCD_;
	float fMeanIPI_;
	float fPercentBlocking_;
	int nFibreAIndex_;
	int nFibreBIndex_;

	BOOL* m_pFibreABlockedFlag;
	BOOL* m_pFibreBBlockedFlag;
	float* m_pFibreAPeakTime;
	float* m_pFibreBPeakTime;

public:
	ASDJitterPairBlock();
	~ASDJitterPairBlock();

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
	virtual BOOL ReadXMLBlock(TiXmlElement * jitterPairData);
	virtual BOOL WriteXMLBlock(TiXmlElement * jitterPairData) const;
	virtual void ReleasePointers();
};


#endif //!defined(AFX_ASDBlock_H__9E1E97BE_E327_4e46_BC22_053A3CF677C7__INCLUDED_)
