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
 * $Id: tlColumn.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"
#include "mathtools.h"

#include "tlColumn.h"


#define		TL_COLUMN_BLOCKSIZE		64



tlColumn::tlColumn()
{
	type_ = tlSrValue::NONE;
	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;
	name_ = tlSrString::nil;
	loadMapping_ = (-1);
	nilBitString_ = NULL;
	isLabel_ = 0;
}

tlColumn::tlColumn(int newType)
{
	type_ = newType;
	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;
	name_ = tlSrString::nil;
	loadMapping_ = (-1);
	nilBitString_ = NULL;
	isLabel_ = 0;
}

tlColumn::tlColumn(const char *name, int newType)
{
	type_ = newType;
	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;
	name_.setValue(name);
	loadMapping_ = (-1);
	nilBitString_ = NULL;
	isLabel_ = 0;
}

tlColumn::tlColumn(tlSrString name, int newType)
{
	type_ = newType;
	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;
	name_ = name; 
	loadMapping_ = (-1);
	nilBitString_ = NULL;
	isLabel_ = 0;
}

tlColumn::tlColumn(tlColumn *other)
{
	int i;

	type_ = other->type_;
	name_ = other->name_;

	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;
	nilBitString_ = NULL;

	setLength( other->getLength() );

	/** copy over the data, ensuring that the string copy gets called */
	if (type_ == tlSrValue::STRING)
	{
		for (i = 0; i < numValues_; i++)
		{
			(*data_[i].str_) = (*other->data_[i].str_);
		}
	} else
	{
		for (i = 0; i < numValues_; i++)
		{
			data_[i]= other->data_[i];
		}
	}
	loadMapping_ = other->loadMapping_;

	if ((other->nilBitString_ != NULL) && (nilBitString_ != NULL))
	{
		for (i = 0; i < numValues_; i++)
		{
			SET_BIT(nilBitString_, i, GET_BIT(other->nilBitString_, i));
		}
	}
	isLabel_ = 0;
}

tlColumn::~tlColumn()
{
	clear();
}

const char *
tlColumn::clsId() const
{
	return "tlColumn";
}

void
tlColumn::clear()
{
	int i;

	if (data_ != NULL)
	{
		if (type_ == tlSrValue::STRING)
		{
			for (i = 0; i < numValues_; i++)
			{
				delete data_[i].str_;
			}
		}
		ckfree(data_);
	}
	type_ = tlSrValue::NONE;
	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;
	name_ = tlSrString::nil;
	if (nilBitString_ != NULL)
		ckfree(nilBitString_);
}

void
tlColumn::setType(int newType)
{
	MSG_ASSERT(numValues_ == 0, "Cannot change type once data loaded");
	type_ = newType;
}

void
tlColumn::setLength(int length)
{
	int status;
	int i;

	status = listCheckSize(
				length,
				(void **) &data_,
				&numBlocks_,
				TL_COLUMN_BLOCKSIZE,
				sizeof(tlValueStorage));
	MSG_ASSERT(status, "Out of memory");

	if (length > 0)
	{
		status = listCheckSize(
				(length + 8) / 8,
				(void **) &nilBitString_,
				&numBitBlocks_,
				TL_COLUMN_BLOCKSIZE / 8,
				1);
		MSG_ASSERT(status, "Out of memory");
	}

	if (type_ == tlSrValue::STRING)
	{
		for (i = numValues_; i < length; i++)
		{
			data_[i].str_ = new tlSrString(tlSrString::nil);
			MSG_ASSERT(data_[i].str_ != NULL, "Out of memory");
		}
	}
	numValues_ = length;
}

void
tlColumn::setValue(tlColumn *other)
{
	int i;

	type_ = other->type_;
	name_ = other->name_;

	data_ = NULL;
	numValues_ = numBitBlocks_ = numBlocks_ = 0;

	setLength( other->getLength() );

	/** copy over the data, ensuring that the string copy gets called */
	if (type_ == tlSrValue::STRING)
	{
		for (i = 0; i < numValues_; i++)
		{
			(*data_[i].str_) = (*other->data_[i].str_);
			SET_BIT(nilBitString_, i, GET_BIT(other->nilBitString_, i));
		}
	} else
	{
		for (i = 0; i < numValues_; i++)
		{
			data_[i] = other->data_[i];
			SET_BIT(nilBitString_, i, GET_BIT(other->nilBitString_, i));
		}
	}
	loadMapping_ = other->loadMapping_;
}

void
tlColumn::setValue(int index, int i)
{
	MSG_ASSERT(index >= 0 && index <= numValues_, "Bad index");
	if (type_ == tlSrValue::NONE)
	{
		type_ = tlSrValue::INTEGER;
	}
	MSG_ASSERT(type_ == tlSrValue::INTEGER, "Bad Type");
	data_[index].ival_ = i;
}

