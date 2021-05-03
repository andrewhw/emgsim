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
 * $Id: tlBinTable.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "tokens.h"

#include "tlBinTable.h"
#include "tlColumn.h"
#include "tlTuple.h"
#include "tlBin.h"
#include "tlErrorManager.h"

tlBinTable::tlBinTable()
{
	binBlocks_ = 0;
	bin_ = NULL;
	binsCreated_ = 0;
	binStrategy_ = tlBinTable::MME_GROUP_UNIQUE;
}

tlBinTable::tlBinTable(tlBinTable *other)
    : tlTable(other)
{
	int status;
	int i;

	binBlocks_ = 0;
	bin_ = NULL;
	binsCreated_ = other->binsCreated_;

	if (other->bin_ == NULL)
	{
		bin_ = NULL;
	} else
	{

		status = listCheckSize(
				other->numColumns_,
				(void **) &bin_,
				&binBlocks_,
				1,
				sizeof(tlBin *));
		MSG_ASSERT(status, "Out of memory");


		for (i = 0; i < numColumns_; i++)
		{
			bin_[i] = new tlBin(column_[i], other->bin_[i]);
			MSG_ASSERT(bin_[i] != NULL, "Out of memory");
			bin_[i]->ref();
		}
	}
}

tlBinTable::~tlBinTable()
{
	clear();
}

void
tlBinTable::clear()
{
	int i;

	if (bin_ != NULL)
	{
		for (i = 0; i < numColumns_; i++)
		{
			if (bin_[i] != NULL)
			{
				bin_[i]->unref();
			}
		}
		ckfree(bin_);
		bin_ = NULL;
	}
	binBlocks_ = 0;
	binsCreated_ = 0;

	tlTable::clear();
}


int
tlBinTable::calculateVariances()
{
	int i;

	if (sigmaSquared_ != NULL)
	{
		ckfree(sigmaSquared_);
		ckfree(mu_);
	}

	sigmaSquared_ = (double *) ckalloc(sizeof(double) * numColumns_);
	MSG_ASSERT(sigmaSquared_ != NULL, "Out of memory");
	mu_ = (double *) ckalloc(sizeof(double) * numColumns_);
	MSG_ASSERT(mu_ != NULL, "Out of memory");

	for (i = 0; i < numColumns_; i++)
	{
		bin_[i]->calculateVariance(&sigmaSquared_[i], &mu_[i]);
	}
	return 1;
}

int
tlBinTable::createBins(
		tlBinTable::BinStrategy strategy,
		int masterNumBins,
		tlErrorManager *err,
		int nColumnsInBinOverrideList,
		tlBinTable::BinIndexPair *overrideNumBinList
    )
{
	int numBinsInThisColumn, i, j;

	MSG_ASSERT(numColumns_ > 0, "no data");
	MSG_ASSERT(maxRows_ > 0, "no data");

	binStrategy_ = strategy;


	for (i = 0; i < numColumns_; i++)
	{
		numBinsInThisColumn = masterNumBins;
		for (j = 0; j < nColumnsInBinOverrideList; j++)
		{
			if (getColumnName(i) == overrideNumBinList[j].name_)
			{
				numBinsInThisColumn = overrideNumBinList[j].nBins_;
				break;
			}
		}

		if (strategy == tlBinTable::MME_GROUP_UNIQUE)
		{

			if ( ! bin_[i]->binDataMMEGroupUnique(numBinsInThisColumn, err))
			{
				return 0;
			}

		} else if (strategy == tlBinTable::MME_IGNORE_UNIQUE)
		{

			if ( ! bin_[i]->binDataMMEIgnoreUnique(numBinsInThisColumn, err))
			{
				return 0;
			}

		} else if (strategy == tlBinTable::EQUAL_BIN_RANGE)
		{

			/**
			 * equal range is simple, we are simply dividing
			 * up into numBinsInThisColumn bins
			 */
			if ( ! bin_[i]->binDataEqualRange(numBinsInThisColumn, err))
			{
				return 0;
			}
		} else
		{
			if (err != NULL)
			{
				err->addError("Invalid strategy supplied to createBins()");
			}
			return 0;
		}
	}


	binsCreated_ = 1;
	return 1;
}

