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
 * $Id: tlTrie.h 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef		__TOOL_TRIE_HEADER__
#define		__TOOL_TRIE_HEADER__

#include "os_defs.h"

#include "tlRef.h"

#include "tclCkalloc.h"
#include "tlTVector.h"
#include "bitstring.h"


#define		TL_T_TRIE_BLOCKSIZE		8
/**
CLASS
		tlTrie

	A template vector class (similar to the STL vector) which can be
	used when the STL class cannot -- notably in MFC projects
*/
class OS_EXPORT tlTrie : public tlRef
{
protected:
	int n_;
	tlTrie **subTrie_;
	long *valueList_;
	BITSTRING isValid_;

public:
	////////////////////////////////////////
	// Constructor
	tlTrie();

	////////////////////////////////////////
	// Destructor
	~tlTrie();

	////////////////////////////////////////
	// Loads result with the stored value
	// passed in at add time.
	int query(long *result, void *key, size_t length) const;

	////////////////////////////////////////
	// Add the given data value to the trie;
	// if a value already exists for this key,
	// it is overwritten
	int add(void *key, size_t length, long data);
};

#endif /* __TOOL_TRIE_HEADER__ */
