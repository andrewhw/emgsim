/*
* Implementation of ASDio class.
*
* Created by Eric Hunsberger - Jan 09, 2009
*/

//#include "afx.h"
#include <iostream>
#include <sstream>
#include <fstream>

#include "ASDio.h"
#include "ASDBlocks.h"
#include "tinyxml.h"

#define MAX_TRAINS 20000
#define MAX_BUFFER 1023
#define VARIABLE_TO_STRING(val) #val

using namespace std;

ASDio::ASDio() : TextIO() {
	m_ppTrainBlock = NULL;
	fMaxIntensity_ = F_QUIET_NAN;
	fAverageIntensity_ = F_QUIET_NAN;
	fQuality_ = F_QUIET_NAN;
}

ASDio::~ASDio() 
{
	deleteTrainBlocks();
}

BOOL ASDio::ReadXMLFile()
{
	if(m_sXMLTargetFile == "") 
	{
		m_sError = "XML target read file string was null.";
		return FALSE;
	}
//	TiXmlDocument doc(fName.c_str());
	TiXmlDocument doc(m_sXMLTargetFile.c_str());
	BOOL loadSuccessful = doc.LoadFile();
	if (loadSuccessful)
	{
		TiXmlNode * docNode = &doc;
		TiXmlNode * contractionNode;
		contractionNode = docNode->FirstChild();
		contractionNode = contractionNode->NextSibling();
		TiXmlElement * contractionElement = contractionNode->ToElement();
		TiXmlAttribute * contractionAttribute = contractionElement->FirstAttribute();
		while (contractionAttribute)
		{
			double temp = 0;
			const char* attributeName = contractionAttribute->Name();
			if (strcmp(attributeName, VARIABLE_TO_STRING(nTrains_)) == 0)
			{
				contractionAttribute->QueryIntValue(&nTrains_);
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(bHaveMacro_)) == 0)
			{
				contractionAttribute->QueryIntValue(&bHaveMacro_);
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(bHaveMacro2_)) == 0)
			{
				contractionAttribute->QueryIntValue(&bHaveMacro2_);
			} 
//			else if (strcmp(attributeName, VARIABLE_TO_STRING(bEnsembleDataHasBeenProcessed_)) == 0)
//			{
//				contractionAttribute->QueryIntValue(&bEnsembleDataHasBeenProcessed_);
//			} 
			else if (strcmp(attributeName, VARIABLE_TO_STRING(fPercentMVC_RMS_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fPercentMVC_RMS_ = (float) temp;
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(fRecruitmentIndex_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fRecruitmentIndex_ = (float) temp;
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(fFiringRateIndex_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fFiringRateIndex_ = (float) temp;
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(fMaxIntensity_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fMaxIntensity_ = (float) temp;
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(fAverageIntensity_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fAverageIntensity_ = (float) temp;
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(fQuality_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fQuality_ = (float) temp;
			} else if (strcmp(attributeName, VARIABLE_TO_STRING(fEmgConversionFactor_)) == 0)
			{
				contractionAttribute->QueryDoubleValue(&temp);
				fEmgConversionFactor_ = (float) temp;
			}
			contractionAttribute = contractionAttribute->Next();
		}

		if(!createTrainBlocks()) 
			return FALSE;

		TiXmlElement * trainElement;
		int trainNum = 0;
		trainElement = contractionNode->FirstChildElement();
		while(trainElement)
		{
			m_ppTrainBlock[trainNum]->ReadXMLBlock(trainElement);
			trainNum++;
			trainElement = trainElement->NextSiblingElement();
		}
		
/*
		for (trainNode = contractionNode->FirstChild(); trainNode != 0; trainNode = trainNode->NextSibling()) 
		{
			m_ppTrainBlock[trainNum]->ReadXMLBlock(trainNode);
			trainNum += 1;
		}
*/

		return TRUE;
	}
	else
	{
		// load was unsuccsessful
		return FALSE;
	}
}

BOOL ASDio::readStudyInfoFile() 
{
	if(m_sTargetFile == "") 
	{
		m_sError = "Study Info read file string was null.";
		return FALSE;
	}

	std::ifstream is;
	try 
	{
		is.open(m_sTargetFile.c_str());

		if (is.bad()) 
		{
			m_sError = "Input file stream was bad.";
			return FALSE;
		}

		is >> m_persistenceID;
		if (m_persistenceID != ID_EMGANALYSIS) 
		{
			//m_sError.Format("Persistence ID incorrect for EMG block. Expected %d and received %d.", ID_EMGANALYSIS, m_persistenceID);
			return FALSE;
	    }

		is	>> nTrains_ >> bHaveMacro_ >> bHaveMacro2_ 
//			>> bEnsembleDataHasBeenProcessed_
			>> fPercentMVC_RMS_ >> fRecruitmentIndex_ >> fFiringRateIndex_;

		ReadThroughWhiteSpace(is);
		if(is.peek() != '\n') 
		{
			is >> fMaxIntensity_ >> fAverageIntensity_ >> fQuality_;
		}
		
		if(!createTrainBlocks()) 
		{
			is.close();
			return FALSE;
		}

		for (int trainNum = 0; trainNum < nTrains_; ++trainNum)
		{
			if (!m_ppTrainBlock[trainNum]->ReadBlock(is))
			{
				m_sError = m_ppTrainBlock[trainNum]->getError();
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

BOOL ASDio::WriteXMLFile()
{
	if(m_sXMLTargetFile == "") 
	{
		m_sError = "XML target write file string was null.";
		return FALSE;
	}

	TiXmlDocument doc;
 	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild( decl );
 
	TiXmlElement * contraction = new TiXmlElement("Contraction");
	doc.LinkEndChild(contraction);
	contraction->SetAttribute(VARIABLE_TO_STRING(nTrains_), nTrains_);
	contraction->SetAttribute(VARIABLE_TO_STRING(bHaveMacro_), bHaveMacro_);
	contraction->SetAttribute(VARIABLE_TO_STRING(bHaveMacro2_), bHaveMacro2_);
//	contraction->SetAttribute(VARIABLE_TO_STRING(bEnsembleDataHasBeenProcessed_), bEnsembleDataHasBeenProcessed_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fPercentMVC_RMS_), fPercentMVC_RMS_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fRecruitmentIndex_), fRecruitmentIndex_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fFiringRateIndex_), fFiringRateIndex_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fMaxIntensity_), fMaxIntensity_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fAverageIntensity_), fAverageIntensity_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fQuality_), fQuality_);
	contraction->SetDoubleAttribute(VARIABLE_TO_STRING(fEmgConversionFactor_), fEmgConversionFactor_);

	for (int i = 0; i < nTrains_; ++i) 
	{
		TiXmlElement * train = new TiXmlElement( "MUPTrainData" );
		m_ppTrainBlock[i]->WriteXMLBlock(train);
		contraction->LinkEndChild(train);
	}

//	doc.SaveFile( fName.c_str() );
	BOOL bSussessfulSave = doc.SaveFile(m_sXMLTargetFile.c_str());
	return bSussessfulSave;
//	return TRUE;
}

BOOL ASDio::WriteFile() 
{
	if(m_sTargetFile == "") 
	{
		m_sError = "Write target file string was null.";
		return FALSE;
	}

	std::ofstream os;
	try 
	{
		os.open(m_sTargetFile.c_str());

		if (os.bad()) 
		{
			m_sError = "Output file stream was bad.";
			os.close();
			return FALSE;
		}

		os	<< ID_EMGANALYSIS << std::endl;
	  
		os	<< nTrains_ << " " << bHaveMacro_ << " " << bHaveMacro2_<< " " 
//			<< bEnsembleDataHasBeenProcessed_ << " " 
			<< fPercentMVC_RMS_ << " " << fRecruitmentIndex_ << " " << fFiringRateIndex_ << " " 
			<< fMaxIntensity_ << " " << fAverageIntensity_ << " " << fQuality_ << std::endl;

		// Write the associated ASD blocks to the ADD file
		for (int i = 0; i < nTrains_; ++i) 
		{
			m_ppTrainBlock[i]->WriteBlock(os);
		}

		os.close();
		return TRUE;

	} 
	catch (CException* e) 
	{
		TCHAR msg[1023];
		e->GetErrorMessage(msg, 1023);
		m_sError = "Encountered unexpected error:\n" + *(new string(msg));

		os.close();
		return FALSE;
	}
}

BOOL ASDio::createTrainBlocks() 
{
	deleteTrainBlocks(); // ensure any old train blocks are deleted first

	if(nTrains_ <= 0 || nTrains_ > MAX_TRAINS) 
	{
		//m_sError.Format("Invalid number of trains: must be greater than %d and less than %d", 0, MAX_TRAINS);
		return FALSE;
	}

	m_ppTrainBlock = new ASDTrainBlock* [nTrains_];
    for (int i = 0; i < nTrains_; i++)
		m_ppTrainBlock[i] = new ASDTrainBlock(this);

	return TRUE;
}

void ASDio::deleteTrainBlocks()
{
	if (m_ppTrainBlock)
	{
		for (int i = 0; i < nTrains_; i++){
			if (m_ppTrainBlock[i])
				delete m_ppTrainBlock[i];
		}
		delete [] m_ppTrainBlock;
		m_ppTrainBlock = NULL;
    }
}

void ASDio::ReleasePointers() 
{
	for (int i = 0; i < nTrains_; i++) 
	{
		if(m_ppTrainBlock[i])
			m_ppTrainBlock[i]->ReleasePointers();
	}
}