void
tlBinTable::addColumn(tlSrString name, int type)
{
	int status;

	tlTable::addColumn(name, type);

	status = listCheckSize(
			numColumns_,
			(void **) &bin_,
			&binBlocks_,
			1,
			sizeof(tlBin *));
	MSG_ASSERT(status, "Out of memory");

	MSG_ASSERT(bin_[numColumns_ - 1] == NULL, "Column already added");
	bin_[numColumns_ - 1] = new tlBin(column_[numColumns_ - 1]);
	MSG_ASSERT(bin_[numColumns_ - 1] != NULL, "Out of memory");
	bin_[numColumns_ - 1]->ref();
}

void
tlBinTable::addColumn(const char *name, int type)
{
	int status;

	tlTable::addColumn(name, type);

	status = listCheckSize(
			numColumns_,
			(void **) &bin_,
			&binBlocks_,
			1,
			sizeof(tlColumn *));
	MSG_ASSERT(status, "Out of memory");

	MSG_ASSERT(bin_[numColumns_ - 1] == NULL, "Column already added");
	bin_[numColumns_ - 1] = new tlBin(column_[numColumns_ - 1]);
	MSG_ASSERT(bin_[numColumns_ - 1] != NULL, "Out of memory");
	bin_[numColumns_ - 1]->ref();
}

void
tlBinTable::addRow(tlTuple *row)
{
	int i;

	tlTable::addRow(row);

	if ( ! binsCreated_)
		return;

	for (i = 0; i < numColumns_; i++)
	{
		bin_[i]->updateLastDataValue();
	}
}

int
tlBinTable::loadFeatureDescriptions(FILE *ofp)
{
	(void) ofp;

	MSG_FAIL("loadFeatureDescriptions not implemented");
	return 0;
}

int
tlBinTable::getRowBinIds(tlBin::tlBinId *idVector, tlTuple *row) const
{
	int i;

	for (i = 0; i < numColumns_; i++)
	{

		if ( row->isNil(i) )
		{
			idVector[i] = tlBin::nilId;

		} else
		{
			MSG_ASSERT(row->getValue(i).data()->getType()
							== getColumn(i)->getType(),
							"Type mismatch");
			idVector[i] = bin_[i]->getBinId(
							row->getValue(i).data()->storage(),
						1
					);
		}
	}

	return 1;
}

int
tlBinTable::getRowBinIds(tlBin::tlBinId *idVector, int rowIndex) const
{
	int i;

	for (i = 0; i < numColumns_; i++)
	{

		if ( column_[i]->isNilValue(rowIndex) )
		{
			idVector[i] = tlBin::nilId;

		} else
		{
			idVector[i] = bin_[i]->getBinId(
							&column_[i]->data_[rowIndex]
					);
		}
	}

	return 1;
}

int
tlBinTable::fillInNilDataFromBinValues(
		tlTuple *row,
		tlBin::tlBinId *idVector,
		int *nFilledIn,
		int *nilDataLocations
    ) const
{
	int i;

	MSG_ASSERT(numColumns_ == row->getNumValues(), "row/table width mismatch");
	if (nFilledIn != NULL)
		*nFilledIn = 0;

	for (i = 0; i < numColumns_; i++)
	{

		/**
		 * Only deal with values which are nil and for which we
		 * have a replacement value
		 */
		if (row->isNil(i) && (idVector[i] != tlBin::nilId))
		{
			if (nFilledIn != NULL && nilDataLocations != NULL)
			{
				nilDataLocations[(*nFilledIn)++] = i;
			}

			if (column_[i]->getType() == tlSrValue::INTEGER)
			{
				int lo, hi, avg;

				lo = bin_[i]->getLoBoundValue( idVector[i] )->ival_;
				hi = bin_[i]->getHiBoundValue( idVector[i] )->ival_;
				avg = lo + ((hi - lo) / 2);

				row->setValue(i, avg);

			} else if (column_[i]->getType() == tlSrValue::REAL)
			{
				tlReal lo, hi, avg;

				lo = bin_[i]->getLoBoundValue( idVector[i] )->dval_;
				hi = bin_[i]->getHiBoundValue( idVector[i] )->dval_;
				avg = (float) (lo + ((hi - lo) / 2.0));

				row->setValue(i, avg);

			} else if (column_[i]->getType() == tlSrValue::STRING)
			{
				row->setValue(i,
						*bin_[i]->getLoBoundValue( idVector[i] )->str_
					);

			}
		}
	}

	return 1;
}

