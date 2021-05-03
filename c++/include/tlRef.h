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
 * $Id: tlRef.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_REF_HEADER__
#define		__TOOL_REF_HEADER__

#include "os_defs.h"

#include "massert.h"

class tlRefManager;
/**
CLASS
		tlRef

	Base ref-counting class for memory management.
	All child classes should have protected
	desctructors, which will only be called
	from unref here.

	If there are no circular dependencies based
	on references, then this will manage memory
	for us, based on the life of the object.
*/
class OS_EXPORT tlRef
{
protected:
	int refCount_;
	int isTouched_;

#ifdef REFDEBUG
	/** only record locations if we are debugging references */
	struct Location {
		const char *filename_;
		int line_;
	};

	int blocks_;
	int nLocs_;
	struct Location *locs_;
#endif


public:
	////////////////////////////////////////
	// Constructor
	tlRef();

protected:
	////////////////////////////////////////
	// Protected destructor, called only
	// from unref()
	virtual ~tlRef();

	////////////////////////////////////////
	// Return the typename for
	// reference debugging
	virtual const char *clsId() const;

public:
#ifdef REFDEBUG

	////////////////////////////////////////
	// Called in order to make a reference to the
	// object is made
	void debugRef(const char *filename, int lineno);

#else

	////////////////////////////////////////
	// Called in order to release a reference to the
	// object is made
	void ref();
#endif

	////////////////////////////////////////
	// Called every time a reference to an
	// object is released.
	void unref();

	friend class tlRefManager;
};

#ifdef REFDEBUG
/**
 * when we call ref() in REFDEBUG mode,, we actually want
 * to call debugRef, defined in the cpp file
 */
#define		ref()		debugRef(__FILE__, __LINE__)

#else
inline void
tlRef::ref()
{
	refCount_++;
	if ( ! isTouched_ )
		isTouched_ = 1;
}
#endif

inline void
tlRef::unref()
{
	MSG_ASSERT(isTouched_ == 1,
		"unref occurring on object which was never reffed\n");

	if (--refCount_ == 0) {
		delete this;
	}
}


#endif /* __TOOL_REF_HEADER__ */

