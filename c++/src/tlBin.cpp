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
 * $Id: tlBin.cpp 63 2017-02-03 21:38:35Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
#endif

#include "tclCkalloc.h"
#include "attvalfile.h"
#include "listalloc.h"
#include "stringtools.h"
#include "mathtools.h"
#include "tokens.h"

#include "tlBin.h"
#include "tlColumn.h"
#include "tlSrValue.h"
#include "tlErrorManager.h"
#include "tlSParser.h"
#include "tlStringAllocationTool.h"

#define BLOCKSIZE		16

#define FLAG_DIRTY		0x01
#define FLAG_LOCK		0x02

#define BOUNDS_DIRTY		((boundsFlags_ & FLAG_DIRTY) != 0)
#define BOUNDS_LOCKED		((boundsFlags_ & FLAG_LOCK) != 0)


/** initialize static nil */
const tlBin::tlBinId tlBin::nilId = (-2);

tlBin::tlBin(tlColumn *column)
{
	column_ = column;
	column_->ref();

	numBins_ = 0;

	binData_ = NULL;

	sortedData_ = NULL;
	binDataBlocks_ = 0;

	binLoBound_ = NULL;
	binHiBound_ = NULL;

	binActualLoBound_ = NULL;
	binActualHiBound_ = NULL;

	boundsFlags_ = (FLAG_DIRTY);

	allocStrings_ = new tlStringAllocationTool();
	allocStrings_->ref();

	niceNames_ = NULL;
}

tlBin::tlBin(tlColumn *column, tlBin *other)
{
    int status, i;

    column_ = column;
    column_->ref();

    numBins_ = other->numBins_;

    MSG_ASSERT(column_->numValues_ == other->column_->numValues_,
		    		"column length mismatch");

    binData_ = NULL;
    binDataBlocks_ = 0;

    if (other->binData_ != NULL)
    {
		status = listCheckSize(column_->numValues_,
				(void **) &binData_,
				&binDataBlocks_,
				BLOCKSIZE,
				sizeof(tlBinId));
		MSG_ASSERT(status, "Out of memory");

		if (other->sortedData_ != NULL)
		{
		    sortedData_ = (tlValueStorage **)
				    ckalloc(sizeof(tlValueStorage *) * column_->numValues_);
		    MSG_ASSERT(sortedData_ != NULL, "Out of memory");

		    for (i = 0; i < column_->numValues_; i++)
		    {
				sortedData_[i] = other->sortedData_[i];
				binData_[i] = other->binData_[i];
		    }
		} else
		{
		    sortedData_ = NULL;
		}
    } else
    {
		sortedData_ = NULL;
    }

    if (numBins_ > 0)
    {

		binLoBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
		MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
		memcpy(binLoBound_, other->binLoBound_,
						numBins_ * sizeof(tlValueStorage));

		binHiBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
		MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
		memcpy(binHiBound_, other->binHiBound_,
						numBins_ * sizeof(tlValueStorage));

		if (column_->type_ == tlSrValue::REAL)
		{

		    binActualLoBound_ = (tlReal *)
				    ckalloc(numBins_ * sizeof(tlReal));
		    MSG_ASSERT(binActualLoBound_ != NULL, "Out of memory");
		    memcpy(binActualLoBound_, other->binActualLoBound_,
						    numBins_ * sizeof(tlReal));

		    binActualHiBound_ = (tlReal *)
				    ckalloc(numBins_ * sizeof(tlReal));
		    MSG_ASSERT(binActualHiBound_ != NULL, "Out of memory");
		    memcpy(binActualHiBound_, other->binActualHiBound_,
						    numBins_ * sizeof(tlReal));
		} else
		{
		    binActualLoBound_ = NULL;
		    binActualHiBound_ = NULL;
		}

    } else
    {
		binLoBound_ = NULL;
		binHiBound_ = NULL;
		binActualLoBound_ = NULL;
		binActualHiBound_ = NULL;
    }

    boundsFlags_ = other->boundsFlags_;

    allocStrings_ = new tlStringAllocationTool();
    allocStrings_->ref();

	niceNames_ = NULL;
}

tlBin::~tlBin()
{
	allocStrings_->unref();

	if (niceNames_ != NULL)
	{
		for (int i = 0; i < column_->numValues_; i++)
			if (niceNames_[i] != NULL)
				delete niceNames_[i];
		ckfree(niceNames_);
	}

	if (binData_ != NULL)
	{
		ckfree(binData_);
		binData_ = NULL;
	}
	if (column_ != NULL)
	{
		column_->unref();
		column_ = NULL;
	}
	if (sortedData_ != NULL)
	{
		ckfree(sortedData_);
	}
	if (binLoBound_ != NULL)
	{
		ckfree(binLoBound_);
	}
	if (binHiBound_ != NULL)
	{
		ckfree(binHiBound_);
	}
	if (binActualLoBound_ != NULL)
	{
		ckfree(binActualLoBound_);
	}
	if (binActualHiBound_ != NULL)
	{
		ckfree(binActualHiBound_);
	}

}

void
tlBin::allocateRealDataBinBounds__()
{
	if ((column_->type_ == tlSrValue::REAL) && (binActualLoBound_ == NULL))
	{
		binActualLoBound_ = (tlReal *)
				ckalloc(numBins_ * sizeof(tlReal));
		MSG_ASSERT(binActualLoBound_ != NULL, "Out of memory");
		memset(binActualLoBound_, 0x0, numBins_ * sizeof(tlReal));

		binActualHiBound_ = (tlReal *)
				ckalloc(numBins_ * sizeof(tlReal));
		MSG_ASSERT(binActualHiBound_ != NULL, "Out of memory");
		memset(binActualHiBound_, 0x0, numBins_ * sizeof(tlReal));
	}
}

void tlBin::cleanAllocatedStrings__()
{
	allocStrings_->clean();
}

void
tlBin::calculateVariance(double *sigmaSquared_p, double *mu_p)
{

	/**
	 * we are are not string type, fall through to column
	 * calculation
	 */
	if (column_->getType() != tlSrValue::STRING)
	{
		column_->calculateVariance(sigmaSquared_p, mu_p);
		return;
	}

	/**
	 * If we are string type, then use the bins if we can
	 */
	if (binData_ == NULL)
	{
		*sigmaSquared_p = (-1);
		*mu_p = (-1);
		return;
	}

	/**
	 * otherwise we fudge string variance values based on the
	 * bin offset as though they were ordered integer labels
	 */
	{
		double sqSum, muSum, mu;
		int val;
		int i, nNonNilValues;

		/**
		 * variance can be calculated by:
		 *     sigma^2 = (\Sum(x - mu)^2)/(n)
		 * or more conveniently
		 *     sigma^2 = (\Sum(x)^2)/n - mu^2
		 * which lets us calculate mu in the same loop.
		 *
		 * ^-- this is a biased estimate of the variance,
		 * so instead we use:
		 *
		 *   sigma^2 = \frac{1}{n-1}
		 *   		\left(
		 *   				\left( \Sum{x^2} \right) - n (mu^2)
		 *   		\right
		 */
		sqSum = muSum = 0;
		nNonNilValues = 0;
		for (i = 0; i < column_->numValues_; i++)
		{
			if (!column_->isNilValue(i))
			{
				nNonNilValues++;

				val = binData_[i];

				muSum = muSum + val;
				sqSum = sqSum + SQR(val);
			}
		}

		if (column_->numValues_ < 2)
		{
			*mu_p = 0;
			*sigmaSquared_p = 0;
		} else
		{
			*mu_p = mu = muSum / nNonNilValues;
			*sigmaSquared_p =
					(1.0 / (double) (nNonNilValues - 1))
					* (sqSum - ( nNonNilValues * SQR(mu)));
		}
	}
}

