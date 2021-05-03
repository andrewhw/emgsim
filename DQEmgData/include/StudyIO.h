/*
* This class reads and writes .ASD files
*
* Created by Eric Hunsberger - Jan 09, 2009
*/

#if !defined(AFX_STUDYIO_H__7D63CF6E_03AF_4d44_BEE8_480ABD8971E9__INCLUDED_)
#define AFX_STUDYIO_H__7D63CF6E_03AF_4d44_BEE8_480ABD8971E9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define MAX_BUFFER 256

#include "TextIO.h"

using namespace std;

class BaseGraphGeometryBlock : public ASDBlock
{
public:
	int x1_; 
	int y1_;
	int x2_;
	int y2_;
	int width_;
	int height_;
	double sweep_;
	double range_;
	int xd_;
	int yd_;
	double xscale_;
	double yscale_;
	int isset_;

public:
	BaseGraphGeometryBlock() {
		x1_ = 0; 
		y1_ = 0;
		x2_ = 0;
		y2_ = 0;
		width_ = 0;
		height_ = 0;
		sweep_ = 1;
		range_ = 1;
		xd_ = 1;
		yd_ = 1;
		xscale_ = 1;
		yscale_ = 1;
		isset_ = 0;
	};

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
};

class BaseGraphBlock : public ASDBlock
{
public:
    int nFormats_;

	BaseGraphGeometryBlock m_geometry;
	BaseGraphGeometryBlock* m_aGeometries;

    long samplingRate_;
	int emgScale_;

public:
	BaseGraphBlock() 
	{
		m_correctPersistenceID = ID_BASEGRAPH;
		m_aGeometries = NULL;
		nFormats_ = 0;
		samplingRate_ = 0;
		emgScale_ = 0;
	}

	~BaseGraphBlock() 
	{
		if(m_aGeometries) delete[] m_aGeometries;
	}

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
};

class SmupBlock : public ASDBlock
{
public:
	int numberInAverage_;

	int m_nSamplesInTemplate;
	int* m_aTemplateWaveform;

	float area_;
	float m_negativePeakArea;
	float amplitude_;
	float negPeakDuration_;
	float duration_;
	float negPeakAmplitude_;
	int onset_;
	int peakOnset_;
	int peakEnd_;
	int m_negativePeakPosition;
	int m_positivePeakPosition;
	int end_;
    
	BaseGraphBlock m_baseGraph;

public:
	SmupBlock() 
	{
		m_correctPersistenceID = ID_SMUP;
		m_aTemplateWaveform = NULL;
		m_nSamplesInTemplate = 0;
	};

	~SmupBlock() {
		if(m_aTemplateWaveform) delete[] m_aTemplateWaveform;
	}

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;

	virtual void ReleasePointers() 
	{
		m_aTemplateWaveform = NULL;
		m_nSamplesInTemplate = 0;
	}
};

class CmapBlock : public ASDBlock
{
public:
	int numberInAverage_;

	int m_nSamplesInTemplate;
	int* m_aTemplateWaveform;

	float area_;
	float m_negativePeakArea;
	float amplitude_;
	float negPeakDuration_;
	float duration_;
	float negPeakAmplitude_;
	int onset_;
	int peakOnset_;
	int peakEnd_;
	int m_negativePeakPosition;
	int m_positivePeakPosition;
	int end_;
    
	BaseGraphBlock m_baseGraph;

public:
	CmapBlock() {
		m_correctPersistenceID = ID_CMAP;
		m_aTemplateWaveform = NULL;
		m_nSamplesInTemplate = 0;
	}

	~CmapBlock() {
		if(m_aTemplateWaveform) delete[] m_aTemplateWaveform;
	}

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;

	virtual void ReleasePointers() {
		m_aTemplateWaveform = NULL;
		m_nSamplesInTemplate = 0;
	}
};

class MuneSummaryBlock : public ASDBlock
{
public:
	string CMAPPeakToPeakAmpString_;
	string CMAPNegPeakAmpString_;
	string CMAPNegPeakAreaString_;
	float mSMUPAmplitude_;
	float mSMUPNegPeakAmplitude_;
	float mSMUPm_negativePeakArea;
	int MUNEPeakToPeakAmp_;
	int MUNENegPeakAmp_;
	int MUNEm_negativePeakArea;

	float GetCMAPPeakToPeakAmp() { return strToF(CMAPPeakToPeakAmpString_); }
	float GetCMAPNegPeakAmp() { return strToF(CMAPNegPeakAmpString_); }
	float GetCMAPNegPeakArea() { return strToF(CMAPNegPeakAreaString_); }

	SmupBlock m_mSmupMacroTemplate;
	CmapBlock m_CmapMacroTemplate;

	int nContractions_;
	int nSMUPs_;

public:
	MuneSummaryBlock() { m_correctPersistenceID = ID_MUNE; }
	~MuneSummaryBlock() {}

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;

	virtual void ReleasePointers() 
	{
		m_mSmupMacroTemplate.ReleasePointers();
		m_CmapMacroTemplate.ReleasePointers();
	}

};

class StudySummaryBlock : public ASDBlock
{
public:
	string vendorIdentifier_;
	string generalDescription_;

	string qemgStudyGUID_;
	struct {
		int year;
		int month;
		int day;
	} studyDate_;

	string operatorName_;
	string subjectName_;
	string muscleName_;

	struct {
		int year;
		int month;
		int day;
	} subjectDOB_;

	int age_;
	int gender_;
	int side_;

public:
	StudySummaryBlock() { 
		m_correctPersistenceID = ID_STUDY;
		vendorIdentifier_ = generalDescription_ = "";
		qemgStudyGUID_ = "";
		studyDate_.year = studyDate_.month = studyDate_.day = 1;
		subjectDOB_.year = subjectDOB_.month = subjectDOB_.day = 1;
		operatorName_ = subjectName_ = muscleName_ = "";
		age_ = gender_ = side_ = 0;
	}
	~StudySummaryBlock() {}

	virtual BOOL ReadBlock(std::ifstream& is);
	virtual BOOL WriteBlock(std::ofstream& os) const;
};

class StudyIO : public TextIO
{
public:
	int m_numContractions;
	string* m_aContractionFilePaths;

	StudySummaryBlock* m_pStudySummary;
	MuneSummaryBlock* m_pMuneSummary;

public:
	StudyIO() {
		m_pStudySummary = NULL;
		m_pMuneSummary = NULL;
		m_aContractionFilePaths = NULL;
	}

	~StudyIO() {
		if (m_pStudySummary) delete m_pStudySummary;
		if (m_pMuneSummary) delete m_pMuneSummary;
		if (m_aContractionFilePaths) delete[] m_aContractionFilePaths;
	}

	virtual BOOL readStudyInfoFile();
	virtual BOOL WriteFile();

	void ReleasePointers() 
	{
		//m_aContractionFilePaths = NULL;
		if (m_pStudySummary) m_pStudySummary->ReleasePointers();
		if (m_pMuneSummary) m_pMuneSummary->ReleasePointers();
	}
};

#endif // !defined(AFX_STUDYIO_H__7D63CF6E_03AF_4d44_BEE8_480ABD8971E9__INCLUDED_)