void
tlColumn::setValue(int index, tlReal d)
{
	MSG_ASSERT(index >= 0 && index <= numValues_, "Bad index");
	if (type_ == tlSrValue::NONE)
	{
		type_ = tlSrValue::REAL;
	}
	MSG_ASSERT(type_ == tlSrValue::REAL, "Bad Type");
	data_[index].dval_ = d;
}

void
tlColumn::setValue(int index, const char *s)
{
	MSG_ASSERT(index >= 0 && index <= numValues_, "Bad index");
	if (type_ == tlSrValue::NONE)
	{
		type_ = tlSrValue::STRING;
	}
	MSG_ASSERT(type_ == tlSrValue::STRING, "Bad Type");
	data_[index].str_->setValue(s);
}

void
tlColumn::setValue(int index, tlSrString s)
{
	MSG_ASSERT(index >= 0 && index <= numValues_, "Bad index");
	if (type_ == tlSrValue::NONE)
	{
		type_ = tlSrValue::STRING;
	}
	MSG_ASSERT(type_ == tlSrValue::STRING, "Bad Type");
	*data_[index].str_ = s;
}

int
tlColumn::getIntegerValue(int index) const
{
	if (type_ == tlSrValue::INTEGER)
	{
		return data_[index].ival_;
	}

	if (type_ == tlSrValue::REAL)
	{
		return (int) data_[index].dval_;
	}

	return 0;
}

tlReal
tlColumn::getRealValue(int index) const
{
	if (type_ == tlSrValue::REAL)
	{
		return data_[index].dval_;
	}

	if (type_ == tlSrValue::INTEGER)
	{
		return (tlReal) data_[index].ival_;
	}

	return 0.0;
}

tlSrString
tlColumn::getStringValue(int index) const
{
	if (type_ == tlSrValue::STRING)
	{
		return *data_[index].str_;
	} else
	{
		/** create an empty string and populate it if valid data */
		tlSrString val;

		if (type_ == tlSrValue::REAL)
		{
			val.sprintf("%s", niceDouble(data_[index].dval_));

		} else if (type_ == tlSrValue::INTEGER)
		{
			val.sprintf("%d", data_[index].ival_);
		} else if (type_ == tlSrValue::STRING)
		{
			val.setValue(*data_[index].str_);

		} else
		{
			val.setValue("<unknown>");
		}

		return val;
	}
	/** NOTREACHED */
}

int tlColumn::isNilValue(int index) const
{
	if (nilBitString_ == NULL)
	{
		MSG_ASSERT(numValues_ == 0,
						"bit string null with data in table");
		return 0;
	}
	return GET_BIT(nilBitString_, index);
}

int tlColumn::hasAnyNilValue() const
{
	int bitstringLength, i;

	bitstringLength = ((numValues_ + 8) / 8);
	for (i = 0; i < bitstringLength; i++)
	{
		if (nilBitString_[i] != 0)
		{
			return 1;
		}
	}
	return 0;
}

void tlColumn::setNilValue(int index, int flag)
{
	SET_BIT(nilBitString_, index, flag);
}

void tlColumn::addNilValue()
{
	int status;
	
	status = listCheckSize(
			numValues_ + 1,
			(void **) &data_,
			&numBlocks_,
			TL_COLUMN_BLOCKSIZE,
			sizeof(tlValueStorage));
	MSG_ASSERT(status, "Out of memory");

	status = listCheckSize(
			(numValues_ + 9) / 8,
			(void **) &nilBitString_,
			&numBitBlocks_,
			TL_COLUMN_BLOCKSIZE / 8,
			1);
	MSG_ASSERT(status, "Out of memory");

	SET_BIT(nilBitString_, numValues_, 1);

	numValues_++;
}

void
tlColumn::addValue(tlSrString s)
{
	int status;

	MSG_ASSERT(type_ == tlSrValue::STRING, "Bad type");
	
	status = listCheckSize(
			numValues_ + 1,
			(void **) &data_,
			&numBlocks_,
			TL_COLUMN_BLOCKSIZE,
			sizeof(tlValueStorage));
	MSG_ASSERT(status, "Out of memory");

	status = listCheckSize(
			(numValues_ + 9) / 8,
			(void **) &nilBitString_,
			&numBitBlocks_,
			TL_COLUMN_BLOCKSIZE / 8,
			1);
	MSG_ASSERT(status, "Out of memory");

	data_[numValues_].str_ = new tlSrString(s);
	MSG_ASSERT(data_[numValues_].str_ != NULL, "Out of memory");
	numValues_++;
}