/**
 * These three functions need to take voids so that they
 * can be used in qsort
 */
static int compare_String(const void *v1, const void *v2)
{
	tlValueStorage **data1;
	tlValueStorage **data2;

	data1 = (tlValueStorage **) v1;
	data2 = (tlValueStorage **) v2;

	return strcmp(
				(*data1)->str_->getValue(),
				(*data2)->str_->getValue()
			);
}

static int compare_Integer(const void *v1, const void *v2)
{
	tlValueStorage **data1;
	tlValueStorage **data2;

	data1 = (tlValueStorage **) v1;
	data2 = (tlValueStorage **) v2;

	return (*data1)->ival_ - (*data2)->ival_;
}

static int compare_Double(const void *v1, const void *v2)
{
	tlValueStorage **data1;
	tlValueStorage **data2;

	data1 = (tlValueStorage **) v1;
	data2 = (tlValueStorage **) v2;

	if ((*data1)->dval_ == (*data2)->dval_)
		return 0;
	if ((*data1)->dval_ < (*data2)->dval_)
		return (-1);
	return 1;
}

static tlBin::tlBinId
getBinIndex_Integer(
		const tlValueStorage *element,
		const tlValueStorage *lowRange,
		const tlValueStorage *highRange,
		int index,
		int totalNumElements,
		int numBins
    )
{
	int totalRange;
	int fraction;

	(void) index;		// suppress warning
	(void) totalNumElements;

	totalRange = highRange->ival_ - lowRange->ival_;
	fraction = (int) (((numBins-1) * (element->ival_ - lowRange->ival_))
				/ (float) totalRange);

	return (tlBin::tlBinId) fraction;
}

