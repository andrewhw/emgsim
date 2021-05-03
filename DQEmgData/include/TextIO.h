/*
* 
*
* Created by Eric Hunsberger - Jan 09, 2009
*/

#if !defined(AFX_TEXTIO_H__9D8556B4_91E7_4b49_8300_BA1A2C642834__INCLUDED_)
#define AFX_TEXTIO_H__9D8556B4_91E7_4b49_8300_BA1A2C642834__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <istream>
#include <ostream>
#include <fstream>
#include "afxwin.h" //CString class
#include <string>

#include "math.h"
#include <limits>

#include "persistenceIDs.h"

using namespace std;

static const string S_INDETERMINITE = "1.#IND";
static const string S_QUIET_NAN = "1.#QNAN";
static const string S_INFINITY = "1.#INF";
static const string S_UNKNOWN = "unknown";

static const float F_QUIET_NAN = std::numeric_limits<float>::quiet_NaN();
static const float F_INFINITY = std::numeric_limits<float>::infinity();
static const float F_NEG_INFINITY = -std::numeric_limits<float>::infinity();

class TextIO 
{
protected:
	string m_sXMLTargetFile;
	string m_sTargetFile;
	string m_sError; //Stores message for any error encountered when reading the file

public:
	TextIO() {
		m_sError = "";
		m_sTargetFile = "";
	}

	virtual BOOL readStudyInfoFile() {return FALSE;}
	virtual BOOL WriteFile() {return FALSE;}

public:
	//Accessors and Mutators
	string getTargetFile() {return m_sTargetFile;}
	string getError() {return m_sError;}

	void setTargetFile(string file){m_sTargetFile = file;}
	void setXMLTargetFile(string file){m_sXMLTargetFile = file;}

};

class ASDBlock 
{
public:
	int m_correctPersistenceID;
	int m_receivedPersistenceID;
	CString m_sError;

public:
	ASDBlock();

	virtual BOOL ReadBlock(std::ifstream& is) = 0;
	virtual BOOL WriteBlock(std::ofstream& os) const = 0;
	
	virtual void ReleasePointers() {};

	CString getError() {return m_sError;}
	void clearError() {m_sError = "";}

	BOOL isPersistenceIDok();
	BOOL isValid() const;
	int getReceivedPersistenceID()	{ return m_receivedPersistenceID;}
	int getValidityInteger() const;
	void setValid(BOOL valid);
};

float strToF(string s);

inline int atoi(string str) { 
	return atoi(str.c_str()); 
}

inline void ReadThroughWhiteSpace(ifstream& is) {
	char peekChar;
	do {
		peekChar = is.get();
	} while (peekChar == ' ' || peekChar == '\t');
	is.putback(peekChar);
}

#endif // !defined(AFX_TEXTIO_H__9D8556B4_91E7_4b49_8300_BA1A2C642834__INCLUDED_)
