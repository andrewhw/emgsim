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
 * $Id: tlStringAllocationTool.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#include "tclCkalloc.h"
#include "listalloc.h"

#include "tlStringAllocationTool.h"

#define BLOCKSIZE		(BUFSIZ / sizeof(void *))

tlStringAllocationTool::tlStringAllocationTool()
{
	n_ = 0;
	nBlocks_ = 0;
	references_ = NULL;
}

tlStringAllocationTool::~tlStringAllocationTool()
{
	clean();
}

void tlStringAllocationTool::add(tlSrString *newReference)
{
	int status;
	
	status = listCheckSize(
				n_ + 1,
				(void **) &references_,
				&nBlocks_,
				BLOCKSIZE,
				sizeof(tlSrString *));
	MSG_ASSERT(status, "Out of memory");
	references_[n_++] = newReference;
}

void tlStringAllocationTool::clean()
{
	if (n_ > 0)
	{
		while (n_ > 0)
		{
			n_--;
			delete references_[n_];
		}

		ckfree(references_);
		references_ = NULL;
		nBlocks_ = 0;
	}
}

