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
 * $Id: tlRefManager.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdio.h>
#endif

#include "tlRefManager.h"
#include "tlRef.h"

#include "tlSrString.h"
#include "tlSrMatrix.h"

#include "massert.h"
#include "listalloc.h"
#include "tclCkalloc.h"

int tlRefManager::sNRefObjects_ = 0;
int tlRefManager::sNRefBlocks_ = 0;
tlRef **tlRefManager::sRefList_ = NULL;

tlRefManager::tlRefManager()
{
	MSG_FAIL("Constructor of static class called");
}

tlRefManager::~tlRefManager()
{
}


void
tlRefManager::sAddRef(tlRef *reffedObject)
{
	int status;

	status = listCheckSize(
			sNRefObjects_ + 1,
			(void **) &sRefList_,
			&sNRefBlocks_,
			64,
			sizeof(tlRef *));
	MSG_ASSERT(status == 1, "tlRef Management list alloc failed");
	sRefList_[ sNRefObjects_++ ] = reffedObject;
}

void
tlRefManager::sDelRef(tlRef *reffedObject)
{
	int i;

	for (i = 0; i < sNRefObjects_; i++)
	{
		if (sRefList_[i] == reffedObject)
			goto REMOVE;
	}
	MSG_ASSERT(sNRefObjects_ == 0, "Could not find object in list");
	return;

REMOVE:
	i++;
	while (i < sNRefObjects_)
	{
		sRefList_[i - 1] = sRefList_[i];
		i++;
	}
	sNRefObjects_--;

	/** if we are out of objects, clear up the list too */
	if (sNRefObjects_ == 0)
	{
		ckfree(sRefList_);
		sRefList_ = NULL;
		sNRefBlocks_ = 0;
	}
}

void
tlRefManager::sDumpRefs(FILE *fp)
{
	int i, j;

	(void) j; /** prevent compiler warning */

	if (sNRefObjects_ == 0)
	{
		return;
	}

	fprintf(fp, "Remaining referenced objects:\n");
	for (i = 0; i < sNRefObjects_; i++)
	{
		fprintf(fp, "%3d : 0x%08lx %s\n",
						i, (unsigned long) sRefList_[i],
						sRefList_[i]->clsId());
#ifdef		REFDEBUG
		for (j = 0; j < sRefList_[i]->nLocs_; j++)
		{
			fprintf(fp, "%3s :     @ %s (%d)\n", "",
							sRefList_[i]->locs_[j].filename_,
							sRefList_[i]->locs_[j].line_);
		}
#endif
	}
	fprintf(fp, "<<<\n");
	fprintf(fp, "\n");
	fflush(fp);
}

void
tlRefManager::sExitCleanup()
{
	tlSrString::sExitCleanup();
	tlSrMatrix::sExitCleanup();
}

