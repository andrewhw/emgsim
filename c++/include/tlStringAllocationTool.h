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
 * CUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, BIN, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *..
 * $Id: tlStringAllocationTool.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__ALLOCATION_TOOL_HEADER__
#define		__ALLOCATION_TOOL_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
#endif

#include "os_defs.h"

#include "tlRef.h"
#include "tlSrString.h"

/**
CLASS
		tlStringAllocationTool

	Remember things that need to be deleted.
*/
class OS_EXPORT tlStringAllocationTool : public tlRef
{
private:
	int n_;
	int nBlocks_;
	tlSrString **references_;

public:
	////////////////////////////////////////
	// Constructor
	tlStringAllocationTool();

protected:
	////////////////////////////////////////
	// Destructor.
	virtual ~tlStringAllocationTool();

public:

	////////////////////////////////////////
	// Add a reference to the internal list
	void add(tlSrString *str);

	////////////////////////////////////////
	// Cleanup all the internal references
	void clean();
};

#endif /* __ALLOCATION_TOOL_HEADER__ */

