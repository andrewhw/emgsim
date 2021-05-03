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
 * $Id: tlErrorManager.h 58 2014-01-06 20:29:59Z andrew $
 */


#ifndef		__TOOL_ERROR_MANAGER_HEADER__
#define		__TOOL_ERROR_MANAGER_HEADER__

#ifndef MAKDEPEND
# include <stdio.h>
# include <stdarg.h>
#endif

#include "os_defs.h"

#include "tlSrString.h"
#include "tlErrorManager.h"

/**
CLASS
		tlErrorManager

	Manage a list of errors, clearing when asked
*/
class OS_EXPORT tlErrorManager : public tlRef
{
private:
	int numErrors_;
	int numErrorBlocks_;
	tlSrString **errors_;

protected:
	static tlErrorManager *sGlobalErrorManager_;

public:
	////////////////////////////////////////
	// Create an empty error manager
	tlErrorManager();

protected:
	////////////////////////////////////////
	// Clean up the errors we have
	virtual ~tlErrorManager();

public:
	////////////////////////////////////////
	// Add an error to the list
	void addError(const char *format, ...);

	////////////////////////////////////////
	// Add an error to the list
	void addError(tlSrString message);

	////////////////////////////////////////
	// Clear any errors we currently are recording
	void clearErrors();

	////////////////////////////////////////
	// How many errors do we have?
	int getNumErrors() const;

	////////////////////////////////////////
	// get an error by index
	tlSrString getError(int index) const;

	////////////////////////////////////////
	// factory for a single global error manager
	static tlErrorManager *globalErrorManager();
};

inline int tlErrorManager::getNumErrors() const
{
	return numErrors_;
}

inline tlSrString tlErrorManager::getError(int index) const
{
	if (errors_ == NULL || index < 0 || index >= numErrors_) {
		tlSrString badError("Bad Error Index");
		return badError;
	}
	return *errors_[index];
}

inline tlErrorManager *tlErrorManager::globalErrorManager()
{
	if (sGlobalErrorManager_ == NULL) {
		sGlobalErrorManager_ = new tlErrorManager();
	}
	return sGlobalErrorManager_;
}

#endif /* __TOOL_ERROR_MANAGER_HEADER__ */

