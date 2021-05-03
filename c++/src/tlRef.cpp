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
 * $Id: tlRef.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
#endif

#include "tlRef.h"
#include "tlRefManager.h"

#include "massert.h"
#include "listalloc.h"
#include "tclCkalloc.h"


tlRef::tlRef()
{
	refCount_ = 0;
	isTouched_ = 0;
#ifdef REFDEBUG
	tlRefManager::sAddRef(this);

	blocks_ = 0;
	nLocs_ = 0;
	locs_ = NULL;
#endif
}

tlRef::~tlRef()
{
#ifdef REFDEBUG
	tlRefManager::sDelRef(this);

	if (locs_ != NULL)
	{
		ckfree(locs_);
	}
#endif
}

const char *
tlRef::clsId() const
{
	return "tlRef";
}

#ifdef REFDEBUG
void
tlRef::debugRef(const char *filename, int line)
{
	int status;

	refCount_++;
	if ( ! isTouched_ )
	{
		isTouched_ = 1;
	}

	status = listCheckSize(
			nLocs_ + 1,
			(void **) &locs_,
			&blocks_,
			4,
			sizeof(struct Location));
	MSG_ASSERT(status, "Out of memory");

	locs_[ nLocs_ ].filename_ = filename;
	locs_[ nLocs_ ].line_ = line;
	nLocs_++;
}
#endif

