/*
* This class reads and writes .ASD files
*
* Created by Eric Hunsberger - Jan 09, 2009
*/

#if !defined(AFX_ASDIO_H__1E5DECA9_6954_4b25_A14A_E302A628EDF2__INCLUDED_)
#define AFX_ASDIO_H__1E5DECA9_6954_4b25_A14A_E302A628EDF2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "TextIO.h"

class ASDTrainBlock;

//#include <istream>
//#include <ostream>
//#include <fstream>
//#include "afxwin.h" //CString class
//#include <string>

using namespace std;

class ASDio : public TextIO
{
public:
	int m_persistenceID;

	int nTrains_;
	BOOL bHaveMacro_;		// True if have macro-EMG data (MACRO??.DAT exists)
    BOOL bHaveMacro2_;		// True if have macro2-EMG data (MACR2??.DAT exists)
//	BOOL bEnsembleDataHasBeenProcessed_;	// flag to signal whether jitter info has been processed
	float fPercentMVC_RMS_;
	float fMaxIntensity_;
	float fAverageIntensity_;
	float fQuality_;
	float fEmgConversionFactor_;
	float fRecruitmentIndex_;
	float fFiringRateIndex_;

	ASDTrainBlock** m_ppTrainBlock;

public:
	ASDio();
	//ASDio(string targetFile);
	~ASDio();

	virtual BOOL readStudyInfoFile();
	BOOL ReadXMLFile();
	virtual BOOL WriteFile();
	BOOL WriteXMLFile();

	void ReleasePointers();

public:
	BOOL createTrainBlocks();
	void deleteTrainBlocks();
};

#endif // !defined(AFX_ASDIO_H__1E5DECA9_6954_4b25_A14A_E302A628EDF2__INCLUDED_)
