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
 * $Id: tlRefManager.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_REF_MANAGER_HEADER__
#define		__TOOL_REF_MANAGER_HEADER__

#include "os_defs.h"

#include <stdio.h>

class tlRef;

/**
CLASS
		tlRefManager

	Memory management class to count the number of ref-counted
	objects in memory.

	Used for debugging, to ensure that unref's are done
	correctly.
*/
class OS_EXPORT tlRefManager
{
private:
	static int sNRefObjects_;
	static int sNRefBlocks_;
	static tlRef **sRefList_;

protected:
	////////////////////////////////////////
	// private constructor -- static only
	tlRefManager();

	////////////////////////////////////////
	// static only class -- no destructor
	~tlRefManager();

public:
	////////////////////////////////////////
	// add an object to the list
	static void sAddRef(tlRef *reffedObject);

	////////////////////////////////////////
	// remove the object from the list
	static void sDelRef(tlRef *reffedObject);

	////////////////////////////////////////
	// Dump out all the objects
	static void sDumpRefs(FILE *fp);


	////////////////////////////////////////
	// Clean up static refs
	static void sExitCleanup();
};


#endif /* __TOOL_REF_MANAGER_HEADER__ */

