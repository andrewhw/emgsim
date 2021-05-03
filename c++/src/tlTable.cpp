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
 * $Id: tlTable.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"

#include "tlTable.h"
#include "tlColumn.h"
#include "tlTuple.h"

tlTable::tlTable()
{
	numColumns_ = 0;
	columnBlocks_ = 0;

	column_ = NULL;
	maxRows_ = 0;
	sigmaSquared_ = NULL;
	mu_ = NULL;
	hasLoadMappings_ = 0;

	tupleList_ = NULL;
}

tlTable::tlTable(tlTable *other)
{
	int status, i;

	sigmaSquared_ = NULL;
	mu_ = NULL;

	hasLoadMappings_ = other->hasLoadMappings_;

	numColumns_ = other->numColumns_;
	maxRows_ = other->maxRows_;

	column_ = NULL;
	columnBlocks_ = 0;

	status = listCheckSize(
			numColumns_,
			(void **) &column_,
			&columnBlocks_,
			1,
			sizeof(tlColumn *));
	MSG_ASSERT(status, "Out of memory");

	for (i = 0; i < numColumns_; i++)
	{
		column_[i] = new tlColumn( other->column_[i] );
		MSG_ASSERT(column_[i] != NULL, "Out of memory");
		column_[i]->ref();
	}

	if (other->sigmaSquared_ != NULL)
		calculateVariances();

	tupleList_ = NULL;
}

tlTable::~tlTable()
{
	clear();
}

void
tlTable::clear()
{
	int i;

	if (column_ != NULL)
	{
		for (i = 0; i < numColumns_; i++)
		{
			if (column_[i] != NULL)
			{
				column_[i]->unref();
			}
		}
		ckfree(column_);
		column_ = NULL;
	}
	if (sigmaSquared_ != NULL)
	{
		ckfree(sigmaSquared_);
		sigmaSquared_ = NULL;
		ckfree(mu_);
		mu_ = NULL;
	}
	columnBlocks_ = 0;
	numColumns_ = 0;

	clearTupleList_();
}


void
tlTable::clearTupleList_()
{
	int i;

	if (tupleList_ != NULL)
	{
		for (i = 0; i < maxRows_; i++)
		{
			if (tupleList_[i] != NULL)
				tupleList_[i]->unref();
		}
		ckfree(tupleList_);
		tupleList_ = NULL;
	}
}

void
tlTable::initTupleList_()
{
	if (tupleList_ == NULL && maxRows_ != 0)
	{
		tupleList_ = (tlTuple **) ckalloc(sizeof(tlTuple *) * maxRows_);
		memset(tupleList_, 0, sizeof(tlTuple *) * maxRows_);
	}
}

tlTuple *
tlTable::getRow(int index)
{
	int i;

	initTupleList_();

	if (index < 0 || index >= maxRows_)
		return NULL;

	if (tupleList_[index] == NULL)
	{
		tupleList_[index] = new tlTuple();
		tupleList_[index]->ref();
		MSG_ASSERT(tupleList_[index] != NULL, "Out of memory");
		for (i = 0; i < numColumns_; i++)
		{

			if ( column_[i]->isNilValue(index) )
			{
				tupleList_[index]->addValue( tlSrValue::nil );

			} else if (column_[i]->getType() == tlSrValue::INTEGER)
			{
				tupleList_[index]->addValue(
							column_[i]->getIntegerValue(index)
						);

			} else if (column_[i]->getType() == tlSrValue::REAL)
			{
				tupleList_[index]->addValue(
							column_[i]->getRealValue(index)
						);

			} else if (column_[i]->getType() == tlSrValue::STRING)
			{
				tupleList_[index]->addValue(
							column_[i]->getStringValue(index)
						);

			} else
			{
				tupleList_[index]->addValue(tlSrValue::nil);
			}
		}
	}

	return tupleList_[index];
}

void
tlTable::addRow(tlTuple *row)
{
	int i;

	MSG_ASSERT(row->getNumValues() == numColumns_,
				"Mismatch in array width");

	clearTupleList_();

	for (i = 0; i < numColumns_; i++)
	{
		column_[i]->addValue( row->getValue(i) );
		if ( maxRows_ < column_[i]->getLength())
		{
			maxRows_ = column_[i]->getLength();
		}
	}
}

