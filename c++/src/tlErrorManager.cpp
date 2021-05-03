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
 * $Id: tlErrorManager.cpp 58 2014-01-06 20:29:59Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <stdarg.h>
# include <math.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"
#include "massert.h"
#include "tokens.h"
#include "attvalfile.h"

#include "tlErrorManager.h"
#include "tlSrString.h"

/** static initialization */
tlErrorManager *tlErrorManager::sGlobalErrorManager_ = NULL;


tlErrorManager::tlErrorManager()
{
	numErrors_ = 0;
	numErrorBlocks_ = 0;
	errors_ = NULL;
}

tlErrorManager::~tlErrorManager()
{
	clearErrors();
}

void tlErrorManager::clearErrors()
{
	int i;

	if (errors_ != NULL)
	{
		for (i = 0; i < numErrors_; i++)
		{
			if (errors_[i] != NULL)
			{
				delete errors_[i];
			}
		}
		ckfree(errors_);
	}
	numErrors_ = 0;
	numErrorBlocks_ = 0;
	errors_ = NULL;
}

void tlErrorManager::addError(const char *format, ...)
{
	va_list args;
	tlSrString *newString = new tlSrString();
	MSG_ASSERT(newString != NULL, "Out of memory");

	va_start(args, format);
	newString->vsprintf(format, args);
	va_end(args);

	listCheckSize(
			numErrors_ + 1,
			(void **) &errors_,
			&numErrorBlocks_,
			4,
			sizeof(tlSrString *));
	errors_[numErrors_++] = newString;
}


void tlErrorManager::addError(tlSrString message)
{
	tlSrString *newString = new tlSrString(message);
	MSG_ASSERT(newString != NULL, "Out of memory");

	listCheckSize(
			numErrors_ + 1,
			(void **) &errors_,
			&numErrorBlocks_,
			4,
			sizeof(tlSrString *));
	errors_[numErrors_++] = newString;
}

