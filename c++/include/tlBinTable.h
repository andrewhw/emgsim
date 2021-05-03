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
 * CUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, BIN, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *..
 * $Id: tlBinTable.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_BIN_TABLE_HEADER__
#define		__TOOL_BIN_TABLE_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "tlTable.h"
#include "tlBin.h"

class tlErrorManager;

/**
CLASS
		tlBinTable

	A data table which can aggregate the data it stores.
	Data is stored by column name &amp; type;
	the results can be accessed similar to an array.
*/
class OS_EXPORT tlBinTable : public tlTable
{
public:
	enum BinStrategy {
				MME_GROUP_UNIQUE = 0,
				MME_IGNORE_UNIQUE,
				EQUAL_BIN_RANGE
		    };

	class BinIndexPair {
	public:
		tlSrString		name_;
		int				nBins_;
	};

	////////////////////////////////////////
	// Constructor
	tlBinTable();

	////////////////////////////////////////
	// Constructor used for cloning tables
	tlBinTable(tlBinTable *other);

protected:
	////////////////////////////////////////
	// Destructor.
	~tlBinTable();

public:
	////////////////////////////////////////
	// add a new column, potentially with a name
	void addColumn(const char *name = NULL, int type = tlSrValue::NONE);

	////////////////////////////////////////
	// add a new column, potentially with a name
	void addColumn(tlSrString name, int type = tlSrValue::NONE);

	////////////////////////////////////////
	// Build up the bin descriptions
	int createBins(
		    BinStrategy strategy,
		    int masterNumBins,
		    tlErrorManager *err,
		    int nColumnsInBinOverrideList = 0,
		    BinIndexPair *overrideNumBinList = NULL
		);

	////////////////////////////////////////
	// Return the bin description for a column
	tlBin *getColumnBin(int columnIndex) const;

	////////////////////////////////////////
	// add a new row
	void addRow(tlTuple *row);

	////////////////////////////////////////
	// Calculate the variance for each column,
	// using the bin ids for string data if
	// necessary
	int calculateVariances();

	////////////////////////////////////////
	// Clear contents
	virtual void clear();

	////////////////////////////////////////
	// Dump to a file
	virtual int dump(FILE *fp, int printHeader = 1) const;

	////////////////////////////////////////
	// Store the bin definitions to a file
	// for use in a configuration-based load
	int storeConfigFeatureDescriptions(
		    int indent,
		    FILE *fp
		) const;

	////////////////////////////////////////
	// Store the data to the file pointer,
	// recording only the bin id of each
	// data item.
	int saveDataAsBinIds(FILE *fp) const;

	////////////////////////////////////////
	// Store one row to the file pointer,
	// recording only the bin id of each
	// data item.
	int saveRowAsBinIds(FILE *ofp, int index, int colWidth) const;

	////////////////////////////////////////
	// Load the bin definitions from a file
	int loadFeatureDescriptions(FILE *fp);

	////////////////////////////////////////
	// Fill the vector with the bin ids for
	// this pre-loaded row
	int getRowBinIds(tlBin::tlBinId *idVector, int rowIndex) const;

	////////////////////////////////////////
	// Fill the vector with the bin ids using
	// the supplied row, and only our header data
	int getRowBinIds(tlBin::tlBinId *idVector, tlTuple *row) const;

	////////////////////////////////////////
	// Assign data values to items in the given <b>row</b>
	// based on the ids in <b>idVector</b>.
	int fillInNilDataFromBinValues(
				tlTuple *row,
				tlBin::tlBinId *idVector,
				int *nFilledIn = NULL,
				int *nilDataLocations = NULL
		    ) const;


private:
	/** data members */
	int binBlocks_;
	tlBin **bin_;
	int binsCreated_;

	BinStrategy binStrategy_;
};

inline tlBin *tlBinTable::getColumnBin(int columnIndex) const
{
	return bin_[columnIndex];
}

#endif /* __TOOL_BIN_TABLE_HEADER__ */

