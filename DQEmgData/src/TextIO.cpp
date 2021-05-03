
#include "TextIO.h"

ASDBlock::ASDBlock(){
	m_correctPersistenceID = 1;
	m_receivedPersistenceID = 0;
}

BOOL ASDBlock::isPersistenceIDok() {
	if (abs(m_receivedPersistenceID) % 100 != m_correctPersistenceID % 100) {
		m_sError.Format("Incorrect persistence ID. Was expecting %d and received %d", m_correctPersistenceID, m_receivedPersistenceID);
		return FALSE;
	} else
		return TRUE;
}

BOOL ASDBlock::isValid() const {
	return (BOOL)(m_receivedPersistenceID >= 0);
}

BOOL ASDBlock::getValidityInteger() const {
	if(isValid())
		return 1;
	else
		return -1;
}

void ASDBlock::setValid(BOOL valid) {
	m_receivedPersistenceID = ((valid) ? 1 : -1) * m_correctPersistenceID;
}

float strToF(string s) {
	string posS = s;
	if(s.length() > 0 && s[0] == '-') posS = s.erase(0, 1);

	if(posS == S_INDETERMINITE || posS == S_QUIET_NAN || posS == S_UNKNOWN) {
		return F_QUIET_NAN;
	} else if(posS == S_INFINITY) {
		return (s[0] != '-') ? F_INFINITY : F_NEG_INFINITY;
	} else {
		return (float)atof(s.c_str());
	}
}

