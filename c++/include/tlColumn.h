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
 * $Id: tlColumn.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_COLUMN_HEADER__
#define		__TOOL_COLUMN_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
#endif

#include "os_defs.h"
#include "bitstring.h"

#include "tlSrString.h"
#include "tlSrValue.h"

class tlBin;
class tlBinTable;

/**
CLASS
		tlColumn

	A typed list of values; a column from a tlTable.
*/
class OS_EXPORT tlColumn : public tlRef
{
public:
protected:
	int type_;
	tlValueStorage *data_;
	int numValues_;
	int numBlocks_;
	tlSrString name_;
	int loadMapping_;

	char *nilBitString_;
	int numBitBlocks_;

	int isLabel_;

public:
	////////////////////////////////////////
	// Construct with no internals
	tlColumn();

	////////////////////////////////////////
	// Construct with type only
	tlColumn(int type);

	////////////////////////////////////////
	// Constructor with name and type
	tlColumn(const char *name, int type = tlSrValue::NONE);

	////////////////////////////////////////
	// Constructor with name and type
	tlColumn(tlSrString name, int type = tlSrValue::NONE);

	////////////////////////////////////////
	// Constructor used in cloning tables
	tlColumn(tlColumn *other);

protected:
	////////////////////////////////////////
	// Destructor.
	~tlColumn();

	////////////////////////////////////////
	// Return the typename for
	// reference debugging
	const char *clsId() const;

public:
	////////////////////////////////////////
	// get the type of this column
	int getType() const;

	////////////////////////////////////////
	// set the type of this column
	void setType(int newType);



	////////////////////////////////////////
	// set a number of (zero) values
	void setLength(int length);

	////////////////////////////////////////
	// get the type of this column
	int getLength() const;


	////////////////////////////////////////
	// return the name of the column
	tlSrString getName() const;


	////////////////////////////////////////
	// Create a new entry for this column,
	// whose value is Nil
	void addNilValue();

	////////////////////////////////////////
	// return the NULL status for this field
	int isNilValue(int index) const;

	////////////////////////////////////////
	// set the NULL status for this field
	void setNilValue(int index, int flag);

	////////////////////////////////////////
	// query whether there are any nil values
	// in this column at all
	int hasAnyNilValue() const;


	////////////////////////////////////////
	// return the value as a string
	tlSrString getStringValue(int index) const;

	////////////////////////////////////////
	// return the value as an int
	int getIntegerValue(int index) const;

	////////////////////////////////////////
	// return the value as a tlReal
	tlReal getRealValue(int index) const;


	////////////////////////////////////////
	// Set from a companion value
	void setValue(tlColumn *v);

	////////////////////////////////////////
	// Set to a string value
	void setValue(int index, tlSrString s);

	////////////////////////////////////////
	// Set to a string value
	void setValue(int index, const char *s);

	////////////////////////////////////////
	// Set to an integer value
	void setValue(int index, int ival);

	////////////////////////////////////////
	// Set to a tlReal value
	void setValue(int index, tlReal dval);


	////////////////////////////////////////
	// Add a string value
	void addValue(tlSrString s);

	////////////////////////////////////////
	// Add a string value
	void addValue(const char *s);

	////////////////////////////////////////
	// Add an integer value
	void addValue(int ival);

	////////////////////////////////////////
	// Add a tlReal value
	void addValue(tlReal dval);

	////////////////////////////////////////
	// Add a generic value
	void addValue(tlSrValue val);


	////////////////////////////////////////
	// Set the load mapping value
	void setLoadMapping(int index);

	////////////////////////////////////////
	// Get the load mapping value
	int getLoadMapping() const;


	////////////////////////////////////////
	// Calculate variance, storing sigmaSquared and mu
	void calculateVariance(double *sigmaSquared, double *mu) const;

	////////////////////////////////////////
	// Calculate mean
	double calculateMean() const;


	////////////////////////////////////////
	// Flag this as a label, or not
	void setLabel(int flag);

	////////////////////////////////////////
	// Is this flagged as a label
	int isLabel() const;


	////////////////////////////////////////
	// Clear the old contents of the vector
	void clear();

	friend class tlBin;
	friend class tlBinTable;
};

inline int tlColumn::getType() const
{
	return type_;
}

inline tlSrString tlColumn::getName() const
{
	return name_;
}

inline int tlColumn::getLength() const
{
	return numValues_;
}

inline void tlColumn::setLoadMapping(int mapping)
{
	loadMapping_ = mapping;
}

inline int tlColumn::getLoadMapping() const
{
	return loadMapping_;
}

inline void tlColumn::setLabel(int flag)
{
	isLabel_ = flag;
}

inline int tlColumn::isLabel() const
{
	return isLabel_;
}

#endif /* __TOOL_COLUMN_HEADER__ */

