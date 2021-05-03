
#include "StudyIO.h"
#include <iostream>
#include <fstream>

void peekInt(std::ifstream &is, int &value) {
	int g = is.tellg();
	is >> value;
	is.seekg(g);
}

BOOL StudyIO::readStudyInfoFile() 
{
	if(m_sTargetFile == "") 
	{
		m_sError = "Target file string was null.";
		return FALSE;
	}

	std::ifstream is;
	try 
	{
		is.open(m_sTargetFile.c_str());

		if (is.bad()) 
		{
			m_sError = "Input file stream was bad.";
			is.close();
			return FALSE;
		}

		char strBuffer[MAX_BUFFER];

		is.getline(strBuffer, MAX_BUFFER);
		m_numContractions = atoi(strBuffer);

		m_aContractionFilePaths = new string[m_numContractions];

		for ( int i = 0; i < m_numContractions; i++ )
		{
			is.getline(strBuffer, MAX_BUFFER);
			m_aContractionFilePaths[i] = strBuffer;
		}
	
		if(m_pStudySummary) delete m_pStudySummary;
		m_pStudySummary = NULL;

		if(m_pMuneSummary) delete m_pMuneSummary;
		m_pMuneSummary = NULL;

		char peekChar = is.get();
		if ( !is.eof() )
		{
			is.putback(peekChar);

			int val;
			peekInt(is, val);

			if (val >= 100 || val < 0) 
			{
				// new file format
				m_pStudySummary = new StudySummaryBlock();
				if (!m_pStudySummary->ReadBlock(is)) 
				{
					delete m_pMuneSummary;
					m_pMuneSummary = NULL;

					is.close();
					return FALSE;
				}
			}

			m_pMuneSummary = new MuneSummaryBlock();
			
			if (!m_pMuneSummary->ReadBlock(is))
			{
				delete m_pMuneSummary;
				m_pMuneSummary = NULL;

				is.close();
				return FALSE;
			}
		}
		
		is.close();
		return TRUE;

	} 
	catch (CException* e) 
	{
		TCHAR msg[1023];
		e->GetErrorMessage(msg, 1023);
		m_sError = "Encountered unexpected error:\n" + *(new string(msg));	

		is.close();
		return FALSE;
	}
}

BOOL StudyIO::WriteFile() {
	if(m_sTargetFile == "") {
		m_sError = "Target file string was null.";
		return FALSE;
	}

	std::ofstream os;
	try {
		os.open(m_sTargetFile.c_str());

		if (os.bad()) {
			m_sError = "Output file stream was bad.";
			os.close();
			return FALSE;
		}

		os << m_numContractions << std::endl;
	    
		// Generate a list of constituent contractions
		for ( int i = 0; i < m_numContractions; i++ ) {
			os << m_aContractionFilePaths[i] << std::endl;
		}

		if ( m_pStudySummary && !m_pStudySummary->WriteBlock(os) ) {
			os.close();
			return FALSE;
		}
		
		if ( m_pMuneSummary && !m_pMuneSummary->WriteBlock(os) ) {
			os.close();
			return FALSE;
		}
	    
		os.close();
		return TRUE;

	} catch (CException* e) {
		TCHAR msg[1023];
		e->GetErrorMessage(msg, 1023);
		m_sError = "Encountered unexpected error:\n" + *(new string(msg));

		os.close();
		return FALSE;
	}
}

// -----------------------------------------------------------------------------------------
BOOL StudySummaryBlock::ReadBlock(std::ifstream& is) {	
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) { return FALSE; }

	string remaining;
	getline(is, remaining); // get the rest of the persistenceID line

	getline(is, vendorIdentifier_);
	getline(is, generalDescription_);
	getline(is, operatorName_);
	getline(is, subjectName_);
	getline(is, muscleName_);
	getline(is, qemgStudyGUID_);
	is	>> studyDate_.year >> studyDate_.month >> studyDate_.day;
	is	>> subjectDOB_.year >> subjectDOB_.month >> subjectDOB_.day;
	is	>> gender_ >> age_ >> side_;

    return TRUE;
}