int
tlBinTable::saveRowAsBinIds(FILE *ofp, int index, int colWidth) const
{
	int i;

	for (i = 0; i < getNumColumns(); i++)
	{
		if (i != 0)
		{
			fputs(", ", ofp);
		}
		if (column_[i]->isNilValue(index))
			fprintf(ofp, "%*s", colWidth, "NIL");
		else
			fprintf(ofp, "%*d", colWidth, bin_[i]->getBinId(index));
	}
	fputc('\n', ofp);

	return !ferror(ofp);
}

int
tlBinTable::saveDataAsBinIds(FILE *ofp) const
{
	int colWidth, tmpWidth;
	int sawSpace = 0;
	int i;

	/** figure out data width */
	colWidth = getColumn(0)->getName().getLength();
	for (i = 1; i < getNumColumns(); i++)
	{
		tmpWidth = getColumn(i)->getName().getLength();
		if (colWidth < tmpWidth)
			colWidth = tmpWidth;
		if (getColumn(i)->getName()
					.containsAny(TOK_NAME_PROTECT_CHARS) >= 0)
			sawSpace = 1;
	}

	/** print out headers */
	for (i = 0; i < getNumColumns(); i++)
	{
		if (i != 0)
		{
			fputs(", ", ofp);
		}
		if (sawSpace)
		{
			tmpWidth = colWidth
					- (getColumn(i)->getName().getLength() + 2);
			if (tmpWidth > 0)
			{
				fprintf(ofp, "%*s\"%s\"", tmpWidth, "",
						getColumn(i)->getName().getValue());
			} else
			{
				fprintf(ofp, "\"%s\"",
						getColumn(i)->getName().getValue());
			}
		} else
		{
			fprintf(ofp, "%*s", colWidth,
						getColumn(i)->getName().getValue());
		}
	}
	fputc('\n', ofp);

	for (i = 0; i < maxRows_; i++)
	{
		if ( ! saveRowAsBinIds(ofp, i, colWidth) )
			return 0;
	}

	return !ferror(ofp);
}

int
tlBinTable::storeConfigFeatureDescriptions(
		int indent,
		FILE *ofp
    ) const
{
	tlSrString columnName;
	const char *strategyString = "<unknown>";
	int i;

	if (binStrategy_ == tlBinTable::MME_GROUP_UNIQUE)
	{
		strategyString = "Maximum Marginal Entropy - Group Unique Values";

	} else if (binStrategy_ == tlBinTable::MME_IGNORE_UNIQUE)
	{
		strategyString = "Maximum Marginal Entropy - Ignore Unique Values";

	} else if (binStrategy_ == tlBinTable::EQUAL_BIN_RANGE)
	{
		strategyString = "Equal Bin Range";
	}

	fprintf(ofp, "# Table of features created using binning strategy:\n");
	fprintf(ofp, "#    \"%s\"\n", strategyString);

	fprintf(ofp, "Features %d\n", numColumns_);
	for (i = 0; i < numColumns_; i++)
	{
		if (i != 0)
			fprintf(ofp, "\n");
		columnName = column_[i]->getName().getQuoted();
		fprintf(ofp, "%*s%s %s %d", indent, "",
				columnName.getValue(),
				(column_[i]->type_ == tlSrValue::STRING ? "string" :
				 column_[i]->type_ == tlSrValue::INTEGER ? "integer" :
				 column_[i]->type_ == tlSrValue::REAL ? "real" :
				"unknownType"),
				bin_[i]->getNumBins());
		if (column_[i]->isLabel())
			fprintf(ofp, " Label");
		fprintf(ofp, "\n");

		bin_[i]->storeFeatureDescription(indent + 3, ofp);
	}
	return (!ferror(ofp));
}

int
tlBinTable::dump(FILE *ofp, int printHeader) const
{
	int i;

	tlTable::dump(ofp, printHeader);

	for (i = 0; i < numColumns_; i++)
	{
		fprintf(ofp, "Bins defining column %d:\n", i);
		bin_[i]->dump(ofp);
		fprintf(ofp, "\n");
	}
	return (!ferror(ofp));
}

