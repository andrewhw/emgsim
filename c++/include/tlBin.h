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
 * $Id: tlBin.h 61 2017-02-03 20:49:55Z andrew $
 */


#ifndef		__TOOL_COLUMN_BIN_HEADER__
#define		__TOOL_COLUMN_BIN_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrValue.h"

class tlColumn;
class tlHashTable;
class tlErrorManager;
class tlStringAllocationTool;

/**
CLASS
		tlBin

	A `bin' which will hold all the data associated with a
	specific lookup value.  This is the results of the discretization
	algorithm, and is held/managed within a tlBinTable.
*/
class OS_EXPORT tlBin : public tlRef
{
public:
	typedef short tlBinId;

	static const tlBinId nilId;

private:
	int numBins_;
	tlBinId *binData_;
	int binDataBlocks_;
	tlColumn *column_;
	tlValueStorage **sortedData_;
	tlValueStorage *binLoBound_;
	tlValueStorage *binHiBound_;

	tlReal *binActualLoBound_;
	tlReal *binActualHiBound_;

	int boundsFlags_;

	tlStringAllocationTool *allocStrings_;

	tlSrString **niceNames_;

public:
	////////////////////////////////////////
	// Constructor
	tlBin(tlColumn *column);

	////////////////////////////////////////
	// Constructor used for cloning bin tables
	tlBin(tlColumn *column, tlBin *other);

protected:
	////////////////////////////////////////
	// Destructor.
	~tlBin();

	void allocateRealDataBinBounds__();

	int binDataMMEGroupUniqueNumeric__(
				int numPerBin,
				tlErrorManager *err
		    );

	int binDataMMEIgnoreUniqueNumeric__(
				int numPerBin,
				tlErrorManager *err
		    );

	int binDataEqualRangeFloat__(
				int numPerBin,
				tlErrorManager *err
		    );

	int binDataString__(tlErrorManager *err);

	int binDataEqualRangeOrdinal__(
				int compare(const void *v1, const void *v2),
				tlBinId getBinIndex(
							const tlValueStorage *element,
						const tlValueStorage *lo,
						const tlValueStorage *hi,
						int index, int numElements, int numBins
				    ),
				int numPerBin,
				tlErrorManager *err
		    );

	void cleanAllocatedStrings__();

	int computeBounds__();
	int resolveGapsBetweenRealBins__();
	int allocateBounds__(int newNumBins);

public:

	////////////////////////////////////////
	// Calculate variance, storing sigmaSquared and mu
	void calculateVariance(double *sigmaSquared, double *mu);

	////////////////////////////////////////
	// Create the bins for a column, putting
	// at least <b>numPerBin</b> values in each
	// Maximum Marginal Entropy defined bin.
	//
	// Identical values will be assigned
	// to the same bin, possibly increasing this
	// bin above the standard assignment number
	int binDataMMEGroupUnique(
				int numBins,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Create the bins for a column, putting
	// at least <b>numPerBin</b> values in each
	// Maximum Marginal Entropy defined bin.
	//
	// Identical values will be assigned
	// across bins, possibly creating bins
	// which will never be used when assigning
	// new data values (because other bins
	// will share their data values
	int binDataMMEIgnoreUnique(
				int numBins,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// Create the bins for a column dividing
	// the data range up into <b>numBins</b>
	// regions, and assiging the values to
	// each region as they fall.
	int binDataEqualRange(
				int numBins,
				tlErrorManager *err
		    );

	////////////////////////////////////////
	// return a bin ID for the data shown
	tlBinId getBinId(
					const tlValueStorage *data,
				int useNearest = 0
		    ) const;

	////////////////////////////////////////
	// return a bin ID by index
	tlBinId getBinId(int index) const;


	////////////////////////////////////////
	// return the low bound for a given bin
	tlValueStorage *getLoBoundValue(int index) const;

	////////////////////////////////////////
	// return the high bound for a given bin
	tlValueStorage *getHiBoundValue(int index) const;

	////////////////////////////////////////
	// return the low bound for a given bin
	tlReal getActualLoBoundValue(int index) const;

	////////////////////////////////////////
	// return the high bound for a given bin
	tlReal getActualHiBoundValue(int index) const;


	////////////////////////////////////////
	// data was added to the watched column,
	// so we need to update the bin id
	// for it
	void updateLastDataValue();

	////////////////////////////////////////
	// return the number of bins created
	int getNumBins() const;

	////////////////////////////////////////
	// store the descriptions to be loaded later
	int storeFeatureDescription(int indent, FILE *ofp);

	////////////////////////////////////////
	// load the descriptions stored earlier;
	// this will be used to bin incoming data
	int loadFeatureDescription(FILE *ofp);

	////////////////////////////////////////
	// load the descriptions stored earlier;
	// using a tokenier.
	// This will be used to bin incoming data
	int loadFeatureDescription(
				struct tokenizer *t,
				struct attValList *parsingVariables = NULL,
				tlErrorManager *err = NULL
		    );

	////////////////////////////////////////
	// return a nice name for this bin value
	tlSrString getNiceName(int index);

	////////////////////////////////////////
	// dump out contents
	int dump(FILE *ofp) const;

};

inline int tlBin::getNumBins() const
{
	return numBins_;
}

inline tlValueStorage * tlBin::getLoBoundValue(int index) const
{
	return &binLoBound_[index];
}

inline tlValueStorage * tlBin::getHiBoundValue(int index) const
{
	return &binHiBound_[index];
}

inline tlReal tlBin::getActualLoBoundValue(int index) const
{
	return binActualLoBound_[index];
}

inline tlReal tlBin::getActualHiBoundValue(int index) const
{
	return binActualHiBound_[index];
}

#endif /* __TOOL_COLUMN_BIN_HEADER__ */