BOOL StudySummaryBlock::WriteBlock(std::ofstream& os) const {
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	os	<< vendorIdentifier_ << std::endl;
	os	<< generalDescription_ << std::endl;
	os	<< operatorName_ << std::endl;
	os	<< subjectName_ << std::endl;
	os	<< muscleName_ << std::endl;
	os	<< qemgStudyGUID_ << std::endl;
	os	<< studyDate_.year << " " << studyDate_.month << " " << studyDate_.day << std::endl;
	os	<< subjectDOB_.year << " " << subjectDOB_.month << " " << subjectDOB_.day << std::endl;
	os	<< gender_ << " " << age_ << " " << side_ << std::endl;
	os	<< std::endl;

    return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL MuneSummaryBlock::ReadBlock(std::ifstream& is) {
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok() && m_receivedPersistenceID != 9) { return FALSE; }
	int version = abs(m_receivedPersistenceID) / 100;

	if (version > 0)
		is >> nContractions_ >> nSMUPs_;

	string sAmplitude, sNegPkAmp, sNegPkArea;
	is >> CMAPPeakToPeakAmpString_ >> CMAPNegPeakAmpString_ >> CMAPNegPeakAreaString_;
	is >> sAmplitude >> sNegPkAmp >> sNegPkArea;
	is >> MUNEPeakToPeakAmp_ >> MUNENegPeakAmp_ >> MUNEm_negativePeakArea;

	mSMUPAmplitude_ = strToF(sAmplitude);
	mSMUPNegPeakAmplitude_ = strToF(sNegPkAmp);
	mSMUPm_negativePeakArea = strToF(sNegPkArea);

    m_mSmupMacroTemplate.ReadBlock(is);
    m_CmapMacroTemplate.ReadBlock(is);

    if (version == 0)
		is >> nContractions_ >> nSMUPs_;
    
    return TRUE;
}

