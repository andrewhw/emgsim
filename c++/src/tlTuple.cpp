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
 * $Id: tlTuple.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"

#include "tlTuple.h"


#define		BLOCKSIZE		4

tlTuple::tlTuple()
{
	n_ = 0;
	nBlocks_ = 0;
	value_ = NULL;
}

tlTuple::tlTuple(int length, tlSrValue *initialValues)
{
	int i;

	n_ = length;
	nBlocks_ = (length / BLOCKSIZE) + 1;
	value_ = new tlSrValue[ nBlocks_ * BLOCKSIZE ];
	MSG_ASSERT(value_ != NULL, "Out of memory");
	if (initialValues != NULL)
	{
		for (i = 0; i < n_; i++)
		{
			value_[i] = initialValues[i];
		}
	}
}

tlTuple::tlTuple(tlTuple *src)
{
	int i;

	n_ = src->n_;
	nBlocks_ = (n_ / BLOCKSIZE) + 1;
	value_ = new tlSrValue[ nBlocks_ * BLOCKSIZE ];
	MSG_ASSERT(value_ != NULL, "Out of memory");
	for (i = 0; i < n_; i++)
	{
		value_[i] = src->value_[i];
	}
}

tlTuple::tlTuple(tlTuple &src)
{
	int i;

	n_ = src.n_;
	nBlocks_ = (n_ / BLOCKSIZE) + 1;
	value_ = new tlSrValue[ nBlocks_ * BLOCKSIZE ];
	MSG_ASSERT(value_ != NULL, "Out of memory");
	for (i = 0; i < n_; i++)
	{
		value_[i] = src.value_[i];
	}
}

tlTuple::~tlTuple()
{
	clear();
}

const char *
tlTuple::clsId() const
{
	return "tlTuple";
}

void
tlTuple::setNumValues(int length)
{
	clear();

	nBlocks_ = (length / BLOCKSIZE) + 1;
	value_ = new tlSrValue[ nBlocks_ * BLOCKSIZE ];
	MSG_ASSERT(value_ != NULL, "Out of memory");
	n_ = length;
}

void
tlTuple::setValues(tlTuple *other)
{
	int i;

	setNumValues( other->getNumValues() );
	for (i = 0; i < n_; i++)
	{
		setValue( i, other->getValue(i) );
	}
}

void
tlTuple::addNil()
{
	int blocksNeeded;
	int i;

	blocksNeeded = ((n_ + 1) / BLOCKSIZE) + 1;
	if (nBlocks_ < blocksNeeded)
	{
		tlSrValue *old;

		nBlocks_ = blocksNeeded;
		old = value_;
		value_ = new tlSrValue[ nBlocks_ * BLOCKSIZE ];
		MSG_ASSERT(value_ != NULL, "Out of memory");
		if (old != NULL)
		{
			for (i = 0; i < n_; i++)
			{
				value_[i] = old[i];
			}
			delete [] old;
		}
	}

	value_[ n_ ] = tlSrValue::nil;
	n_++;
	setNil(n_-1);
}


void
tlTuple::addValue(tlSrValue newValue)
{
	int blocksNeeded;
	int i;

	blocksNeeded = ((n_ + 1) / BLOCKSIZE) + 1;
	if (nBlocks_ < blocksNeeded)
	{
		tlSrValue *old;

		nBlocks_ = blocksNeeded;
		old = value_;
		value_ = new tlSrValue[ nBlocks_ * BLOCKSIZE ];
		MSG_ASSERT(value_ != NULL, "Out of memory");
		if (old != NULL)
		{
			for (i = 0; i < n_; i++)
			{
				value_[i] = old[i];
			}
			delete [] old;
		}
	}

	value_[ n_++ ] = newValue;
}

void
tlTuple::addValue(tlSrString s)
{
	tlSrValue strVal(s);

	tlTuple::addValue(strVal);
}

void
tlTuple::addValue(const char *s)
{
	tlSrValue strVal(s);

	tlTuple::addValue(strVal);
}

void
tlTuple::addValue(int i)
{
	tlSrValue iVal(i);

	tlTuple::addValue(iVal);
}

void
tlTuple::addValue(tlReal d)
{
	tlSrValue dVal(d);

	tlTuple::addValue(dVal);
}

tlSrString
tlTuple::getStringValue() const
{
	tlSrString r;
	int i;

	if (n_ > 0)
	{
		r.setValue(value_[0].getStringValue().getValue());
		for (i = 1; i < n_; i++)
		{
			r.sprintf("%s, %s",
							r.getValue(),
						value_[i].getStringValue().getValue());
		}
	}
	return r;
}

int
tlTuple::printRow(FILE *ofp, int columnWidth, const char *delimString) const
{
	int i;

	for (i = 0; i < n_; i++)
	{
		if (i != 0)
		{
			fputs(delimString, ofp);
		}
		if ( ! value_[i].printValue(ofp, columnWidth) )
			return 0;
	}
	fputc('\n', ofp);

	return !ferror(ofp);
}

void
tlTuple::clear()
{
	if (value_ != NULL)
	{
		delete [] value_;
		value_ = NULL;
	}
	n_ = 0;
	nBlocks_ = 0;
}

