/**
 * Copyright (c) 2013
 * All rights reserved.
 *
 * Andrew Hamilton-Wright (andrewhw@ieee.org)
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
 * ----------------------------------------------------------------
 *
 * Creation and storage of a Kohonen Self Organizing Map
 *
 * ----------------------------------------------------------------
 * $Id: tlDataChooser.h 57 2013-12-13 21:33:01Z andrew $
 */

#ifndef		__DATA_CHOOSER_HEADER__
#define		__DATA_CHOOSER_HEADER__

#include <os_types.h>
#include <tlRef.h>

#include <bitstring.h>

/**
CLASS
		tltlDataChooser

	A Fuzzy Set classifier.
*/
class OS_EXPORT tlDataChooser : public tlRef
{
private:
		BITSTRING usebit_;
		long nElements_;
		long nUsed_;
		long nCycles_;

public:
		////////////////////////////////
		// create a chooser of a given size
		tlDataChooser(long size);

protected:
		virtual ~tlDataChooser();

public:
		////////////////////////////////
		// set all of the elements to unchosen,
		// and clear the number of cycles
		void reset();

		////////////////////////////////
		// choose an element from the list
		long choose();

		////////////////////////////////
		// return the number of cycles
		// through the list
		long getNCycles();

		////////////////////////////////
		// are we at the beginning of a cycle?
		int isStartOfCycle();

		////////////////////////////////
		// are we at the end of a cycle?
		int isEndOfCycle();

		////////////////////////////////
		// return the number of elements
		// chosen in this cycle
		long getNUsed();

		////////////////////////////////
		// return the number of elements
		// defined in the list
		long getNElements();
};

inline long tlDataChooser::getNCycles() {
	return nCycles_;
}

inline long tlDataChooser::getNUsed() {
	return nUsed_;
}

inline long tlDataChooser::getNElements() {
	return nElements_;
}

inline int tlDataChooser::isStartOfCycle() {
	return (nUsed_ == 0);
}

inline int tlDataChooser::isEndOfCycle() {
	return nUsed_ == nElements_;
}

#endif /* __DATA_CHOOSER_HEADER__ */

