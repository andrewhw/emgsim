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
 * $Id: tlTrie.cpp 57 2013-12-13 21:33:01Z andrew $
 */


#ifndef MAKEDEPEND
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
# include <ctype.h>
# include <math.h>
#endif

#include "tclCkalloc.h"
#include "listalloc.h"
#include "stringtools.h"

#include "tlTrie.h"

/**
 * we calculate branching by byte, so therefore our fanout
 * is based on the maximum value in a byte (plus one for the
 * zero value)
 */
#define	FANOUT (int) (UCHAR_MAX + 1)

tlTrie::tlTrie()
{
	n_ = 0;
	subTrie_ = NULL;
	valueList_ = NULL;
	isValid_ = NULL;
}

tlTrie::~tlTrie()
{
	int i;

	if (n_ > 0)
	{
		if (subTrie_ != NULL)
		{
			for (i = 0; i < FANOUT; i++)
			{
				if (subTrie_[i] != NULL)
					delete subTrie_[i];
			}
			ckfree(subTrie_);
		}

		if (valueList_ != NULL)
			ckfree(valueList_);
		
		if (isValid_ != NULL)
			FREE_BITSTRING(isValid_);
	}
}

int
tlTrie::query(long *result, void *voidKey, size_t length) const
{
	unsigned char key, *keyString;

	keyString = (unsigned char *) voidKey;

	if (n_ == 0)
		return -1;


	key = keyString[0];
	if (length == 1)
	{
		/** then we are simply looking for data */
		if (valueList_ == NULL)
			return -1;

		if (GET_BIT(isValid_, key) == 0)
			return -1;

		*result = valueList_[ key ];
		return 1;
	}


	/** if the key doesn't exist, no subkey can */
	if ((subTrie_ == NULL) || (subTrie_[ key ] == NULL))
		return -1;

	/** otherwise, ask our appropriate child for the data */
	return subTrie_[ key ]->query(result, &keyString[1], length - 1);
}

int
tlTrie::add(void *voidKey, size_t length, long value)
{
	unsigned char key, *keyString;

	keyString = (unsigned char *) voidKey;

	key = keyString[0];
	n_++;

	if (length == 1)
	{
		/** then we are simply looking for data */
		if (valueList_ == NULL)
		{
			valueList_ = (long *) ckalloc(sizeof(long) * FANOUT);
			isValid_ = ALLOC_BITSTRING(FANOUT);
			ZERO_BITSTRING(isValid_, FANOUT);
		}

		valueList_[ key ] = value;
		SET_BIT(isValid_, key, 1);
		return 1;
	}

	/** if we have no storage for sub-keys, create that ... */
	if (subTrie_ == NULL)
	{
		subTrie_ = (tlTrie **) ckalloc(sizeof(tlTrie *) * FANOUT);
		memset(subTrie_, 0x00, sizeof(tlTrie *) * FANOUT);
	}

	/** then, if the key doesn't exist, create it ... */
	if (subTrie_[ key ] == NULL)
	{
		subTrie_[ key ] = new tlTrie();
	}

	/** and then insert into it based on our sub-value */
	return subTrie_[ key ]->add(&keyString[1], length - 1, value);
}