BOOL MuneSummaryBlock::WriteBlock(std::ofstream& os) const {
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

    os << nContractions_ << " " << nSMUPs_ << std::endl;

    os << CMAPPeakToPeakAmpString_ << std::endl 
	   << CMAPNegPeakAmpString_ << std::endl 
	   << CMAPNegPeakAreaString_ << std::endl;
    os << mSMUPAmplitude_ << std::endl 
	   << mSMUPNegPeakAmplitude_ << std::endl 
	   << mSMUPm_negativePeakArea << std::endl;
    os << MUNEPeakToPeakAmp_ << std::endl 
	   << MUNENegPeakAmp_ << std::endl 
	   << MUNEm_negativePeakArea << std::endl;

    m_mSmupMacroTemplate.WriteBlock(os);
	m_CmapMacroTemplate.WriteBlock(os);

    return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL SmupBlock::ReadBlock(std::ifstream& is) {
	is >> m_receivedPersistenceID;

	int version;
	if (abs(m_receivedPersistenceID) == ID_MACROTMPL)
		version = 0;
	else if (isPersistenceIDok()) 
		version = abs(m_receivedPersistenceID) / 100;
	else 
	{ 
		return FALSE; 
	}
    
	is >> numberInAverage_ >> m_nSamplesInTemplate;
    
	if(m_aTemplateWaveform) 
		delete[] m_aTemplateWaveform;
	m_aTemplateWaveform = new int[m_nSamplesInTemplate];

    for (int i = 0; i < m_nSamplesInTemplate; i++)
		is >> m_aTemplateWaveform[i];
    
	string sArea, sNegPkArea, sAmplitude, sNegPkDur, sDur, sNegPkAmp;
	is >> sArea >> sNegPkArea >> sAmplitude >> sNegPkDur >> sDur >> sNegPkAmp;
	area_ = strToF(sArea);
	amplitude_ = strToF(sAmplitude);
	duration_ = strToF(sDur);
	m_negativePeakArea = strToF(sNegPkArea);
	negPeakAmplitude_ = strToF(sNegPkAmp);
	negPeakDuration_ = strToF(sNegPkDur);

    is >> onset_ >> peakOnset_ >> peakEnd_ >> m_negativePeakPosition >> m_positivePeakPosition >> end_;

    if (version == 0)
		m_baseGraph.ReadBlock(is);

	return TRUE;
}

BOOL SmupBlock::WriteBlock(std::ofstream& os) const {
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

    os << numberInAverage_ << " " << m_nSamplesInTemplate << std::endl;

	if (m_nSamplesInTemplate > 0) 
	{
		for (int i = 0; i < m_nSamplesInTemplate; i++)
			os << m_aTemplateWaveform[i] << " ";
		os << std::endl;
	}

    os << area_ << " " << m_negativePeakArea << " " << amplitude_ << " "
       << negPeakDuration_ << " " << duration_ << " " << negPeakAmplitude_ << " " << std::endl;
    os << onset_ << " " << peakOnset_ << " " << peakEnd_ << " " << m_negativePeakPosition << " " << m_positivePeakPosition
       << " " << end_ << std::endl;
	os << std::endl;

    return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL CmapBlock::ReadBlock(std::ifstream& is) {
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) { return FALSE; }
	int version = abs(m_receivedPersistenceID) / 100;

	is >> numberInAverage_ >> m_nSamplesInTemplate;

	if(m_aTemplateWaveform) delete[] m_aTemplateWaveform;
   	m_aTemplateWaveform = new int[m_nSamplesInTemplate];

    for (int i = 0; i < m_nSamplesInTemplate; i++)
		is >> m_aTemplateWaveform[i];

	string sArea, sNegPkArea, sAmplitude, sNegPkDur, sDur, sNegPkAmp;
	is >> sArea >> sNegPkArea >> sAmplitude >> sNegPkDur >> sDur >> sNegPkAmp;
	area_ = strToF(sArea);
	amplitude_ = strToF(sAmplitude);
	duration_ = strToF(sDur);
	m_negativePeakArea = strToF(sNegPkArea);
	negPeakAmplitude_ = strToF(sNegPkAmp);
	negPeakDuration_ = strToF(sNegPkDur);

    is >> onset_ >> peakOnset_ >> peakEnd_ >> m_negativePeakPosition >> m_positivePeakPosition >> end_;

    if (version <= 1)
		m_baseGraph.ReadBlock(is);
    
	return TRUE;
}

BOOL CmapBlock::WriteBlock(std::ofstream& os) const {
		int version = abs(m_receivedPersistenceID) / 100;

		os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

    os << numberInAverage_ << " " << m_nSamplesInTemplate << std::endl;

	if (m_nSamplesInTemplate > 0) {
		for (int i = 0; i < m_nSamplesInTemplate; i++)
			os << m_aTemplateWaveform[i] << " ";	
		os << std::endl;
	}

    os << area_ << " " << m_negativePeakArea << " " << amplitude_ << " "
       << negPeakDuration_ << " " << duration_ << " " << negPeakAmplitude_ << " " << std::endl;
    os << onset_ << " " << peakOnset_ << " " << peakEnd_ << " " << m_negativePeakPosition << " " << m_positivePeakPosition
       << " " << end_ << std::endl;
	os << std::endl;
    
    if (version <= 1)
		m_baseGraph.WriteBlock(os);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL BaseGraphBlock::ReadBlock(std::ifstream& is) {
	is >> m_receivedPersistenceID;
	if (!isPersistenceIDok()) { return FALSE; }

	m_geometry.ReadBlock(is);

    int nFormats_;
    is >> nFormats_;

	if(m_aGeometries != NULL) delete[] m_aGeometries;
	m_aGeometries = new BaseGraphGeometryBlock[nFormats_]();

    for (int i = 0; i < nFormats_; i++)
		m_aGeometries[i].ReadBlock(is);

    is >> samplingRate_ >> emgScale_;
    
    return TRUE;
}

BOOL BaseGraphBlock::WriteBlock(std::ofstream& os) const {
	os	<< getValidityInteger() * m_correctPersistenceID << std::endl;

	m_geometry.WriteBlock(os);

    os << nFormats_ << std::endl;

    for (int i = 0; i < nFormats_; i++)
		m_aGeometries[i].WriteBlock(os);
    
    os << samplingRate_ << " " << emgScale_ << std::endl;

    return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL BaseGraphGeometryBlock::ReadBlock(std::ifstream& is) {
    is	>> x1_ >> y1_ >> x2_ >> y2_
		>> width_ >> height_ >> sweep_ >> range_
		>> xd_ >> yd_ >> xscale_ >> yscale_ >> isset_;

	return TRUE;
}

BOOL BaseGraphGeometryBlock::WriteBlock(std::ofstream& os) const {
    os	<< x1_ << " " << y1_ << " " << x2_ << " "<< y2_ << std::endl
		<< width_ << " " << height_ << " " << sweep_ << " " << range_ << std::endl
		<< xd_ << " " << yd_ << " " << xscale_ << " " << yscale_ << " " << isset_ << std::endl;

    return TRUE;
}