void
tlColumn::addValue(const char *s)
{
	int status;

	MSG_ASSERT(type_ == tlSrValue::STRING, "Bad type");

	status = listCheckSize(
			numValues_ + 1,
			(void **) &data_,
			&numBlocks_,
			TL_COLUMN_BLOCKSIZE,
			sizeof(tlValueStorage));
	MSG_ASSERT(status, "Out of memory");

	status = listCheckSize(
			(numValues_ + 9) / 8,
			(void **) &nilBitString_,
			&numBitBlocks_,
			TL_COLUMN_BLOCKSIZE / 8,
			1);
	MSG_ASSERT(status, "Out of memory");

	data_[numValues_].str_ = new tlSrString(s);
	MSG_ASSERT(data_[numValues_].str_ != NULL, "Out of memory");
	numValues_++;
}

void
tlColumn::addValue(int i)
{
	int status;

	MSG_ASSERT(type_ == tlSrValue::INTEGER, "Bad type");

	status = listCheckSize(
			numValues_ + 1,
			(void **) &data_,
			&numBlocks_,
			TL_COLUMN_BLOCKSIZE,
			sizeof(tlValueStorage));
	MSG_ASSERT(status, "Out of memory");

	status = listCheckSize(
			(numValues_ + 9) / 8,
			(void **) &nilBitString_,
			&numBitBlocks_,
			TL_COLUMN_BLOCKSIZE / 8,
			1);
	MSG_ASSERT(status, "Out of memory");

	data_[numValues_++].ival_ = i;
}

void
tlColumn::addValue(tlReal d)
{
	int status;

	MSG_ASSERT(type_ == tlSrValue::REAL, "Bad type");

	status = listCheckSize(
			numValues_ + 1,
			(void **) &data_,
			&numBlocks_,
			TL_COLUMN_BLOCKSIZE,
			sizeof(tlValueStorage));
	MSG_ASSERT(status, "Out of memory");

	status = listCheckSize(
			(numValues_ + 9) / 8,
			(void **) &nilBitString_,
			&numBitBlocks_,
			TL_COLUMN_BLOCKSIZE / 8,
			1);
	MSG_ASSERT(status, "Out of memory");

	data_[numValues_++].dval_ = d;
}

void
tlColumn::addValue(tlSrValue val)
{
	/** if we have no type yet, then set it */
	if (type_ == 0)
	{
		type_ = val.getType();
	}

	if (val.isNil())
	{
		addNilValue();

	} else if (type_ == tlSrValue::STRING)
	{
		addValue( val.getStringValue() );

	} else if (type_ == tlSrValue::REAL)
	{
		addValue( val.getRealValue() );

	} else if (type_ == tlSrValue::INTEGER)
	{
		addValue( val.getIntegerValue() );
	}
}

void
tlColumn::calculateVariance(double *sigmaSquared_p, double *mu_p) const
{
	double sqSum, muSum, mu;
	double val = 0;
	int i, nNonNilValues;

	if (type_ == tlSrValue::STRING)
	{
		*sigmaSquared_p = (-1);
		*mu_p = (-1);
		return;
	}

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
	for (i = 0; i < numValues_; i++)
	{
		if (! isNilValue(i))
		{
			nNonNilValues++;

			if (type_ == tlSrValue::REAL)
				val = data_[i].dval_;

			else if (type_ == tlSrValue::INTEGER)
				val = (double) data_[i].ival_;

			else
			{
				MSG_ASSERT(type_ == tlSrValue::STRING, "Bad type");
			}

			muSum = muSum + val;
			sqSum = sqSum + SQR(val);
		}
	}

	if (nNonNilValues < 2)
	{
		*mu_p = muSum;
		*sigmaSquared_p = 0;
	}
	else
	{
		*mu_p = mu = muSum / nNonNilValues;
		*sigmaSquared_p =
					(1.0 / (double) (nNonNilValues - 1))
				* (sqSum - ( nNonNilValues * SQR(mu)));
	}
}


double
tlColumn::calculateMean() const
{
	double muSum = 0;
	double val = 0;
	int i;

	if (type_ == tlSrValue::STRING)
	{
		return (-1);
	}

	for (i = 0; i < numValues_; i++)
	{

		if (type_ == tlSrValue::REAL)
		{
			val = data_[i].dval_;
		} else if (type_ == tlSrValue::INTEGER)
		{
			val = (double) data_[i].ival_;
		} else
		{
			MSG_ASSERT(type_ == tlSrValue::STRING, "Bad type");
		}

		muSum = muSum + val;
	}

	if (numValues_ < 2)
	{
		return muSum;
	}

	return muSum / numValues_;
}