void
tlTable::addColumn(tlSrString name, int type)
{
	int status;

	MSG_ASSERT(maxRows_ == 0, "Adding column after data added");

	status = listCheckSize(
			numColumns_ + 1,
			(void **) &column_,
			&columnBlocks_,
			1,
			sizeof(tlColumn *));
	MSG_ASSERT(status, "Out of memory");

	column_[numColumns_] = new tlColumn(name, type);
	MSG_ASSERT(column_[numColumns_] != NULL, "Out of memory");
	column_[numColumns_]->ref();
	numColumns_++;
}

void
tlTable::addColumn(const char *name, int type)
{
	if (name != NULL)
	{
		tlTable::addColumn(tlSrString(name), type);
	} else
	{
		tlTable::addColumn(tlSrString::nil, type);
	}
}

tlSrValue
tlTable::getValue(int row, int column) const
{
	tlColumn *col;
	tlSrValue result;

	MSG_ASSERT(column >= 0 && column < numColumns_,
			"column out of range");
	col = getColumn(column);

	MSG_ASSERT(row >= 0 && row < col->getLength(),
					"row out of range");

	if (col->isNilValue(row))
	{
		result.setNil();

	} else
	{
		if (col->getType() == tlSrValue::STRING)
		{
			result.set(col->getStringValue(row));

		} else if (col->getType() == tlSrValue::INTEGER)
		{
			result.set(col->getIntegerValue(row));

		} else if (col->getType() == tlSrValue::REAL)
		{
			result.set(col->getRealValue(row));
		}
	}
	return result;
}

int tlTable::getColumnIndex(const char *name) const
{
	int i;

	for (i = 0; i < numColumns_; i++)
	{
		if (column_[i]->getName() == name)
			return i;
	}

	return (-1);
}

tlColumn *tlTable::getColumn(const char *name) const
{
	int index;

	index = getColumnIndex(name);
	if (index < 0)
		return NULL;

	return column_[index];
}

int
tlTable::calculateVariances()
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
		column_[i]->calculateVariance(&sigmaSquared_[i], &mu_[i]);
	}
	return 1;
}

void
tlTable::clearColumnLoadMappings()
{
	int i;

	for (i = 0; i < numColumns_; i++)
	{
		column_[i]->setLoadMapping(-1);
	}
	hasLoadMappings_ = 0;
}

void
tlTable::setColumnLoadMapping(int actualIndex, int mappedIndex)
{
	int i;

	column_[actualIndex]->setLoadMapping(mappedIndex);

	/** check whether we still have any mappings */
	if (mappedIndex >= 0)
	{
		hasLoadMappings_ = 1;
	} else
	{
		for (i = 0; i < numColumns_; i++)
		{
			hasLoadMappings_ = 0;
			if (column_[i]->getLoadMapping() >= 0)
			{
				hasLoadMappings_ = 1;
				break;
			}
		}
	}
}

int
tlTable::dump(FILE *ofp, int printHeader) const
{
	tlSrValue dataVal;
	int *widths, numWidth = 5;
	int i, j;

	widths = (int *) ckalloc(numColumns_ * sizeof(int));
	MSG_ASSERT(widths != NULL, "Out of memory");


	/** calculate the column widths */
	for (j = 0; j < numColumns_; j++)
	{
		widths[j] = getColumnName(j).getLength();
		if (widths[j] < 7)
			widths[j] = 7;
	}


	/** dump the data */
	for (i = 0; i < maxRows_; i++)
	{

		/** print a header row every 25 */
		if ((i == 0) || (printHeader && (i % 25) == 0))
		{

			fprintf(ofp, "%*s : ", numWidth, "");
			for (j = 0; j < numColumns_; j++)
			{
				fprintf(ofp, "%*s : ",
						widths[j],
						getColumnName(j).getValue());
			}
			fprintf(ofp, "\n");

		}

		/** dump the data */
		fprintf(ofp, "%*d : ", numWidth, i);
		for (j = 0; j < numColumns_; j++)
		{
			dataVal = getValue(i, j);
			if (dataVal.getType() == tlSrValue::REAL)
			{
				fprintf(ofp, "%*.*f : ",
						widths[j], widths[j] - 4,
						dataVal.getRealValue());

			} else if (dataVal.getType() == tlSrValue::INTEGER)
			{
				fprintf(ofp, "%*d : ",
						widths[j],
						dataVal.getIntegerValue());

			} else if (dataVal.getType() == tlSrValue::STRING)
			{
				fprintf(ofp, "%*s : ",
						widths[j],
						dataVal.getStringValue().getValue());
			} else
			{
				fprintf(ofp, "%*s : ", widths[j], "<NUL>");
			}
		}
		fprintf(ofp, "\n");
	}
	fprintf(ofp, "\n");

	ckfree(widths);

	return !ferror(ofp);
}

