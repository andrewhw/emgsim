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
 *..
 * $Id: tlTable.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_DATA_TABLE_HEADER__
#define		__TOOL_DATA_TABLE_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrString.h"
#include "tlSrValue.h"
#include "tlColumn.h"

class tlTuple;

/**
CLASS
		tlTable

	A data table which stores data by column name &amp; type;
	the results can be accessed similar to an array.
*/
class OS_EXPORT tlTable : public tlRef
{
protected:
	int numColumns_;
	int columnBlocks_;

	tlColumn **column_;
	double *sigmaSquared_;
	double *mu_;

	int maxRows_;

	int hasLoadMappings_;

	tlTuple **tupleList_;

	void initTupleList_();
	void clearTupleList_();

public:

	////////////////////////////////////////
	// Constructor
	tlTable();

	////////////////////////////////////////
	// Constructor used in cloning tables
	tlTable(tlTable *other);

protected:
	////////////////////////////////////////
	// Destructor.
	~tlTable();

public:
	////////////////////////////////////////
	// get a single value
	tlSrValue getValue(int row, int column) const;

	////////////////////////////////////////
	// How many columns do we have?
	int getNumColumns() const;

	////////////////////////////////////////
	// Get the column, including column description
	tlColumn *getColumn(int index) const;

	////////////////////////////////////////
	// Get the column, including column description
	tlColumn *getColumn(const char *name) const;

	////////////////////////////////////////
	// Get a column index by name
	int getColumnIndex(const char *name) const;

	////////////////////////////////////////
	// get the name of a column
	tlSrString getColumnName(int index) const;

	////////////////////////////////////////
	// add a new column, potentially with a name
	virtual void addColumn(
				const char *name = NULL,
				int type = tlSrValue::NONE
		    );

	////////////////////////////////////////
	// add a new column, potentially with a name
	virtual void addColumn(
					tlSrString name,
				int type = tlSrValue::NONE
		    );


	////////////////////////////////////////
	// Clear the inter-column load mappings
	void clearColumnLoadMappings();

	////////////////////////////////////////
	// Are there any column load mappings?
	int hasColumnLoadMappings() const;

	////////////////////////////////////////
	// Clear the inter-column load mappings
	int getColumnLoadMapping(
				int columnIndex
		    ) const;

	////////////////////////////////////////
	// set an inter-column load mapping
	void setColumnLoadMapping(
				int actualIndex,
				int mappedIndex
		    );


	////////////////////////////////////////
	// How many rows do we have?
	int getNumRows() const;

	////////////////////////////////////////
	// return a row by index
	tlTuple *getRow(int index);

	////////////////////////////////////////
	// add a new row
	virtual void addRow(tlTuple *row);


	////////////////////////////////////////
	// Calculate the variance for each column
	virtual int calculateVariances();

	////////////////////////////////////////
	// return the variance for this column
	double getVariance(int columnIndex) const;

	////////////////////////////////////////
	// return the average for this column
	double getMu(int columnIndex) const;

	////////////////////////////////////////
	// clear all the internal data, including column
	// descriptions.
	virtual void clear();

	////////////////////////////////////////
	// Dump to a file
	virtual int dump(FILE *fp, int printHeader = 1) const;

};

inline int tlTable::getNumColumns() const
{
	return numColumns_;
}

inline int tlTable::getNumRows() const
{
	return maxRows_;
}

inline tlSrString tlTable::getColumnName(int index) const
{
	return column_[index]->getName();
}

inline tlColumn *tlTable::getColumn(int index) const
{
	MSG_ASSERT(index >= 0 && index < numColumns_, "Bad column index");
	return column_[index];
}

inline double tlTable::getVariance(int column) const
{
	if (sigmaSquared_ == NULL) return (-1);
	return sigmaSquared_[column];
}

inline double tlTable::getMu(int column) const
{
	if (mu_ == NULL) return (-1);
	return mu_[column];
}

inline int tlTable::getColumnLoadMapping(int columnIndex) const
{
	return column_[columnIndex]->getLoadMapping();
}

inline int tlTable::hasColumnLoadMappings() const
{
	return hasLoadMappings_;
}

#endif /* __TOOL_DATA_TABLE_HEADER__ */