int
tlBin::binDataMMEGroupUniqueNumeric__(int desiredNumBins, tlErrorManager *err)
{
	int sortIndex, unsortIndex;
	int curBinIndex, numBinned;
	int numPerBin = 0, checkNumPerBin;
	int nUniqueElements;
	int status;

	(void) err;

	boundsFlags_ |= (FLAG_DIRTY);
	boundsFlags_ |= (FLAG_LOCK);
	cleanAllocatedStrings__();

	if (column_->type_ == tlSrValue::REAL)
	{
		qsort(sortedData_, column_->numValues_, sizeof(tlSrValue *),
				compare_Double);

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		qsort(sortedData_, column_->numValues_, sizeof(tlSrValue *),
				compare_Integer);

	} else
	{
		MSG_FAIL("unknown column type");
	}

	nUniqueElements = 1;
	if (column_->type_ == tlSrValue::REAL)
	{
		for (sortIndex = 1; sortIndex < column_->numValues_; sortIndex++)
		{
			if (sortedData_[sortIndex]->dval_
							!= sortedData_[sortIndex - 1]->dval_)
						{
				nUniqueElements++;
			}
		}
	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		for (sortIndex = 1; sortIndex < column_->numValues_; sortIndex++)
		{
			if (sortedData_[sortIndex]->ival_
							!= sortedData_[sortIndex - 1]->ival_)
						{
				nUniqueElements++;
			}
		}
	}


	/**
	 * figure out how many bins we need
	 */
	if (nUniqueElements == 1)
	{
		numBins_ = 1;
	} else
	{

		/** we certainly cannot have more bins than unique elements */
		if (desiredNumBins > nUniqueElements)
			desiredNumBins = nUniqueElements;

		/**
		 * as long as the number of bins will have at least
		 * 5 (possibly identical) values in it, we are ok
		 */
		numPerBin = (int) ((double) (nUniqueElements + 1)
					/ (double) desiredNumBins);
		checkNumPerBin = (int) ((double) (column_->numValues_ + 1)
					/ (double) desiredNumBins);

		if (checkNumPerBin < 5)
		{
			numPerBin = 5;
			numBins_ = nUniqueElements / 5;
			if (numBins_ <= 0)
				numBins_ = 1;
		} else
		{
			numBins_ = desiredNumBins;
		}
	}



	/**
	 * allocate storage for all the bins so that we can put pointers
	 * to these locations in the hash table and not worry about them
	 * moving
	 */
	if (binData_ != NULL)
	{
		ckfree(binData_);
		binData_ = NULL;
	}

	status = listCheckSize(column_->numValues_,
			(void **) &binData_,
			&binDataBlocks_,
			BLOCKSIZE,
			sizeof(tlBinId));
	MSG_ASSERT(status, "Out of memory");

	memset(binData_, 0xFF, column_->numValues_ * sizeof(tlBinId));





	/**
	 * allocate the bin bounds, now that we know how many bins
	 * we will be using
	 */
	MSG_ASSERT(binLoBound_ == NULL, "bound already allocated");
	binLoBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
	memset(binLoBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	MSG_ASSERT(binHiBound_ == NULL, "bound already allocated");
	binHiBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
	memset(binHiBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	allocateRealDataBinBounds__();



	/** if there is only one bin, handle it specially */
	if (numBins_ == 1)
	{
		for (unsortIndex = 0; unsortIndex < column_->numValues_; unsortIndex++)
		{
			binData_[unsortIndex] = 0;
		}
		binHiBound_[0] = *(sortedData_[0]);
		binHiBound_[0] = *(sortedData_[0]);

		boundsFlags_ &= (~FLAG_DIRTY);
		return 1;
	}

	/** set the low value of the bottom bin and the high value
		of the top bin to the min and max of the data, respectively */
	binLoBound_[0] = *(sortedData_[0]);
	binHiBound_[numBins_ - 1] = *(sortedData_[column_->numValues_ - 1]);

	curBinIndex = 0;
	sortIndex = 0;

	numBinned = 0; // the number of values covered by the bottom bins whose ranges have already been assigned

	// bypass nil values
	while (column_->isNilValue((int) (sortedData_[numBinned] - column_->data_)))
		numBinned++;

	/** go through all bins, and find the nearest position to the optimal bin position 
		that does not break a group of non-unique elements */
	for(curBinIndex = 0; curBinIndex < numBins_ - 1; curBinIndex++) {
		if (numBinned == column_->numValues_ - 1)
			sortIndex = numBinned;
		else
			sortIndex = (int) (numBinned + (1.0 / (numBins_ - curBinIndex)) * (column_->numValues_ - numBinned) - 1);

		int down = 0; // the amount we have to move down before finding a unique pair of elements
		int up = 0; // the amount we have to move up before finding a unique pair of elements
		if (column_->type_ == tlSrValue::REAL)
		{
			while ((sortIndex - down) > numBinned &&
				   (sortedData_[sortIndex - down]->dval_ == sortedData_[sortIndex - down - 1]->dval_))
				down++;

			while ((sortIndex + up + 1) < column_->numValues_ && 
				   (sortedData_[sortIndex + up]->dval_ == sortedData_[sortIndex + up + 1]->dval_))
				up++;

		} else if (column_->type_ == tlSrValue::INTEGER)
		{
			while ((sortIndex - down) > numBinned &&
				   (sortedData_[sortIndex - down]->ival_ == sortedData_[sortIndex - down - 1]->ival_))
				down++;

			while ((sortIndex + up + 1) < column_->numValues_ && 
				   (sortedData_[sortIndex + up]->ival_ == sortedData_[sortIndex + up + 1]->ival_))
				up++;
		}

		if (down < up && (sortIndex - down) > numBinned) {
			//binHiBound_[curBinIndex] = *(sortedData_[sortIndex - down - ((sortIndex - down > numBinned) ? 1 : 0)]);
			numBinned = sortIndex - down;
			binHiBound_[curBinIndex] = *(sortedData_[sortIndex - down - 1]);
			binLoBound_[curBinIndex + 1] = *(sortedData_[sortIndex - down]);
		} else {
			numBinned = sortIndex + up + ((sortIndex + up + 1 < column_->numValues_) ? 1 : 0);
			binHiBound_[curBinIndex] = *(sortedData_[sortIndex + up]);
			binLoBound_[curBinIndex + 1] = *(sortedData_[numBinned]);
		}
	}

	/** Go through all values and assign them to a bin */
	curBinIndex = 0;
	if (column_->type_ == tlSrValue::REAL)
	{
		for(sortIndex = 0; sortIndex < column_->numValues_; sortIndex++) {
			unsortIndex = (int) (sortedData_[sortIndex] - column_->data_);

			if (sortedData_[sortIndex]->dval_ <= binHiBound_[curBinIndex].dval_)
				binData_[unsortIndex] = curBinIndex;
			else
				binData_[unsortIndex] = ++curBinIndex;
		}

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		for(sortIndex = 0; sortIndex < column_->numValues_; sortIndex++) {
			unsortIndex = (int) (sortedData_[sortIndex] - column_->data_);

			if (sortedData_[sortIndex]->ival_ <= binHiBound_[curBinIndex].ival_)
				binData_[unsortIndex] = curBinIndex;
			else
				binData_[unsortIndex] = ++curBinIndex;
		}
	}

	/**
	 * Set the number of bins to the number actually used
	 */
	if ((curBinIndex  + 1) != numBins_)
		numBins_ = curBinIndex  + 1;

	/** make sure that the bin definitions meet each other */
	if (column_->type_ == tlSrValue::REAL)
	{
		resolveGapsBetweenRealBins__();
	}

	boundsFlags_ &= (~FLAG_DIRTY);

	return 1;
}

int
tlBin::binDataMMEGroupUnique(int numBins, tlErrorManager *err)
{
	int i;

	/**
	 * We need a sorted list of data in order to do binning
	 */
	sortedData_ = (tlValueStorage **)
				ckalloc(sizeof(tlValueStorage *) * column_->numValues_);
	MSG_ASSERT(sortedData_ != NULL, "Out of memory");
	for (i = 0; i < column_->numValues_; i++)
	{
		sortedData_[i] = &column_->data_[i];
	}


	if (column_->type_ == tlSrValue::STRING)
	{
		return binDataString__(err);

	} 

	return binDataMMEGroupUniqueNumeric__(numBins, err);

	return 0;
}

int
tlBin::binDataMMEIgnoreUniqueNumeric__(int desiredNumBins, tlErrorManager *err)
{
	int sortIndex, unsortIndex;
	int curBinIndex, countInThisBin;
	int numPerBin;
	int status;

	(void) err;

	boundsFlags_ |= (FLAG_DIRTY);
	boundsFlags_ |= (FLAG_LOCK);
	cleanAllocatedStrings__();

	if (column_->type_ == tlSrValue::REAL)
	{
		qsort(sortedData_, column_->numValues_, sizeof(tlSrValue *),
				compare_Double);

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		qsort(sortedData_, column_->numValues_, sizeof(tlSrValue *),
				compare_Integer);

	} else
	{
		MSG_FAIL("unknown column type");
	}


	/**
	 * figure out how many bins we need
	 */

	/**
	 * as long as the number of bins will have at least
	 * 5 (possibly identical) values in it, we are ok
	 */
	numPerBin = (int) ((double) (column_->numValues_ + 1)
				/ (double) desiredNumBins);

	if (numPerBin < 5)
	{
		numPerBin = 5;
		numBins_ = column_->numValues_ / 5;
		if (numBins_ <= 0)
			numBins_ = 1;
	} else
	{
		numBins_ = desiredNumBins;
	}



	/**
	 * allocate storage for all the bins so that we can put pointers
	 * to these locations in the hash table and not worry about them
	 * moving
	 */
	if (binData_ != NULL)
	{
		ckfree(binData_);
		binData_ = NULL;
	}

	status = listCheckSize(column_->numValues_,
			(void **) &binData_,
			&binDataBlocks_,
			BLOCKSIZE,
			sizeof(tlBinId));
	MSG_ASSERT(status, "Out of memory");

	memset(binData_, 0xFF, column_->numValues_ * sizeof(tlBinId));





	/**
	 * allocate the bin bounds, now that we know how many bins
	 * we will be using
	 */
	MSG_ASSERT(binLoBound_ == NULL, "bound already allocated");
	binLoBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
	memset(binLoBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	MSG_ASSERT(binHiBound_ == NULL, "bound already allocated");
	binHiBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
	memset(binHiBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	if (column_->type_ == tlSrValue::REAL)
	{
		binActualLoBound_ = (tlReal *)
				ckalloc(numBins_ * sizeof(tlReal));
		MSG_ASSERT(binActualLoBound_ != NULL, "Out of memory");
		memset(binActualLoBound_, 0x0, numBins_ * sizeof(tlReal));

		binActualHiBound_ = (tlReal *)
				ckalloc(numBins_ * sizeof(tlReal));
		MSG_ASSERT(binActualHiBound_ != NULL, "Out of memory");
		memset(binActualHiBound_, 0x0, numBins_ * sizeof(tlReal));
	}



	/** if there is only one bin, handle it specially */
	if (numBins_ == 1)
	{
		for (unsortIndex = 0; unsortIndex < column_->numValues_; unsortIndex++)
		{
			binData_[unsortIndex] = 0;
		}
		binHiBound_[0] = *(sortedData_[0]);
		binHiBound_[0] = *(sortedData_[column_->numValues_ - 1]);

		boundsFlags_ &= (~FLAG_DIRTY);
		return 1;
	}



	/** go through all values, assigning a bin in the binData vector */
	curBinIndex = 0;
	countInThisBin = 0;
	sortIndex = 0;
	binLoBound_[0] = *(sortedData_[0]);
	while (sortIndex < column_->numValues_)
	{

		/**
		 * Indices here are complicated because we want to got
		 * through the data in sorted order.
		 * sortIndex   - index into sorted array
		 * unsortIndex - index into unsorted array, calculated
		 * 				 by taking the offset into the sorted array
		 */
		unsortIndex = (int) (sortedData_[sortIndex] - column_->data_);


		binData_[unsortIndex] = curBinIndex;
		sortIndex++;
		countInThisBin++;


		/**
		 * if the bin is "full" start another, unless we are at the end,
		 * or we have run out of bins (due to roundoff we may have to
		 * `overfill' the last bin.
		 * 
		 * (if this is the last element, then we don't want to set
		 * up the next bin (and our tests below are invalid))
		 */
		if ((sortIndex < column_->numValues_)
						&& (countInThisBin >= numPerBin)
						&& (curBinIndex < (numBins_ - 1)))
		{


			binHiBound_[curBinIndex] = *(sortedData_[sortIndex - 1]);
			binLoBound_[curBinIndex + 1] = *(sortedData_[sortIndex]);

			curBinIndex++;
			MSG_ASSERT(curBinIndex < numBins_, "bin index out of range");
			countInThisBin = 0;
		}
	}


	/**
	 * Ensure that the number of bins works out correctly
	 */
	MSG_ASSERT((curBinIndex  + 1) == numBins_, "bin index mismatch");


	/** set the high bound of the last bin */
	binHiBound_[numBins_ - 1] = *(sortedData_[column_->numValues_ - 1]);


	/** make sure that the bin definitions meet each other */
	if (column_->type_ == tlSrValue::REAL)
	{
		resolveGapsBetweenRealBins__();
	}

	/**
	 * we have filled in the bin bounds ourselves, so we do not
	 * want computeBounds__() to try and do it
	 */
	boundsFlags_ &= (~FLAG_DIRTY);
	boundsFlags_ &= (~FLAG_LOCK);

	return 1;
}

int
tlBin::binDataMMEIgnoreUnique(int numBins, tlErrorManager *err)
{
	int i;

	/**
	 * We need a sorted list of data in order to do binning
	 */
	sortedData_ = (tlValueStorage **)
				ckalloc(sizeof(tlValueStorage *) * column_->numValues_);
	MSG_ASSERT(sortedData_ != NULL, "Out of memory");
	for (i = 0; i < column_->numValues_; i++)
	{
		sortedData_[i] = &column_->data_[i];
	}


	if (column_->type_ == tlSrValue::STRING)
	{
		return binDataString__(err);

	} 

	return binDataMMEIgnoreUniqueNumeric__(numBins, err);

	return 0;
}

/**
 * This function assigns id to each bin based on the
 * placement of the data within the sequence.
 *
 * If the number of uniquye elements is less than the
 * total number of bins, then each element value can
 * have its own bin id.  The algorithm optimistically
 * assumes that this is so, and begins by creating
 * a vector of unique indices, which are populated
 * as we count the number of unique values.  If this
 * number of values exceed the number of bins, then
 * this unique index is discarded, and the values are
 * placed simply by their relative sequence within
 * the overall range.
 *
 * This differs primarily for strings, which simply
 * get their ordinal value from the placement within
 * the overall sequence in the sorted list.
 */
int
tlBin::binDataEqualRangeOrdinal__(
		int compare(const void *v1, const void *v2),
		tlBin::tlBinId getBinIndex(
		    		const tlValueStorage *element,
				const tlValueStorage *lowestData,
				const tlValueStorage *highestData,
				int index, int numElements, int numBins
		    ),
		int numSpecifiedBins,
		tlErrorManager *err
    )
{
	tlValueStorage *lastDataPtr;
	tlValueStorage *thisDataPtr;
	tlValueStorage *highestData;
	tlValueStorage *lowestData;
	tlBin::tlBinId *uniqueIds = NULL;
	int sortIndex, unsortIndex;
	int status;
	int i;


	(void) err;

	boundsFlags_ |= (FLAG_DIRTY);

	/** we cannot have less than one bin */
	if (numSpecifiedBins <= 0)
	{
		numSpecifiedBins = 1;
	}

	/**
	 * allocate a vector of unique ids large enough to record
	 * an id for each data element; we will hope that there are
	 * actually less unique values than there are bins, and 
	 * abandon this hope only when we see the first unique
	 * element whose ordinal number is too high.
	 */
	uniqueIds = (tlBin::tlBinId *)
				ckalloc(sizeof(tlBin::tlBinId) * column_->numValues_);
	MSG_ASSERT(uniqueIds != NULL, "Out of memory");

	/**
	 * count the discrete values, trimming back the number of
	 * bins as fit
	 */

	/** set up for the loop */
	numBins_ = 1;
	unsortIndex = (int) (sortedData_[0] - column_->data_);
	thisDataPtr = &column_->data_[unsortIndex];
	uniqueIds[unsortIndex] = (tlBin::tlBinId) 0;

	/**
	 * start at offset 1 and search for distinct values forthe D
	 * through the data set
	 */
	for (sortIndex = 1; sortIndex < column_->numValues_; sortIndex++)
	{

		/**
		 * Indices here are complicated because we want to got
		 * through the data in sorted order.
		 * sortIndex   - index into sorted array
		 * unsortIndex - index into unsorted array, calculated
		 * 				 by taking the offset into the sorted array
		 */
		unsortIndex = (int) (sortedData_[sortIndex] - column_->data_);

		lastDataPtr = thisDataPtr;
		thisDataPtr = &column_->data_[unsortIndex];

		if ((*compare)(&thisDataPtr, &lastDataPtr) != 0)
		{
			numBins_++;

			/**
			 * if we have seen enough distict values, then we are done
			 */
			if (numBins_ >= numSpecifiedBins)
			{

				/**
				 * if we are actually right at the end of the list,
				 * then the unique ids are ok, otherwise, we must
				 * ditch that idea
				 */
				if (sortIndex == column_->numValues_ -1)
				{
					MSG_ASSERT(unsortIndex < column_->numValues_,
							"Index error creating equal size bins");
					uniqueIds[unsortIndex] =
									(tlBin::tlBinId) (numBins_ - 1);
				} else
				{
					ckfree(uniqueIds);
					uniqueIds = NULL;
				}
				numBins_ = numSpecifiedBins;
				break;
			}
		}

		MSG_ASSERT(unsortIndex < column_->numValues_,
							"Index error creating equal size bins");
		uniqueIds[unsortIndex] = (tlBin::tlBinId) (numBins_ - 1);
	}



	/**
	 * Now we know how many bins there are
	 *
	 * allocate storage for all the bins so that we can put pointers
	 * to these locations in the hash table and not worry about them
	 * moving
	 */
	if (binData_ != NULL)
	{
		ckfree(binData_);
		binData_ = NULL;
	}

	status = listCheckSize(column_->numValues_,
			(void **) &binData_,
			&binDataBlocks_,
			BLOCKSIZE,
			sizeof(tlBinId));
	MSG_ASSERT(status, "Out of memory");

	/**
	 * Assign data based on the unique Ids if we got 'em,
	 * otherwise use the index function
	 */
	if (uniqueIds != NULL)
	{
		memcpy(binData_, uniqueIds,
						column_->numValues_ * sizeof(tlBinId));

		/** we are now done with them, so clean them up */
		ckfree(uniqueIds);
	} else
	{
		memset(binData_, 0xFF, column_->numValues_ * sizeof(tlBinId));


		/** record the top and bottom values in the list */
		highestData = sortedData_[column_->numValues_ - 1];
		lowestData = sortedData_[0];


		/** iterate through each element, setting the id value */
		for (i = 0; i < column_->numValues_; i++)
		{

			binData_[i] = getBinIndex(
					&column_->data_[i],
					lowestData, highestData,
					i, column_->numValues_,
					numBins_
				);
		}
	}

	boundsFlags_ |= (FLAG_LOCK);
	computeBounds__();

	return 1;
}


/**
 * Divide the continuous range into bins based on dividing the
 * total range by the number of bins.
 *
 * As we compute the bounds as we go, we never need to sort
 * the data, only find the limits
 */
int
tlBin::binDataEqualRangeFloat__(
		int numSpecifiedBins,
		tlErrorManager *err
    )
{
	double maxValue, minValue;
	double range, binRange = 0;
	double fraction;
	int binId;
	int status;
	int i;

	(void) err;

	boundsFlags_ |= (FLAG_DIRTY);

	/** we cannot have less than one bin */
	if (numSpecifiedBins <= 0)
	{
		numSpecifiedBins = 1;
	}


	/** find the min and max values, and the range */
	minValue = maxValue = column_->data_[0].dval_;
	for (i = 1; i < column_->numValues_; i++)
	{
		if (maxValue < column_->data_[i].dval_)
			maxValue = column_->data_[i].dval_;
		if (minValue > column_->data_[i].dval_)
			minValue = column_->data_[i].dval_;
	}
	range = maxValue - minValue;


	/** if the range is nil, then everything goes in one bin */
	if (range == 0.0)
	{
		numBins_ = 1;
	} else
	{
		binRange = range / numSpecifiedBins;
		numBins_ = numSpecifiedBins;
	}


	/** allocate space for all of the bin ids */
	status = listCheckSize(column_->numValues_,
			(void **) &binData_,
			&binDataBlocks_,
			BLOCKSIZE,
			sizeof(tlBinId));
	MSG_ASSERT(status, "Out of memory");


	for (i = 0; i < column_->numValues_; i++)
	{
		if (numBins_ == 1)
		{
			binData_[i] = (tlBin::tlBinId) 0;
		} else
		{
			fraction = ((column_->data_[i].dval_ - minValue) / range);
			if (fraction == 1.0)
			{
				binId = numBins_ - 1;
			} else
			{
				binId = (int) (fraction * (double) numBins_);
				MSG_ASSERT(binId < numBins_, "bin index overflow");
			}

			binData_[i] = (tlBin::tlBinId) binId;
		}
	}

	boundsFlags_ |= (FLAG_LOCK);
	{
		if ( !  BOUNDS_LOCKED )
			return 0;

		if ( ! BOUNDS_DIRTY )
			return 1;


		cleanAllocatedStrings__();

		MSG_ASSERT(binLoBound_ == NULL, "bound already allocated");
		binLoBound_ = (tlValueStorage *)
					ckalloc(numBins_ * sizeof(tlValueStorage));
		MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
		memset(binLoBound_, 0x0, numBins_ * sizeof(tlValueStorage));

		MSG_ASSERT(binHiBound_ == NULL, "bound already allocated");
		binHiBound_ = (tlValueStorage *)
					ckalloc(numBins_ * sizeof(tlValueStorage));
		MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
		memset(binHiBound_, 0x0, numBins_ * sizeof(tlValueStorage));

		allocateRealDataBinBounds__();

		for (i = 0; i < numBins_; i++)
		{
			binLoBound_[i].dval_ = (tlReal) (minValue + (i * binRange));
			binHiBound_[numBins_ - (i + 1)].dval_ =
					(tlReal) (maxValue - (i * binRange));
		}

		boundsFlags_ &= (~FLAG_DIRTY);
	}

	return 1;
}


/**
 * Divide the continuous range into bins based on dividing the
 * total range by the number of bins.
 */
int
tlBin::binDataString__(
		tlErrorManager *err
    )
{
	tlSrString lastValue, thisValue;
	int nUniqueValues;
	int binId;
	int sortIndex, unsortIndex;
	int status;

	(void) err;

	boundsFlags_ |= (FLAG_DIRTY);

	qsort(sortedData_, column_->numValues_,
					sizeof(tlSrValue *), compare_String);

	nUniqueValues = 1;
	unsortIndex = (int) (sortedData_[0] - column_->data_);
	lastValue = *column_->data_[unsortIndex].str_;
	/** count up the number of unique values (the data is already sorted) */
	for (sortIndex = 1; sortIndex < column_->numValues_; sortIndex++)
	{

		/**
		 * Indices here are complicated because we want to got
		 * through the data in sorted order.
		 * sortIndex   - index into sorted array
		 * unsortIndex - index into unsorted array, calculated
		 * 				 by taking the offset into the sorted array
		 */
		unsortIndex = (int) (sortedData_[sortIndex] - column_->data_);
		thisValue = *column_->data_[unsortIndex].str_;
		if (thisValue != lastValue)
		{
			lastValue = thisValue;
			nUniqueValues++;
		}
	}

	/** we cannot have less than one bin */
	if (nUniqueValues <= 0)
	{
		nUniqueValues = 1;
	}

	numBins_ = nUniqueValues;

	/** allocate space for all of the bin ids */
	status = listCheckSize(column_->numValues_,
			(void **) &binData_,
			&binDataBlocks_,
			BLOCKSIZE,
			sizeof(tlBinId));
	MSG_ASSERT(status, "Out of memory");


	/** set up to fix bounds at same time */
	boundsFlags_ |= (FLAG_LOCK);

	if ( ! BOUNDS_LOCKED )
		return 0;

	if ( ! BOUNDS_DIRTY )
		return 1;

	cleanAllocatedStrings__();

	MSG_ASSERT(binLoBound_ == NULL, "bound already allocated");
	binLoBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
	memset(binLoBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	MSG_ASSERT(binHiBound_ == NULL, "bound already allocated");
	binHiBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
	memset(binHiBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	allocateRealDataBinBounds__();



	/** now loop, assigning bin ids and bounds */
	binId = 0;
	unsortIndex = (int) (sortedData_[0] - column_->data_);
	lastValue = *column_->data_[unsortIndex].str_;
	binData_[0] = (tlBin::tlBinId) binId;
	binLoBound_[0].str_ = column_->data_[unsortIndex].str_;

	/** use the same algorithm to assign ids to these bins */
	for (sortIndex = 1; sortIndex < column_->numValues_; sortIndex++)
	{
		unsortIndex = (int) (sortedData_[sortIndex] - column_->data_);
		thisValue = *column_->data_[unsortIndex].str_;

		/**
		 * if the value has changed, store the highest instance of
		 * the previous value as the current bins high bound; store
		 * the lowest version of the new value as the new bin's lower
		 * bound, and update the bin id
		 */
		if (thisValue != lastValue)
		{
			binHiBound_[binId].str_ = sortedData_[sortIndex-1]->str_;
			MSG_ASSERT(((binId + 1) < numBins_), "bin id out of range");
			binLoBound_[binId+1].str_ = sortedData_[sortIndex]->str_;

			lastValue = thisValue;
			binId++;
		}
		binData_[unsortIndex] = (tlBin::tlBinId) binId;
	}
	MSG_ASSERT(((binId + 1) == numBins_), "bin id mismatch");
	unsortIndex = (int) (sortedData_[column_->numValues_-1] - column_->data_);
	binHiBound_[numBins_-1].str_ = column_->data_[unsortIndex].str_;

	boundsFlags_ &= (~FLAG_DIRTY);

	return 1;
}

int
tlBin::binDataEqualRange(int numBins, tlErrorManager *err)
{
	int i;

	if (column_->type_ == tlSrValue::REAL)
	{
		return binDataEqualRangeFloat__(
				numBins,
				err);
	}


	/**
	 * We need a sorted list of data in order to do binning
	 */
	sortedData_ = (tlValueStorage **)
				ckalloc(sizeof(tlValueStorage *) * column_->numValues_);
	MSG_ASSERT(sortedData_ != NULL, "Out of memory");
	for (i = 0; i < column_->numValues_; i++)
	{
		sortedData_[i] = &column_->data_[i];
	}


	if (column_->type_ == tlSrValue::STRING)
	{
		qsort(sortedData_, column_->numValues_, sizeof(tlSrValue *),
				compare_String);

		return binDataString__(err);

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		qsort(sortedData_, column_->numValues_, sizeof(tlSrValue *),
				compare_Integer);

		return binDataEqualRangeOrdinal__(
				compare_Integer,
				getBinIndex_Integer,
				numBins,
				err);
	}

	return 0;
}

int
tlBin::allocateBounds__(int newNumBins)
{
	boundsFlags_ |= (FLAG_LOCK);

	numBins_ = newNumBins;

	cleanAllocatedStrings__();

	MSG_ASSERT(binData_ == NULL, "bin data already set");



	MSG_ASSERT(binLoBound_ == NULL, "bound already allocated");
	binLoBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
	memset(binLoBound_, 0x0, numBins_ * sizeof(tlValueStorage));


	MSG_ASSERT(binHiBound_ == NULL, "bound already allocated");
	binHiBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
	memset(binHiBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	allocateRealDataBinBounds__();


	return 1;
}


int
tlBin::computeBounds__()
{
	int sortIndex, unsrtIndex, lastIndex = (-1);
	int lastBin = (-1);
	int beginIndex = 0;

	if ( ! BOUNDS_LOCKED )
		return 0;

	if ( ! BOUNDS_DIRTY )
		return 1;


	cleanAllocatedStrings__();

	MSG_ASSERT(binLoBound_ == NULL, "bound already allocated");
	binLoBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binLoBound_ != NULL, "Out of memory");
	memset(binLoBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	MSG_ASSERT(binHiBound_ == NULL, "bound already allocated");
	binHiBound_ = (tlValueStorage *)
				ckalloc(numBins_ * sizeof(tlValueStorage));
	MSG_ASSERT(binHiBound_ != NULL, "Out of memory");
	memset(binHiBound_, 0x0, numBins_ * sizeof(tlValueStorage));

	allocateRealDataBinBounds__();


	for (sortIndex = 0; sortIndex < column_->numValues_; sortIndex++)
	{

		unsrtIndex = (int) (sortedData_[sortIndex] - column_->data_);

		if (binData_[unsrtIndex] != lastBin)
		{
			if (lastBin != (-1))
			{

				/** save the group we have found */
				MSG_ASSERT(binData_[lastIndex] < numBins_,
								"bin id out of range");
				binLoBound_[ binData_[lastIndex] ] = 
								column_->data_[beginIndex];
				binHiBound_[ binData_[lastIndex] ] = 
								column_->data_[lastIndex];
			}

			/** set the "begin" marker */
			beginIndex = unsrtIndex;
			lastBin = binData_[unsrtIndex];
		}
		lastIndex = unsrtIndex;
	}

	/** now save the last element */
	MSG_ASSERT(binData_[lastIndex] < numBins_, "bin id out of range");
	binLoBound_[ binData_[lastIndex] ] = column_->data_[beginIndex];
	binHiBound_[ binData_[lastIndex] ] = column_->data_[lastIndex];

	if (column_->type_ == tlSrValue::REAL)
	{
		resolveGapsBetweenRealBins__();
	}

	boundsFlags_ &= (~FLAG_DIRTY);
	return 1;
}

int
tlBin::resolveGapsBetweenRealBins__()
{
	double midpoint;
	int i;

	allocateRealDataBinBounds__();

	/** store the current bounds in the "actual" bounds and close gaps */
	binActualLoBound_[ 0 ] = binLoBound_[ 0 ].dval_;
	binActualHiBound_[ numBins_ - 1 ] = binHiBound_[ numBins_ - 1 ].dval_;

	for (i = 1; i < numBins_; i++)
	{
		binActualLoBound_[ i ] = binLoBound_[ i ].dval_;
		binActualHiBound_[ i-1 ] = binHiBound_[ i-1 ].dval_;

		midpoint = (binLoBound_[ i ].dval_ + binHiBound_[ i-1 ].dval_) / 2.0;
		binLoBound_[ i ].dval_ = (tlReal) midpoint;
		binHiBound_[ i-1 ].dval_ = (tlReal) midpoint;
	}

	return 1;
}

tlBin::tlBinId
tlBin::getBinId(const tlValueStorage *data, int useNearest) const
{
	int cmpLo, cmpHi;
	int i;

		/** print out the end of the group we have found */
	if (column_->type_ == tlSrValue::STRING)
	{
		for (i = 0; i < numBins_; i++)
		{
			cmpLo = strcmp(
						data->str_->getValue(),
						binLoBound_[i].str_->getValue()
					);
			if (cmpLo <= 0)
			{
				cmpHi = strcmp(
						binHiBound_[i].str_->getValue(),
						data->str_->getValue()
					);
				if (cmpHi >= 0)
				{
					return i;
				}
			}
		}

	} else if (column_->type_ == tlSrValue::INTEGER)
	{

		/**
		 * if we are rounding to the nearest bin for out-of-range
		 * values, check that we do so
		 */
		if (useNearest) {
			if (data->ival_ < binLoBound_[0].ival_)
				return 0;

			if (data->ival_ > binHiBound_[numBins_-1].ival_)
				return numBins_-1;
		}

		for (i = 0; i < numBins_; i++)
		{
			if ((binLoBound_[i].ival_ <= data->ival_)
					&& (binHiBound_[i].ival_ >= data->ival_))
			{
				return i;
			}
		}

	} else if (column_->type_ == tlSrValue::REAL)
	{

		/**
		 * if we are rounding to the nearest bin for out-of-range
		 * values, check that we do so
		 */
		if (useNearest) {
			if (data->dval_ < binLoBound_[0].dval_)
				return 0;

			if (data->dval_ > binHiBound_[numBins_-1].dval_)
				return numBins_-1;
		}

		for (i = 0; i < numBins_; i++)
		{
			if ((binLoBound_[i].dval_ <= data->dval_)
					&& (binHiBound_[i].dval_ >= data->dval_))
			{
				return i;
			}
		}
	}

	return (-1);
}

tlBin::tlBinId
tlBin::getBinId(int index) const
{
	if (index >= 0 && index < column_->numValues_)
		return binData_[index];

	return (-1);
}


void
tlBin::updateLastDataValue()
{
	tlBinId newId;
	int status;

	status = listCheckSize(column_->numValues_,
			(void **) &binData_,
			&binDataBlocks_,
			BLOCKSIZE,
			sizeof(tlBinId));
	MSG_ASSERT(status, "Out of memory");

	newId = getBinId(&column_->data_[column_->numValues_]);
	MSG_ASSERT(newId >= 0, "Bad bin id");
	binData_[column_->numValues_] = newId;
}

int
tlBin::loadFeatureDescription(FILE *ofp)
{
	int status;
	tokenizer *t;

	t = tknGetTokenizer(ofp);
	status = loadFeatureDescription(t);
	tknDeleteTokenizer(t);

	return status;
}

int
tlBin::loadFeatureDescription(
		tokenizer *t,
		attValList *parsingVariables,
		tlErrorManager *err
    )
{
	token *token;
	double dVal, diff;
	int numBinsRead;
	int oldMode = 0;
	int i;


	token = tknGetToken(t);

	if (token->type_ != TT_STRING)
	{
		if (err != NULL)
		{
			err->addError("Expected column name at %d",
					tknGetLineNo(t));
		}
		return 0;
	}

	if (column_->getName() != token->data_.strptr_)
	{
		if (err != NULL)
		{
			err->addError("Found name '%s', expected name '%s' at %d",
					token->data_.strptr_,
					column_->getName().getValue(),
					tknGetLineNo(t));
		}
		return 0;
	}


	token = tknGetToken(t);
	if (token->type_ != TT_IDENTIFIER)
	{
		/** older mode data files -- pick up type from bin rows */
		oldMode = 1;
		tknPushToken(t);
	} else
	{

		if (tknCompareNoCase(token, "INTEGER") == 0)
		{
			column_->setType(tlSrValue::INTEGER);

		} else if (tknCompareNoCase(token, "REAL") == 0)
		{
			column_->setType(tlSrValue::REAL);

		} else if (tknCompareNoCase(token, "STRING") == 0)
		{
			column_->setType(tlSrValue::STRING);
		} else 
		{
			err->addError("Unknown column type '%s' at line %d",
					token->data_.strptr_, tknGetLineNo(t));
			return 0;
		}
	}


	if ( ! tlSParser::parseNumber(&dVal, t, parsingVariables, err) )
	{
		if (err != NULL)
		{
			err->addError("Cannot read number of bins at line %d",
						tknGetLineNo(t));
		}
		return 0;
	}


	numBinsRead = (int) dVal;
	diff = numBinsRead - dVal;
	if (diff != 0)
	{
		if (err != NULL)
		{
			err->addError("Non-integer number of bins at line %d",
						tknGetLineNo(t));
		}
		return 0;
	}


	token = tknGetToken(t);

	if (token->type_ != TT_IDENTIFIER)
	{
		tknPushToken(t);
	} else
	{
		if (tknCompareNoCase(token, "Label") == 0)
		{
			column_->setLabel(1);
		}
	}



	if ( ! allocateBounds__(numBinsRead) )
	{
		if (err != NULL)
		{
			err->addError("Failure allocating %d bins", numBinsRead);
		}
		return 0;
	}


	/**
	 * now iterate over the bins and read the values in
	 */
	for (i = 0; i < numBins_; i++)
	{

		token = tknGetToken(t);
		if (token->type_ != TT_INTEGER)
		{
			if (err != NULL)
			{
				err->addError("Expected bin id at line %d",
						tknGetLineNo(t));
			}
			return 0;
		}
		if ( token->data_.ival_ != i)
		{
			if (err != NULL)
			{
				err->addError("Bin id mismatch at line %d",
						tknGetLineNo(t));
			}
			return 0;
		}


		if (oldMode == 1)
		{
			token = tknGetToken(t);
			if (token->type_ != TT_IDENTIFIER)
			{
				/** older mode data files -- pick up type from bin rows */
				oldMode = 1;
				tknPushToken(t);
			} else
			{

				if (tknCompareNoCase(token, "INTEGER") == 0)
				{
					column_->setType(tlSrValue::INTEGER);

				} else if (tknCompareNoCase(token, "REAL") == 0)
				{
					column_->setType(tlSrValue::REAL);
					allocateRealDataBinBounds__();

				} else if (tknCompareNoCase(token, "STRING") == 0)
				{
					column_->setType(tlSrValue::STRING);
				} else 
				{
					err->addError("Unknown column type '%s' at line %d",
						token->data_.strptr_, tknGetLineNo(t));
					return 0;
				}
			}
		}

		token = tknGetToken(t);
		if (token->type_ != ':')
		{
			if (err != NULL)
			{
				err->addError("Expected ':' at line %d",
						tknGetLineNo(t));
			}
			return 0;
		}


		token = tknGetToken(t);
		if (token->type_ != '[')
		{
			if (err != NULL)
			{
				err->addError("Expected '[' at line %d",
						tknGetLineNo(t));
			}
			return 0;
		}


		if (column_->type_ == tlSrValue::STRING)
		{

			/** read in the low bound */
			token = tknGetToken(t);
			if (token->type_ != TT_STRING)
			{
				if (err != NULL)
				{
					err->addError("Cannot read low bound at line %d",
								tknGetLineNo(t));
				}
				return 0;
			}
			binLoBound_[i].str_ = new tlSrString(token->data_.strptr_);
			MSG_ASSERT(binLoBound_[i].str_ != NULL, "Out of memory");
			allocStrings_->add( binLoBound_[i].str_ );

		} else
		{
			/** read in the low bound */
			if ( ! tlSParser::parseNumber(&dVal, t,
							parsingVariables, err) )
							{
				if (err != NULL)
				{
					err->addError("Cannot read low bound at line %d",
								tknGetLineNo(t));
				}
				return 0;
			}
			if (column_->type_ == tlSrValue::INTEGER)
			{
				binLoBound_[i].ival_ = (int) dVal;
			} else if (column_->type_ == tlSrValue::REAL)
			{
				binLoBound_[i].dval_ = (float) dVal;
			}
		}

		token = tknGetToken(t);
		tknPushToken(t);

		if (token->type_ == ']')
		{

			if (column_->type_ == tlSrValue::STRING)
			{
				binHiBound_[i].str_ =
							new tlSrString(*binLoBound_[i].str_);
				MSG_ASSERT(binHiBound_[i].str_ != NULL,
								"Out of memory");
				allocStrings_->add( binHiBound_[i].str_ );
			} else
			{
				if (column_->type_ == tlSrValue::INTEGER)
				{
					binHiBound_[i].ival_ = binLoBound_[i].ival_;

				} else if (column_->type_ == tlSrValue::REAL)
				{
					binHiBound_[i].dval_ = binLoBound_[i].dval_;

				}
			}

		} else
		{

			if (column_->type_ == tlSrValue::STRING)
			{
				/** read in the high bound */
				token = tknGetToken(t);
				if (token->type_ != TT_STRING)
				{
					if (err != NULL)
					{
						err->addError(
								"Cannot read high bound at line %d",
								tknGetLineNo(t));
					}
					return 0;
				}
				binHiBound_[i].str_ =
									new tlSrString(token->data_.strptr_);
				MSG_ASSERT(binHiBound_[i].str_ != NULL,
								"Out of memory");
				allocStrings_->add( binHiBound_[i].str_ );


			} else
			{

				/** read in the high bound */
				if ( ! tlSParser::parseNumber(&dVal, t,
								parsingVariables, err) )
				{
					if (err != NULL)
					{
						err->addError(
								"Cannot read high bound at line %d",
								tknGetLineNo(t));
					}
					return 0;
				}
				if (column_->type_ == tlSrValue::INTEGER)
				{
					binHiBound_[i].ival_ = (int) dVal;
				} else if (column_->type_ == tlSrValue::REAL)
				{
					binHiBound_[i].dval_ = (float) dVal;
				}
			}
		}


		token = tknGetToken(t);
		if (token->type_ != ']')
		{
			if (err != NULL)
			{
				err->addError("Expected ']' at line %d",
						tknGetLineNo(t));
			}
			return 0;
		}


		/** now check for actual bound pair */
		if (column_->type_ == tlSrValue::REAL)
		{
			token = tknGetToken(t);
			if (token->type_ == '{') { /* } */

				/** read in the low real bound */
				if ( ! tlSParser::parseNumber(&dVal, t, parsingVariables, err) )
				{
					if (err != NULL)
					{
						err->addError("Cannot read low bound at line %d",
									tknGetLineNo(t));
					}
					return 0;
				}
				binActualLoBound_[i] = (float) dVal;

				/** read in the high real bound */
				if ( ! tlSParser::parseNumber(&dVal, t, parsingVariables, err) )
				{
					if (err != NULL)
					{
						err->addError("Cannot read low bound at line %d",
									tknGetLineNo(t));
					}
					return 0;
				}
				binActualHiBound_[i] = (float) dVal;

				token = tknGetToken(t); /* { */
				if (token->type_ != '}')
				{
					if (err != NULL) {		/* { */
						err->addError("Expected '}' at line %d",
								tknGetLineNo(t));
					}
					return 0;
				}

			} else
			{
				tknPushToken(t);
				binActualLoBound_[i] = binLoBound_[i].dval_;
				binActualHiBound_[i] = binHiBound_[i].dval_;
			}
		}
	}

	boundsFlags_ |= FLAG_LOCK;
	boundsFlags_ &= (~FLAG_DIRTY);


	return 1;
}

int
tlBin::storeFeatureDescription(int indent, FILE *ofp)
{
	int i;

	computeBounds__();

	for (i = 0; i < numBins_; i++)
	{
		if (column_->type_ == tlSrValue::STRING)
		{
			if (*binLoBound_[i].str_ == *binHiBound_[i].str_)
			{
				fprintf(ofp, "%*s%3d : [ %s ]\n", indent, "",
						i,
						binLoBound_[i].str_->getQuoted().getValue());
			} else
			{
				fprintf(ofp, "%*s%3d : [ %s %s ]\n", indent, "",
						i,
						binLoBound_[i].str_->getQuoted().getValue(),
						binHiBound_[i].str_->getQuoted().getValue());
			}

		} else if (column_->type_ == tlSrValue::INTEGER)
		{
			if (binLoBound_[i].ival_ == binHiBound_[i].ival_)
			{
				fprintf(ofp, "%*s%3d : [ %d ]\n", indent, "",
						i,
						binLoBound_[i].ival_);
			} else
			{
				fprintf(ofp, "%*s%3d : [ %d %d ]\n", indent, "",
						i,
						binLoBound_[i].ival_,
						binHiBound_[i].ival_);
			}

		} else if (column_->type_ == tlSrValue::REAL)
		{
			if (binLoBound_[i].dval_ == binHiBound_[i].dval_)
			{
				fprintf(ofp, "%*s%3d : [ %10.6f ]", indent, "",
						i,
						binHiBound_[i].dval_);
			} else
			{
				fprintf(ofp, "%*s%3d : [ %10.6f %10.6f ]",
						indent, "",
						i,
						binLoBound_[i].dval_,
						binHiBound_[i].dval_);
			}
			if ((binLoBound_[i].dval_ != binActualLoBound_[i]) ||
							(binHiBound_[i].dval_ != binActualHiBound_[i]))
			{
				fprintf(ofp, " { %10.6f %10.6f }",
						binActualLoBound_[i],
						binActualHiBound_[i]);
			}

			fprintf(ofp, "\n");
		}
	}

	return !ferror(ofp);
}

int
tlBin::dump(FILE *ofp) const
{
	int *binCounts;
	int i;

	binCounts = (int *) ckalloc(sizeof(int) * numBins_);
	memset(binCounts, 0, sizeof(int) * numBins_);

	for (i = 0; i < column_->numValues_; i++)
	{
		if (!column_->isNilValue(i))
		binCounts[binData_[i]]++;
	}

	fprintf(ofp, "   Range for %d bins ", numBins_);
	if (column_->type_ == tlSrValue::STRING)
	{
		fprintf(ofp, "[%s:%s]\n",
				binLoBound_[0].str_->getValue(),
				binHiBound_[numBins_-1].str_->getValue());


	} else if (column_->type_ == tlSrValue::REAL)
	{
		fprintf(ofp, "[%.4f:%.4f]\n",
				binLoBound_[0].dval_,
				binHiBound_[numBins_-1].dval_);

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		fprintf(ofp, "[%d:%d]\n",
				binLoBound_[0].ival_,
				binHiBound_[numBins_-1].ival_);
	}


	/** now the bin bounds */
	fprintf(ofp, "     Bounds for %d bins:\n", numBins_);
	if (column_->type_ == tlSrValue::STRING)
	{
		for (i = 0; i < numBins_; i++)
		{
			fprintf(ofp, "     Bin %3d [%s:%s]  count %d\n",
					i,
					binLoBound_[i].str_->getValue(),
					binHiBound_[i].str_->getValue(),
					binCounts[i]);
		}

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		for (i = 0; i < numBins_; i++)
		{
			fprintf(ofp, "     Bin %3d [%d:%d]  count %d\n",
					i,
					binLoBound_[i].ival_,
					binHiBound_[i].ival_,
					binCounts[i]);
		}

	} else if (column_->type_ == tlSrValue::REAL)
	{
		for (i = 0; i < numBins_; i++)
		{
			fprintf(ofp, "     Bin %3d [%.4f:%.4f]  count %d\n",
					i,
					binLoBound_[i].dval_,
					binHiBound_[i].dval_,
					binCounts[i]);
		}
	}

	ckfree(binCounts);


	/** now the bin data */
	fprintf(ofp, "   Data classifications:\n");
	if (binData_ != NULL)
	{
		for (i = 0; i < column_->numValues_; i++)
		{
			if (column_->isNilValue(i))
				fprintf(ofp,
						"   %3d : Bin %3d Nil \n",
						i, binData_[i]);

			else if (column_->type_ == tlSrValue::STRING)
				fprintf(ofp,
					"   %3d : Bin %3d String : '%s'  [%s:%s]\n",
						i, binData_[i],
						column_->data_[i].str_->getValue(),
						binLoBound_[binData_[i]].str_->getValue(),
						binHiBound_[binData_[i]].str_->getValue());

			else if (column_->type_ == tlSrValue::INTEGER)
				fprintf(ofp,
					"   %3d : Bin %3d Int : (%d)  [%d:%d]\n",
						i, binData_[i],
						column_->data_[i].ival_,
						binLoBound_[binData_[i]].ival_,
						binHiBound_[binData_[i]].ival_);

			else if (column_->type_ == tlSrValue::REAL)
				fprintf(ofp,
					"   %3d : Bin %3d  Real : (%0.4f)  [%.4f:%.4f]\n",
						i, binData_[i],
						column_->data_[i].dval_,
						binLoBound_[binData_[i]].dval_,
						binHiBound_[binData_[i]].dval_);

			else
				fprintf(ofp,
					"   %3d : Bin %3d  BAD TYPE (%d)\n",
						i, binData_[i], column_->type_);
		}

	}

	return !ferror(ofp);
}

tlSrString
tlBin::getNiceName(int index)
{
	int binIndex;


	if (niceNames_ == NULL)
	{
		niceNames_ = (tlSrString **) ckalloc(sizeof(tlSrString *) * numBins_);
		memset(niceNames_, 0, sizeof(tlSrString *) * numBins_);
	}

	MSG_ASSERT(index < numBins_, "tlBin Name index out of range");

	if (niceNames_[index] != NULL) {
		return *(niceNames_[index]);
	}

	// must allocate the string
	niceNames_[index] = new tlSrString();


	if (binData_ == NULL)
	{
		binIndex = index;
	} else {
		binIndex = binData_[index];
	}


	if (column_->isNilValue(index))
	{
		niceNames_[index]->sprintf("NIL");

	} else if (column_->type_ == tlSrValue::STRING)
	{
		if ((*binLoBound_[binIndex].str_)
					== (*binHiBound_[binIndex].str_))
		{
			niceNames_[index]->sprintf("%s",
					binHiBound_[binIndex].str_->getValue());
		} else
		{
			niceNames_[index]->sprintf("[%s:%s]",
				binLoBound_[binIndex].str_->getValue(),
				binHiBound_[binIndex].str_->getValue());
		}

	} else if (column_->type_ == tlSrValue::REAL)
	{
		if (binLoBound_[binIndex].dval_
					== binHiBound_[binIndex].dval_)
			niceNames_[index]->sprintf("%.4f", binHiBound_[binIndex].dval_);
		else
			niceNames_[index]->sprintf("[%.4f:%.4f]",
				binLoBound_[binIndex].dval_,
				binHiBound_[binIndex].dval_);

	} else if (column_->type_ == tlSrValue::INTEGER)
	{
		if (binLoBound_[binIndex].ival_
					== binHiBound_[binIndex].ival_)
			niceNames_[index]->sprintf("%d", binHiBound_[binIndex].ival_);
		else
			niceNames_[index]->sprintf("[%d:%d]",
				binLoBound_[binIndex].ival_,
				binHiBound_[binIndex].ival_);
	}

	return *(niceNames_[index]);
}

