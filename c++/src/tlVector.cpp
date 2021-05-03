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
 * $Id: tlVector.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"

#include "tlVector.h"



tlVector::tlVector()
{
	n_ = 0;
	nBlocks_ = 0;
	value_ = NULL;
}

tlVector::tlVector(int length)
{
	n_ = length;
	nBlocks_ = length;
	value_ = (double *) ckalloc(sizeof(double) * length);
	MSG_ASSERT(value_ != NULL, "Out of memory");
	memset(value_, 0, sizeof(double) * length);
}

tlVector::~tlVector()
{
	if (value_ != NULL)
	{
		ckfree(value_);
	}
}

const char *
tlVector::clsId() const
{
	return "tlVector";
}

void
tlVector::setNumValues(int length)
{
	n_ = length;
	nBlocks_ = length;
	value_ = (double *) ckalloc(sizeof(double) * length);
	MSG_ASSERT(value_ != NULL, "Out of memory");
	memset(value_, 0, sizeof(double) * length);
}

void
tlVector::addValue(double value)
{
	(void) listCheckSize(
			n_ + 1,
			(void **) &value_,
			&nBlocks_,
			1,
			sizeof(double));

	value_[ n_++ ] = value;
}

void
tlVector::clear()
{
	n_ = 0;
}

