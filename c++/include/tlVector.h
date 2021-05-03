/**
 * Copyright (c) 2013
 * All rights reserved.
 *
 * This code is part of the reseach work of
 * Andrew Hamilton-Wright (andrewhw@ieee.org).
 *
 * ----------------------------------------------------------------
 *
 * Redistribution and use in source and binary forms, with or with-
 * out modification, are permitted provided that recognition of the
 * author as the original contributor is provided in any source or
 * documentation relating to this code.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WAR-
 * RANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.
 *
 * IN NO EVENT SHALL THE AUTHOR OR ANY ASSOCIATED INSTITUTION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PRO-
 * CUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * $Id: tlVector.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_VECTOR_HEADER__
#define		__TOOL_VECTOR_HEADER__

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrString.h"

/** friend for non-reffed handle wrapper */
class tlVector;

/**
CLASS
		tlVector

	A simple non-template vector class (of doubles).
*/
class OS_EXPORT tlVector : public tlRef
{
protected:

	int n_;
	int nBlocks_;
	double *value_;


public:
	////////////////////////////////////////
	// Constructor
	tlVector();

	////////////////////////////////////////
	// Constructor
	tlVector(int nValues);

protected:
	~tlVector();

	////////////////////////////////////////
	// Return the typename for
	// reference debugging
	const char *clsId() const;

public:

	////////////////////////////////////////
	// return the length of the vector
	int getNumValues() const;

	////////////////////////////////////////
	// return the length of the vector
	void setNumValues(int length);

	////////////////////////////////////////
	// return value <b>index</b>
	double getValue(int index) const;

	////////////////////////////////////////
	// Set a value in the vector
	void setValue(int index, double value);

	////////////////////////////////////////
	// Add a value to the vector, increasing the length
	void addValue(double value);

	////////////////////////////////////////
	// Clear the old contents of the vector
	void clear();
};

inline int tlVector::getNumValues() const
{
	return n_;
}

inline double tlVector::getValue(int i) const
{
	return value_[i];
}

inline void tlVector::setValue(int i, double value)
{
	value_[i] = value;
}

#endif /* __TOOL_VECTOR_HEADER__ */

