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
 * $Id: tlTuple.h 65 2019-03-06 16:58:27Z andrew $
 */


#ifndef		__TOOL_TUPLE_HEADER__
#define		__TOOL_TUPLE_HEADER__

#include "os_defs.h"

#include "massert.h"

#include "tlRef.h"
#include "tlSrString.h"
#include "tlSrValue.h"

/** friend for non-reffed handle wrapper */
class tlTuple;

/**
CLASS
		tlTuple

	A ref-counted row from a table.  A "tuple" differs from
	a vector in that a vector is a list of elements of a single
	type; a tuple is a list of possibly mixed types.
*/
class OS_EXPORT tlTuple : public tlRef
{
protected:

	int n_;
	int nBlocks_;
	tlSrValue *value_;

public:
	////////////////////////////////////////
	// Constructor
	tlTuple();

	////////////////////////////////////////
	// Constructor
	tlTuple(int nValues, tlSrValue *values = NULL);

	////////////////////////////////////////
	// Copy constructor
	tlTuple(tlTuple *src);

	////////////////////////////////////////
	// Copy constructor
	tlTuple(tlTuple &src);

protected:
	~tlTuple();

	////////////////////////////////////////
	// Return the typename for
	// reference debugging
	const char *clsId() const;

public:

	////////////////////////////////////////
	// return the length of the vector
	int getNumValues() const;

	////////////////////////////////////////
	// set the length of the vector
	void setNumValues(int length);

	////////////////////////////////////////
	// return value vector
	tlSrValue *getValues() const;

	////////////////////////////////////////
	// return value <b>index</b>
	tlSrValue getValue(int index) const;

	////////////////////////////////////////
	// Set a value in the vector
	void setValue(int index, tlSrValue value);

	////////////////////////////////////////
	// Set a value to Nil in the vector
	void setNil(int index);

	////////////////////////////////////////
	// return the Nil status of element <b>index</b>
	int isNil(int index) const;

	////////////////////////////////////////
	// Set all values from another tlTuple
	void setValues(tlTuple *other);

	////////////////////////////////////////
	// Add a value to the vector, increasing the length
	void addValue(tlSrValue value);

	////////////////////////////////////////
	// Add a string to the vector, increasing the length
	void addValue(tlSrString value);

	////////////////////////////////////////
	// Add a string to the vector, increasing the length
	void addValue(const char *value);

	////////////////////////////////////////
	// Add an integer to the vector, increasing the length
	void addValue(int value);

	////////////////////////////////////////
	// Add a tlReal to the vector, increasing the length
	void addValue(tlReal value);

	////////////////////////////////////////
	// Add a Nil to the vector, increasing the length
	void addNil();

	////////////////////////////////////////
	// Get the contents as a string
	tlSrString getStringValue() const;

	////////////////////////////////////////
	// print out the row to a file pointer
	// (<b>ofp</b>), separating each field
	// with the string in <b>delim</b>
	// and pre-padding each field to columnWidth
	// <b>width</b>
	int printRow(
		    FILE *ofp,
		    int columnWidth,
		    const char *delim
		) const;

	////////////////////////////////////////
	// Clear the old contents of the vector
	void clear();
};

inline int tlTuple::getNumValues() const
{
	return n_;
}

inline tlSrValue *tlTuple::getValues() const
{
	return value_;
}

inline tlSrValue tlTuple::getValue(int i) const
{
	return value_[i];
}

inline void tlTuple::setValue(int i, tlSrValue value)
{
	MSG_ASSERT(i >= 0 && i < n_, "Index out of range");
	value_[i] = value;
}

inline void tlTuple::setNil(int i)
{
	MSG_ASSERT(i >= 0 && i < n_, "Index out of range");
	value_[i].setNil();
}

inline int tlTuple::isNil(int i) const
{
	MSG_ASSERT(i >= 0 && i < n_, "Index out of range");
	return value_[i].isNil();
}

#endif /* __TOOL_TUPLE_HEADER__ */

